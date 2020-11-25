#pragma once
#include <Windows.h>

struct capture_capture {
	int width;
	int height;
	int x;
	int y;
	HWND window;
	HDC hdc_target;
};

extern void capture_init(struct capture_capture* c, const char* title);
extern void capture_free(struct capture_capture* c);
extern void capture_frame(struct capture_capture* c);


