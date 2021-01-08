#include <stdio.h>
#include "shadowply.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include "util.h"
#include "libav.h"
#include "runner.h"
#include <pthread.h>

static char* windowN = "AssaultCube";
static int fps = 60;
static int bit_rate = 10000000;

int main()
{
	avcodec_register_all();
	av_register_all();

	int count = 0;

	for (;;) {
		// init runner
		runner* r = malloc(sizeof(runner));
		runner_init(r, windowN, fps, bit_rate);

		// start runner
		runner_start(r);

		// sleep 10 seconds
		Sleep(10000);

		// wait for runner to stop
		runner_stop(r);

		char dst[20] = "out";
		itoa(count, dst + 3, 10);
		strcat(dst, ".h264");
		runner_write_packets(r, dst);

		runner_free(r);
		count++;
	}

	return 0;
}

