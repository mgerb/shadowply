#include <stdio.h>
#include "shadowply.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include "util.h"
#include "libav.h"
#include "runner.h"
#include <pthread.h>
#include <windows.h>

static char* windowN = "AssaultCube";
static int fps = 60;
static int bit_rate = 20000000;
static int max_seconds = 10;

int main()
{

	// register hotkey
	if (RegisterHotKey(NULL, 1, MOD_ALT | MOD_NOREPEAT, 0x42))    //0x42 is 'b'
	{
		wprintf(L"Hotkey 'alt+b' registered, using MOD_NOREPEAT flag\n");
	}

	avcodec_register_all();
	av_register_all();

	int count = 0;

	// init runner
	runner* r = malloc(sizeof(runner));
	runner_init(r, windowN, fps, bit_rate, max_seconds);
	// start runner
	runner_start(r);

	MSG msg;
	bool starting = false;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_HOTKEY && !starting)
		{
			wprintf(L"WM_HOTKEY received\n");
			starting = true;
			// wait for runner to stop
			runner_stop(r);

			char dst[20] = "out";
			itoa(count, dst + 3, 10);
			strcat(dst, ".h264");
			runner_write_packets(r, dst);
			runner_free(r);
			r = malloc(sizeof(runner));
			runner_init(r, windowN, fps, bit_rate, max_seconds);
			// start runner
			runner_start(r);
			starting = false;
			count++;
		}
	}

	return 0;
}

