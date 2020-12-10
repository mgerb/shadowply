#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include <libavcodec/avcodec.h>


void libav_yuv_from_rgb(AVCodecContext* context, AVFrame* frame, uint8_t* rgb);
bool libav_encode_frame(AVCodecContext* context, AVFrame* frame, AVPacket* pkt);

void ffmpeg_encoder_set_frame_yuv_from_rgb(uint8_t* rgb);
void encode_example(const char* filename, int codec_id);
void ffmpeg_encoder_encode_frame(uint8_t* rgb);
void ffmpeg_encoder_finish(void);
void ffmpeg_encoder_start(const char* filename, int codec_id, int fps, int width, int height);
uint8_t* generate_rgb(int width, int height, int pts, uint8_t* rgb);