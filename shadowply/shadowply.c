#include <stdio.h>
#include "shadowply.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include "util.h"
#include "libav.h"
#include "runner.h"

static char* windowN = "AssaultCube";
static int fps = 60;
static int bit_rate = 5000000;

int main()
{
	runner* r = malloc(sizeof(runner));

	runner_init(r, windowN, fps, bit_rate);

	runner_start_loop(r);
	runner_write_packets(r, "new_output.h264");

	runner_free(r);

	return 0;
}

