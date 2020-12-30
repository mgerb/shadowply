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

	runner* r = malloc(sizeof(runner));

	runner_init(r, windowN, fps, bit_rate);

	//runner_start_capture_loop(r);
	//runner_start_encoder_loop(r);

	 pthread_create(&r->capture_thread, NULL, &runner_start_capture_loop, r);
	 pthread_create(&r->encoder_thread, NULL, &runner_start_encoder_loop, r);

	// wait for threads
	 //pthread_join(r->capture_thread, NULL);

	pthread_join(r->encoder_thread, NULL);

	// TODO: kill capture thread

	//runner_start_loop(r);
	runner_write_packets(r, "out.h264");

	runner_free(r);

	return 0;
}

