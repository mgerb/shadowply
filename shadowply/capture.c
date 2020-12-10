#include <stdio.h>
#include <Windows.h>
#include "capture.h"
#include "util.h"
#include "window_util.h"
#include <inttypes.h>
#include "libav.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>

static int frameCount = 0;

void capture_init(struct capture_capture* c, const char* title, int fps, int avcodec_id) {
	memset(c, 0, sizeof(struct capture_capture));

	c->window = window_util_find_window(title);
	c->hdc = NULL;
	c->fps = fps;

	RECT rect;
	int width = 0, height = 0;

	if (GetClientRect(c->window, &rect)) {
		c->width = rect.right - rect.left;
		c->height = rect.bottom - rect.top;
		c->x = 0; // TODO:
		c->y = 0;
	}
    AVCodec* codec = avcodec_find_encoder(avcodec_id);
    // AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MPEG2VIDEO);
	// AVCodec* codec = avcodec_find_encoder_by_name("libx264rgb");
	// AVCodec* codec = avcodec_find_encoder_by_name("mpeg1");
	if (!codec) {
		fprintf(stderr, "Codec '%d' not found\n", avcodec_id);
		exit(1);
	}

	c->codec_ctx = avcodec_alloc_context3(codec);
	if (!c->codec_ctx) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

    c->codec_ctx->bit_rate = 5000000;
    c->codec_ctx->width = c->width;
    c->codec_ctx->height = c->height;
    c->codec_ctx->time_base = (AVRational){ 1, fps };
    c->codec_ctx->framerate = (AVRational){ fps, 1 };
    c->codec_ctx->gop_size = 10;
    c->codec_ctx->max_b_frames = 1;
    c->codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    // c->codec_ctx->pix_fmt = AV_PIX_FMT_RGB24;
    // c->codec_ctx->pix_fmt = AV_PIX_FMT_RGB32;

	if (codec->id == AV_CODEC_ID_H264) {
		av_opt_set(c->codec_ctx->priv_data, "preset", "ultrafast", 0);
	}

	int ret = avcodec_open2(c->codec_ctx, codec, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
		exit(1);
	}

	c->frame = av_frame_alloc();
    c->frame->format = c->codec_ctx->pix_fmt;
    c->frame->width = c->codec_ctx->width;
    c->frame->height = c->codec_ctx->height;

	ret = av_frame_get_buffer(c->frame, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}

	return c;
}

void capture_free(struct capture_capture* c) {
	struct capture_pkt_node* node = c->pkt_node;
	// free up linked list
	for (;;) {
		if (node == NULL) {
			break;
		}
		struct capture_pkt_node* tmp = node;
		node = tmp->next;
		// av_freep(&tmp->frame->data[0]);
		// av_frame_free(&tmp->frame);

		// TODO: unref packet after writing file
		// av_packet_unref(tmp->pkt);

		av_packet_free(&tmp->pkt);
		free(tmp);
	}

	// TODO: check if this is deprecated or removed?
	// av_codec_close(c->codec_ctx);
	av_free(c->codec_ctx);

	av_frame_free(&c->frame);
	ReleaseDC(c->window, c->hdc);
	CloseWindow(c->window);
	DeleteObject(c->window);
	DeleteObject(c->hdc);

	free(c);
}

void capture_capture_frame(struct capture_capture* c) {

	HDC hdc_target = GetDC(c->window);

	if (c->hdc == NULL) {
		c->hdc = CreateCompatibleDC(hdc_target);
	}

	BITMAPINFO MyBMInfo = { 0 };
	MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

	HBITMAP hBitmap = CreateCompatibleBitmap(hdc_target, c->width, c->height);
	SelectObject(c->hdc, hBitmap);

	BitBlt(c->hdc, 0, 0, c->width, c->height, hdc_target, c->x, c->y, SRCCOPY);

	const h = c->width* c->height * 3;

	GetDIBits(c->hdc, hBitmap, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS);

	BYTE *rgb = malloc(MyBMInfo.bmiHeader.biSizeImage);

	MyBMInfo.bmiHeader.biCompression = BI_RGB;

	GetDIBits(c->hdc, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, rgb, &MyBMInfo, DIB_RGB_COLORS);

	c->frame->pts = frameCount++;

	// fflush(stdout);
	// av_frame_make_writable(c->frame);

	if (c->pkt_node_last == NULL) {
		capture_add_pkt(c, av_packet_alloc());
	}

	libav_yuv_from_rgb(c->codec_ctx, c->frame, rgb);

	bool enc = libav_encode_frame(c->codec_ctx, c->frame, c->pkt_node_last->pkt);

	if (enc) {
		capture_add_pkt(c, av_packet_alloc());
	}

	// cleanup
	// av_frame_free(&frame);
	free(rgb);
	ReleaseDC(NULL, hdc_target);
	DeleteObject(hBitmap);
}

// start capturing frames - sleep based on fps
void capture_start_capture_loop(struct capture_capture* c) {
	uint64_t nanoseconds_per_frame = util_get_nanoseconds_per_frame(c->fps);
	uint64_t init_time = util_get_system_time_ns();
	int frameCount = 0;

	for (;;) {
		if (frameCount >= c->fps * 10) {
			break;
		}
		uint64_t start_time = util_get_system_time_ns();
		uint64_t time_target = start_time + nanoseconds_per_frame;

		// TODO: handle if frame capture takes longer than time target
		capture_capture_frame(c);

		uint64_t frame_time = util_get_system_time_ns() - start_time;
		printf("Frame time:% " PRIu64 "\n", frame_time);
		frameCount++;
		util_sleepto_ns(time_target);
	}

	uint64_t end_time = util_get_system_time_ns() - init_time;
	printf("Total Time: %" PRIu64 "\n", end_time);
}

void capture_add_pkt(struct capture_capture* c, AVPacket* pkt) {
	struct capture_pkt_node* node = malloc(sizeof(struct capture_pkt_node));
	node->pkt = pkt;
	node->next = NULL;

	if (c->pkt_node == NULL) {
		c->pkt_node = node;
		c->pkt_node_last = c->pkt_node;
	} else {
		c->pkt_node_last->next = node;
		c->pkt_node_last = node;
	}
}

void capture_write_packets(struct capture_capture* c) {

	char* fileName = "output.h264";

	AVFormatContext* formatContext;
	avformat_alloc_output_context2(&formatContext, NULL, NULL, fileName);

	// flush encoder
	libav_encode_frame(c->codec_ctx, NULL, c->pkt_node_last->pkt);

	FILE* outfile;
	outfile = fopen(fileName, "wb");

	struct capture_pkt_node* node = c->pkt_node;

	// iterate through list and write packets to file
	while (node != NULL) {

		printf("Write packet %3"PRId64" (size=%5d)\n", node->pkt->pts, node->pkt->size);
		fwrite(node->pkt->data, 1, node->pkt->size, outfile);
		av_packet_unref(node->pkt);

		node = node->next;
	}

	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	// fwrite(endcode, 1, sizeof(endcode), outfile);

	fclose(outfile);
}

//void capture_write_frames_to_bitmaps(struct capture_capture* c) {
//	struct capture_bmp_node* node = c->bmp_node_first;
//
//	int count = 0;
//	for (;;) {
//		if (node == NULL) {
//			break;
//		}
//
//		char buf[20];
//		snprintf(buf, 20, "test%d.bmp", count);
//		util_write_bitmap(node->bmp, buf);
//
//		node = node->next;
//		count++;
//	}
//}
