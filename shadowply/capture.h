#pragma once
#include <Windows.h>

struct capture_bmp_node {
	HBITMAP bmp;
	struct capture_bmp_node *next;
};

struct capture_capture {
	int width;
	int height;
	int x;
	int y;
	HWND window;
	HDC hdc_target;
	int fps;
	// bitmap linked list
	struct capture_bmp_node *bmp_node_first;
	struct capture_bmp_node *bmp_node_last;
};

extern void capture_add_bmp(struct capture_capture* c, HBITMAP bmp);
extern void capture_init(struct capture_capture* c, const char* title, int fps);
extern void capture_free(struct capture_capture* c);
extern HBITMAP capture_frame(struct capture_capture* c);


