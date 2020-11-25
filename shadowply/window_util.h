#pragma once

struct window_util_window {
	char* title;
	HWND window;
};

extern HWND window_util_find_window(const char* title);

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

