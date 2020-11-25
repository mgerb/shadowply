#include <stdio.h>
#include "shadowply.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include "capture.h"

#define FPS 60
static char* windowN = "Windows PowerShell";

int main()
{
	printf("Starting ffmpeg stuff...\n");

	const AVCodec* codec;
	AVCodecContext *context = NULL;
	
	codec = avcodec_find_encoder_by_name("libx264");
	context = avcodec_alloc_context3(codec);

	struct capture_capture* c = malloc(sizeof(struct capture_capture));
	// init
	capture_init(c, windowN);

	int seconds = 0;

	uint64_t nanoseconds_per_frame = util_get_nanoseconds_per_frame(FPS);

	uint64_t init_time = util_get_system_time_ns();
	for (;;) {
		if (seconds == 60) {
			break;
		}
		uint64_t start_time = util_get_system_time_ns();
		uint64_t time_target = start_time + nanoseconds_per_frame;
		// TODO: handle if frame capture takes longer than time target
		capture_frame(c);
		uint64_t frame_time = util_get_system_time_ns() - start_time;
		printf("Frame time: %d\n", frame_time);
		util_sleepto_ns(time_target);
		seconds++;
	}

	uint64_t end_time = util_get_system_time_ns() - init_time;
	printf("Total Time: %d\n", end_time);

	// done
	capture_free(c);
	// avcodec_free_context(context);

	avcodec_free_context(&context);

	return 0;
}

