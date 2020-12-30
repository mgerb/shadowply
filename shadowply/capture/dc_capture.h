#pragma once
#include <Windows.h>
#include <inttypes.h>
#include <libavcodec/avcodec.h>
#include <stdbool.h>

typedef struct dc_capture {
	int width;
	int height;
	HDC hdc;
	HWND window;
	HBITMAP hBitmap;
	// RGB storage for current captured frame
	uint8_t* rgb;
	BITMAPINFOHEADER bmi;
	// convert and store rgb data in frame
	AVFrame* frame;
	// yuv data
	uint8_t* data;
	bool has_data;

} dc_capture;

void dc_capture_init(dc_capture* c, char* title);
void dc_capture_free(dc_capture* c);

// capture bits from window and convert to yuv format
void dc_capture_tick(dc_capture* c);

