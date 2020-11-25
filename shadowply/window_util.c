#include <Windows.h>
#include "window_util.h"

HWND window_util_find_window(const char* title) {
	struct window_util_window* window = &(struct window_util_window) { title };
	EnumWindows(EnumWindowsProc, window);
	return window->window;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	struct window_util_window* window = (struct window_util_window*)lParam;

	char buffer[256];
	int written = GetWindowTextA(hwnd, buffer, 256);

	if (strstr(buffer, window->title) != NULL) {
		window->window = hwnd;
		return FALSE;
	}

	return TRUE;
}

