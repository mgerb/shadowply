#include <stdio.h>
#include <Windows.h>
#include "capture.h"
#include "util.h"
#include "window_util.h"
#include <inttypes.h>

void capture_init(struct capture_capture* c, const char* title, int fps) {
	memset(c, 0, sizeof(struct capture_capture));

	c->window = window_util_find_window(title);
	c->hdc_target = NULL;
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
	for (;;) {
		if (node == NULL) {
			break;
		}
		struct capture_bmp_node* tmp = node;
		node = node->next;
		DeleteObject(tmp->bmp);
		free(tmp);
	}
	DeleteObject(c->window);
	DeleteObject(c->hdc_target);
	free(c);
}

HBITMAP capture_frame(struct capture_capture* c) {

	if (c->hdc_target == NULL) {
		c->hdc_target = GetDC(c->window);
	}

	HDC hdc = CreateCompatibleDC(c->hdc_target);

	HBITMAP hBitmap = CreateCompatibleBitmap(c->hdc_target, c->width, c->height);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

	BitBlt(hdc, 0, 0, c->width, c->height, c->hdc_target, c->x, c->y, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hdc, hOldBitmap);

	ReleaseDC(NULL, c->hdc_target);
	c->hdc_target = NULL;
	ReleaseDC(NULL, hdc);

	return hBitmap;
}

void capture_start_capture_loop(struct capture_capture* c) {
	uint64_t nanoseconds_per_frame = util_get_nanoseconds_per_frame(c->fps);
	uint64_t init_time = util_get_system_time_ns();
	int frameCount = 0;
	// capture 1 second per iteration
	for (;;) {
		if (frameCount >= c->fps * 10) {
			break;
		}
		uint64_t start_time = util_get_system_time_ns();
		uint64_t time_target = start_time + nanoseconds_per_frame;

		// TODO: handle if frame capture takes longer than time target
		HBITMAP bmp = capture_frame(c);
		capture_add_bmp(c, bmp);

		uint64_t frame_time = util_get_system_time_ns() - start_time;
		printf("Frame time:% " PRIu64 "\n", frame_time);
		frameCount++;
		util_sleepto_ns(time_target);
	}

	uint64_t end_time = util_get_system_time_ns() - init_time;
	printf("Total Time: %" PRIu64 "\n", end_time);
}

void capture_add_bmp(struct capture_capture* c, HBITMAP bmp) {
	struct capture_bmp_node *newNode = malloc(sizeof(struct capture_bmp_node));
	newNode->next = NULL;
	newNode->bmp = bmp;

	if (c->bmp_node_first == NULL) {
		c->bmp_node_first = newNode;
		c->bmp_node_last = c->bmp_node_first;
	} else {
		c->bmp_node_last->next = newNode;
		c->bmp_node_last = newNode;
	}
}

