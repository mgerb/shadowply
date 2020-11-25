#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>

#define _POSIX_C_SOURCE 200809L

static inline uint64_t util_get_clockfreq();
uint64_t util_get_system_time_ns();
bool util_sleepto_ns(uint64_t time_target);
uint64_t util_get_nanoseconds_per_frame(int fps);

// get system time in milliseconds
long long util_get_system_time();

bool util_write_bitmap(HBITMAP hBitmap, LPCTSTR lpszFileName);

