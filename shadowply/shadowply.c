#include <stdio.h>
#include "shadowply.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include "capture.h"
#include "util.h"
#include "libav.h"

static char* windowN = "AssaultCube";
static int fps = 30;

int main()
{
	avcodec_register_all();
	av_register_all();

	struct capture_capture* c = malloc(sizeof(struct capture_capture));

	// init
	capture_init(c, windowN, fps, AV_CODEC_ID_H264);
	capture_start_capture_loop(c);
	// capture_write_frames_to_bitmaps(c);

	capture_write_packets(c);

	// encode_example("tmp.h264", AV_CODEC_ID_H264);
	// encode_example("tmp.mpg", AV_CODEC_ID_MPEG1VIDEO);


	// TODO:
	// - convert bitmap rgp data to yuv420
	// - look at ffmpeg encode example

	capture_free(c);

	return 0;
}

