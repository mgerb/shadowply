#include "libav.h"
#include <inttypes.h>
#include <stdbool.h>

#include <libswscale/version.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

struct SwsContext* sws_context = NULL;

void libav_rgb_to_yuv(AVFrame* frame, uint8_t* rgb, int width, int height) {
	const int in_linesize[3] = { 4 * width, 0, 0 };
	sws_context = sws_getCachedContext(sws_context,
		width, height, AV_PIX_FMT_RGB32,
		width, height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
	sws_scale(sws_context, (const uint8_t* const*)&rgb, in_linesize, 0,
		height, frame->data, frame->linesize);
}

// return true if packet has been filled
bool libav_encode_frame(AVCodecContext* context, AVFrame* frame, AVPacket* pkt) {
    int ret;
    /* send the frame to the encoder */
    //if (frame) {
    //    printf("Send frame %3"PRId64"\n", frame->pts);
    //}
    ret = avcodec_send_frame(context, frame);
    if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding: %s\n", av_err2str(ret));
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(context, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return false;
        } else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        // printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        return true;
    }
}

// not tested  yet - might not work
AVFrame* libav_deep_copy_frame(AVFrame* frame) {
    AVFrame* copyFrame = av_frame_alloc();
    copyFrame->format = frame->format;
    copyFrame->width = frame->width;
    copyFrame->height = frame->height;
    copyFrame->channels = frame->channels;
    copyFrame->channel_layout = frame->channel_layout;
    copyFrame->nb_samples = frame->nb_samples;
    av_frame_get_buffer(copyFrame, 0);
    av_frame_copy(copyFrame, frame);
    av_frame_copy_props(copyFrame, frame);
}

//void ffmpeg_encoder_set_frame_yuv_from_rgb(uint8_t* rgb) {
//	const int in_linesize[1] = { 3 * c->width };
//	sws_context = sws_getCachedContext(sws_context,
//		c->width, c->height, AV_PIX_FMT_RGB24,
//		c->width, c->height, AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
//	sws_scale(sws_context, (const uint8_t* const*)&rgb, in_linesize, 0,
//		c->height, frame->data, frame->linesize);
//}

//uint8_t* generate_rgb(int width, int height, int pts, uint8_t* rgb) {
//    int x, y, cur;
//    rgb = realloc(rgb, 3 * sizeof(uint8_t) * height * width);
//    for (y = 0; y < height; y++) {
//        for (x = 0; x < width; x++) {
//            cur = 3 * (y * width + x);
//            rgb[cur + 0] = 0;
//            rgb[cur + 1] = 0;
//            rgb[cur + 2] = 0;
//            if ((frame->pts / 25) % 2 == 0) {
//                if (y < height / 2) {
//                    if (x < width / 2) {
//                        /* Black. */
//                    }
//                    else {
//                        rgb[cur + 0] = 255;
//                    }
//                }
//                else {
//                    if (x < width / 2) {
//                        rgb[cur + 1] = 255;
//                    }
//                    else {
//                        rgb[cur + 2] = 255;
//                    }
//                }
//            }
//            else {
//                if (y < height / 2) {
//                    rgb[cur + 0] = 255;
//                    if (x < width / 2) {
//                        rgb[cur + 1] = 255;
//                    }
//                    else {
//                        rgb[cur + 2] = 255;
//                    }
//                }
//                else {
//                    if (x < width / 2) {
//                        rgb[cur + 1] = 255;
//                        rgb[cur + 2] = 255;
//                    }
//                    else {
//                        rgb[cur + 0] = 255;
//                        rgb[cur + 1] = 255;
//                        rgb[cur + 2] = 255;
//                    }
//                }
//            }
//        }
//    }
//    return rgb;
//}

/* Allocate resources and write header data to the output file. */
//void ffmpeg_encoder_start(const char* filename, int codec_id, int fps, int width, int height) {
//    AVCodec* codec;
//    int ret;
//
//    codec = avcodec_find_encoder(codec_id);
//    if (!codec) {
//        fprintf(stderr, "Codec not found\n");
//        exit(1);
//    }
//    c = avcodec_alloc_context3(codec);
//    if (!c) {
//        fprintf(stderr, "Could not allocate video codec context\n");
//        exit(1);
//    }
//    c->bit_rate = 400000;
//    c->width = width;
//    c->height = height;
//    c->time_base.num = 1;
//    c->time_base.den = fps;
//    c->gop_size = 10;
//    c->max_b_frames = 1;
//    c->pix_fmt = AV_PIX_FMT_YUV420P;
//    if (codec_id == AV_CODEC_ID_H264)
//        av_opt_set(c->priv_data, "preset", "slow", 0);
//    if (avcodec_open2(c, codec, NULL) < 0) {
//        fprintf(stderr, "Could not open codec\n");
//        exit(1);
//    }
//    file = fopen(filename, "wb");
//    if (!file) {
//        fprintf(stderr, "Could not open %s\n", filename);
//        exit(1);
//    }
//    frame = av_frame_alloc();
//    if (!frame) {
//        fprintf(stderr, "Could not allocate video frame\n");
//        exit(1);
//    }
//    frame->format = c->pix_fmt;
//    frame->width = c->width;
//    frame->height = c->height;
//    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
//    if (ret < 0) {
//        fprintf(stderr, "Could not allocate raw picture buffer\n");
//        exit(1);
//    }
//}

/*
Write trailing data to the output file
and free resources allocated by ffmpeg_encoder_start.
*/
//void ffmpeg_encoder_finish(void) {
//    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
//    int got_output, ret;
//    do {
//        fflush(stdout);
//        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
//        if (ret < 0) {
//            fprintf(stderr, "Error encoding frame\n");
//            exit(1);
//        }
//        if (got_output) {
//            fwrite(pkt.data, 1, pkt.size, file);
//            av_packet_unref(&pkt);
//        }
//    } while (got_output);
//    fwrite(endcode, 1, sizeof(endcode), file);
//    fclose(file);
//    avcodec_close(c);
//    av_free(c);
//    av_freep(&frame->data[0]);
//    av_frame_free(&frame);
//}

/*
Encode one frame from an RGB24 input and save it to the output file.
Must be called after ffmpeg_encoder_start, and ffmpeg_encoder_finish
must be called after the last call to this function.
*/
//void ffmpeg_encoder_encode_frame(uint8_t* rgb) {
//    int ret, got_output;
//    ffmpeg_encoder_set_frame_yuv_from_rgb(rgb);
//    av_init_packet(&pkt);
//    pkt.data = NULL;
//    pkt.size = 0;
//    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
//    if (ret < 0) {
//        fprintf(stderr, "Error encoding frame\n");
//        exit(1);
//    }
//    if (got_output) {
//        fwrite(pkt.data, 1, pkt.size, file);
//        av_packet_unref(&pkt);
//    }
//}

//void encode_example(const char* filename, int codec_id) {
//    int pts;
//    int width = 320;
//    int height = 240;
//    uint8_t* rgb = NULL;
//    ffmpeg_encoder_start(filename, codec_id, 25, width, height);
//    for (pts = 0; pts < 100; pts++) {
//        frame->pts = pts;
//        rgb = generate_rgb(width, height, pts, rgb);
//        ffmpeg_encoder_encode_frame(rgb);
//    }
//    ffmpeg_encoder_finish();
//}
