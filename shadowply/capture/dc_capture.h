#pragma once
#include <Windows.h>
#include <inttypes.h>

typedef struct dc_capture {
	int width;
	int height;
	HDC hdc;
	HWND window;
	HBITMAP hBitmap;
	// RGB storage for current captured frame
	uint8_t* rgb;
	BITMAPINFOHEADER bmi;
} dc_capture;

void dc_capture_init(dc_capture* c, char* title);
void dc_capture_free(dc_capture* c);
void dc_capture_tick(dc_capture* c);

