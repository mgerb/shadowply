#include <Windows.h>
#include "window_util.h"

window_util_size window_util_get_size(const char* title) {
	HWND window = window_util_find_window(title);
	RECT rect;
	window_util_size size = { 0, 0 };

	if (GetClientRect(window, &rect)) {
		size.width = rect.right - rect.left;
		size.height = rect.bottom - rect.top;
	}

	DeleteObject(window);

	return size;
}

HWND window_util_find_window(const char* title) {
	window_util_window* window = &(window_util_window) { title };
	EnumWindows(EnumWindowsProc, window);
	return window->window;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	window_util_window* window = (window_util_window*)lParam;

	char buffer[256];
	int written = GetWindowTextA(hwnd, buffer, 256);

	if (strstr(buffer, window->title) != NULL) {
		window->window = hwnd;
		return FALSE;
	}

	return TRUE;
}

