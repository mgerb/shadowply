#include <stdio.h>
#include <Windows.h>
#include "capture.h"
#include "util.h"
#include "window_util.h"

void capture_init(struct capture_capture* c, const char* title) {
	memset(c, 0, sizeof(struct capture_capture));

	c->window = window_util_find_window(title);
	c->hdc_target = NULL;

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
	free(c);
}

void capture_frame(struct capture_capture* c) {

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

	// char buf[20];
	// snprintf(buf, 20, "test%d.bmp", 1);
	util_write_bitmap(hBitmap, "test.bmp");
}

