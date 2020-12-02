#include <stdio.h>
#include "shadowply.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include "capture.h"
#include "util.h"

static char* windowN = "Windows PowerShell";
static int fps = 30;

int main()
{
	const AVCodec* codec;
	AVCodecContext *context = NULL;
	
	codec = avcodec_find_encoder_by_name("libx264");
	context = avcodec_alloc_context3(codec);

	struct capture_capture* c = malloc(sizeof(struct capture_capture));

	// init
	capture_init(c, windowN, fps);
	capture_start_capture_loop(c);
	// capture_write_frames_to_bitmaps(c);


	// TODO:
	// - convert bitmap rgp data to yuv420
	// - look at ffmpeg encode example

	capture_free(c);

	avcodec_free_context(&context);

	return 0;
}

