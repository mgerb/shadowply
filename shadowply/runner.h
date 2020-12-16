#pragma once

#include "capture/dc_capture.h"
#include "encoder.h"

typedef struct runner_pkt_node {
	AVPacket* pkt;
	struct runner_pkt_node* next;
} runner_pkt_node;

typedef struct runner_pkt_list {
	runner_pkt_node* head;
	runner_pkt_node* tail;
} runner_pkt_list;

typedef struct runner {
	int fps;
	encoder* encoder;
	dc_capture* dc_capture;
	runner_pkt_list pkt_list;
} runner;

void runner_init(runner* r, char* window_title, int fps);
void runner_free(runner* r);
void runner_start_loop(runner* r);
void runner_tick(runner* r);
void runner_add_pkt(runner* r, AVPacket* pkt);
void runner_write_packets(runner* r, char* fileName);

