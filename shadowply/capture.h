#pragma once
#include <Windows.h>

struct capture_bmp_node {
	char* rgb;
	HBITMAP bmp;
	struct capture_bmp_node *next;
};

struct capture_capture {
	int width;
	int height;
	int x;
	int y;
	HWND window;
	HDC hdc;
	int fps;
	// bitmap linked list
	struct capture_bmp_node *bmp_node_first;
	struct capture_bmp_node *bmp_node_last;
};

void capture_init(struct capture_capture* c, const char* title, int fps);
void capture_write_frames_to_bitmaps(struct capture_capture* c);
void capture_free(struct capture_capture* c);
char* capture_frame(struct capture_capture* c);
void capture_add_bmp(struct capture_capture* c, char *bmp);


