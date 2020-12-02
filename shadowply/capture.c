#include <stdio.h>
#include <Windows.h>
#include "capture.h"
#include "util.h"
#include "window_util.h"
#include <inttypes.h>

void capture_init(struct capture_capture* c, const char* title, int fps) {
	memset(c, 0, sizeof(struct capture_capture));

	c->window = window_util_find_window(title);
	c->hdc = NULL;
	c->fps = fps;

	RECT rect;
	int width = 0, height = 0;

	if (GetClientRect(c->window, &rect)) {
		c->width = rect.right - rect.left;
		c->height = rect.bottom - rect.top;
		c->x = 0; // TODO:
		c->y = 0;
	}
	return c;
}

void capture_free(struct capture_capture* c) {
	struct capture_bmp_node* node = c->bmp_node_first;
	// free up linked list
	for (;;) {
		if (node == NULL) {
			break;
		}
		struct capture_bmp_node* tmp = node;
		node = node->next;
		DeleteObject(tmp->bmp);
		free(tmp);
	}
	ReleaseDC(c->window, c->hdc);
	CloseWindow(c->window);
	DeleteObject(c->window);
	DeleteObject(c->hdc);
	free(c);
}

/// <summary>
/// Use BitBlt to capture window from HDC
/// </summary>
/// <param name="c"></param>
/// <returns></returns>
char* capture_frame(struct capture_capture* c) {

	HDC hdc_target = GetDC(c->window);

	if (c->hdc == NULL) {
		c->hdc = CreateCompatibleDC(hdc_target);
	}

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;
	bmi.biWidth = c->width;
	bmi.biHeight = c->height;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = c->width * c->height;

	HBITMAP hBitmap = CreateCompatibleBitmap(hdc_target, c->width, c->height);
	// HBITMAP hOldBitmap = (HBITMAP)SelectObject(c->hdc, hBitmap);

	SelectObject(c->hdc, hBitmap);
	BitBlt(c->hdc, 0, 0, c->width, c->height, hdc_target, c->x, c->y, SRCCOPY);
	// hBitmap = (HBITMAP)SelectObject(c->hdc, hOldBitmap);

	const h = c->width* c->height * 3;

	char *rgb = malloc(h);

	GetDIBits(c->hdc, hBitmap, 0, c->height, rgb, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	ReleaseDC(NULL, hdc_target);
	DeleteObject(hBitmap);

	return rgb;
}

// start capturing frames - sleep based on fps
void capture_start_capture_loop(struct capture_capture* c) {
	uint64_t nanoseconds_per_frame = util_get_nanoseconds_per_frame(c->fps);
	uint64_t init_time = util_get_system_time_ns();
	int frameCount = 0;

	for (;;) {
		if (frameCount >= c->fps * 10) {
			break;
		}
		uint64_t start_time = util_get_system_time_ns();
		uint64_t time_target = start_time + nanoseconds_per_frame;

		// TODO: handle if frame capture takes longer than time target
		char *rgb= capture_frame(c);
		capture_add_bmp(c, rgb);

		uint64_t frame_time = util_get_system_time_ns() - start_time;
		printf("Frame time:% " PRIu64 "\n", frame_time);
		frameCount++;
		util_sleepto_ns(time_target);
	}

	uint64_t end_time = util_get_system_time_ns() - init_time;
	printf("Total Time: %" PRIu64 "\n", end_time);
}

/// <summary>
/// Add new bitmap to linked list
/// </summary>
/// <param name="c"></param>
/// <param name="bmp"></param>
void capture_add_bmp(struct capture_capture* c, char *rgb) {
	struct capture_bmp_node *newNode = malloc(sizeof(struct capture_bmp_node));
	newNode->next = NULL;
	// newNode->bmp = bmp;
	newNode->rgb = rgb;

	if (c->bmp_node_first == NULL) {
		c->bmp_node_first = newNode;
		c->bmp_node_last = c->bmp_node_first;
	} else {
		c->bmp_node_last->next = newNode;
		c->bmp_node_last = newNode;
	}
}

/// <summary>
/// Write all all frames to individual bitmap files. Used for debug purposes.
/// </summary>
/// <param name="c"></param>
void capture_write_frames_to_bitmaps(struct capture_capture* c) {
	struct capture_bmp_node* node = c->bmp_node_first;

	int count = 0;
	for (;;) {
		if (node == NULL) {
			break;
		}

		char buf[20];
		snprintf(buf, 20, "test%d.bmp", count);
		util_write_bitmap(node->bmp, buf);

		node = node->next;
		count++;
	}
}
