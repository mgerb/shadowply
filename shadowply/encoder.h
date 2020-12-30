#pragma once
#include <libavcodec/avcodec.h>
#include <stdbool.h>

typedef struct encoder {
	int frame_count;
	AVCodecContext* ctx;
	AVFrame* frame;
} encoder;


void encoder_init(encoder* e, int width, int height, int fps, int avcodec_id, int bit_rate);
void encoder_free(encoder* e);
bool encoder_encode_frame(encoder* e, AVPacket* pkt);

