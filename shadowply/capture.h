#pragma once
#include <Windows.h>
#include <libavcodec/avcodec.h>

struct capture_pkt_node {
	AVPacket* pkt;
	struct capture_pkt_node *next;
};

struct capture_capture {
	int width;
	int height;
	int x;
	int y;
	HWND window;
	HDC hdc;
	int fps;

	AVCodecContext* codec_ctx;
	AVFrame* frame;
	struct capture_pkt_node* pkt_node;
	struct capture_pkt_node* pkt_node_last;
};

void capture_init(struct capture_capture* c, const char* title, int fps);
void capture_write_frames_to_bitmaps(struct capture_capture* c);
void capture_free(struct capture_capture* c);
void capture_capture_frame(struct capture_capture* c);
void capture_add_pkt(struct capture_capture* c, AVPacket* pkt);
void capture_write_packets(struct capture_capture* c);


