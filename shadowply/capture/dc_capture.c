#include "dc_capture.h"
#include "../window_util.h"

void dc_capture_init(dc_capture* c, char* title) {
	memset(c, 0, sizeof(dc_capture));

	c->window = window_util_find_window(title);

	RECT rect;
	int width = 0, height = 0;

	if (GetClientRect(c->window, &rect)) {
		c->width = rect.right - rect.left;
		c->height = rect.bottom - rect.top;
		// c->x = 0; // TODO:
		// c->y = 0;
	}

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;
	bmi.biWidth = c->width;
	bmi.biHeight = -c->height; // must be negative or image is flipped
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = c->width * c->height * 3;

	c->bmi = bmi;
}

void dc_capture_free(dc_capture* c) {
	ReleaseDC(c->window, c->hdc);
	DeleteObject(c->hdc);
	DeleteObject(c->window);
	free(c);
}

void dc_capture_tick(dc_capture* c) {
	HDC hdc_target = GetDC(c->window);

	if (c->hdc == NULL) {
		c->hdc = CreateCompatibleDC(hdc_target);
	}

	if (c->hBitmap == NULL) {
		c->hBitmap = CreateCompatibleBitmap(hdc_target, c->width, c->height);
		SelectObject(c->hdc, c->hBitmap);
	}

	BitBlt(c->hdc, 0, 0, c->width, c->height, hdc_target, 0, 0, SRCCOPY);

	if (c->rgb == NULL) {
		c->rgb = malloc(c->bmi.biSizeImage);
	}

	GetDIBits(c->hdc, c->hBitmap, 0, c->height, c->rgb, (BITMAPINFO*)&c->bmi, DIB_RGB_COLORS);

	ReleaseDC(NULL, hdc_target);
}