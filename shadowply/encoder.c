#include "encoder.h"
#include <libavcodec/avcodec.h>
#include <stdbool.h>
#include "libav.h"
#include "util.h"

void encoder_init(encoder* e, int width, int height, int fps, int bit_rate) {
	memset(e, 0, sizeof(encoder));

	e->frame_count = 0;

	// nvidia encoding
	AVCodec* codec = avcodec_find_encoder_by_name("h264_nvenc");
	e->software_encode = false;

	// default to software encoding if nvenc doesn't exist
	if (codec == NULL) {
		printf("h264_nvenc not available - defaulting to software H264 encode");
		e->software_encode = true;
		codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	}

	e->ctx = avcodec_alloc_context3(codec);
	
	// TODO: handle errors
	if (!e->ctx) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	if (e->software_encode) {
		e->ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	} else {
		e->ctx->pix_fmt = AV_PIX_FMT_0RGB32;
	}

	// TODO: adjust optimal values
	e->ctx->bit_rate = bit_rate;
	e->ctx->width = width;
	e->ctx->height = height;
	e->ctx->time_base = (AVRational){ 1, fps };
	e->ctx->framerate = (AVRational){ fps, 1 };
	e->ctx->gop_size = 20;
	e->ctx->max_b_frames = 0;

	if (codec->id == AV_CODEC_ID_H264) {
		// TODO: software h264 encoding
		// av_opt_set(e->ctx->priv_data, "preset", "ultrafast", 0);
		av_opt_set(e->ctx->priv_data, "tune", "zerolatency", 0);
	}

	int ret = avcodec_open2(e->ctx, codec, NULL);

	// TODO: handle errors
	if (ret < 0) {
		fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
		exit(1);
	}

	e->frame = libav_new_frame(e->ctx->pix_fmt, width, height);
}

void encoder_free(encoder* e) {
	// TODO:
	// av_frame_unref(e->frame);
	av_frame_free(&e->frame);
	avcodec_free_context(&e->ctx);
	free(e);
}

bool encoder_encode_frame(encoder* e, AVPacket* pkt) {
	e->frame->pts = e->frame_count++;
	libav_encode_frame(e->ctx, e->frame, pkt);
}

