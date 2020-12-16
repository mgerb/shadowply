#include "encoder.h"
#include <libavcodec/avcodec.h>
#include <stdbool.h>
#include "libav.h"

void encoder_init(encoder* e, int width, int height, int fps, int avcodec_id, int bit_rate) {
	memset(e, 0, sizeof(encoder));

	e->frame_count = 0;

	AVCodec* codec = avcodec_find_encoder(avcodec_id);
	if (!codec) {
		fprintf(stderr, "Codec '%d' not found\n", avcodec_id);
		exit(1);
	}

	e->ctx = avcodec_alloc_context3(codec);
	if (!e->ctx) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	e->ctx->bit_rate = bit_rate;
	e->ctx->width = width;
	e->ctx->height = height;
	e->ctx->time_base = (AVRational){ 1, fps };
	e->ctx->framerate = (AVRational){ fps, 1 };
	e->ctx->gop_size = 10;
	e->ctx->max_b_frames = 1;
	e->ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264) {
		av_opt_set(e->ctx->priv_data, "preset", "ultrafast", 0);
	}

	int ret = avcodec_open2(e->ctx, codec, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
		exit(1);
	}

	e->frame = av_frame_alloc();
	e->frame->format = e->ctx->pix_fmt;
	e->frame->width = width;
	e->frame->height = height;

	ret = av_frame_get_buffer(e->frame, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}
}

void encoder_free(encoder* e) {
	av_free(e->ctx);
	av_frame_free(&e->frame);
	free(e);
}

bool encoder_encode_rgb(encoder* e, AVPacket* pkt, uint8_t* rgb) {
	e->frame->pts = e->frame_count++;
	libav_yuv_from_rgb(e->ctx, e->frame, rgb);
	return libav_encode_frame(e->ctx, e->frame, pkt);
}

