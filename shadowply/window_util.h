#pragma once

typedef struct window_util_size {
	int width, height;
} window_util_size;

typedef struct window_util_window {
	char* title;
	HWND window;
} window_util_window;

HWND window_util_find_window(const char* title);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
window_util_size window_util_get_size(const char* title);

