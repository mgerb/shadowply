#include "runner.h"
#include "encoder.h";
#include "capture/dc_capture.h"
#include "util.h"
#include "libav.h"
#include <libavformat/avformat.h>
#include <stdbool.h>
#include <pthread.h>

void runner_init(runner* r, char* window_title, int fps, int bit_rate) {
	memset(r, 0, sizeof(runner));

	r->exit_threads = false;
	r->fps = fps;

	// init capture object
	r->dc_capture = malloc(sizeof(dc_capture));
	dc_capture_init(r->dc_capture, window_title);

	// init encoder object
	r->encoder = malloc(sizeof(encoder));
	encoder_init(r->encoder, r->dc_capture->width, r->dc_capture->height, fps, AV_CODEC_ID_H264, bit_rate);

	// init mutex for threading
	pthread_mutex_init(&r->mutex, NULL);

	// init texture frame
	r->texture_frame = av_frame_alloc();
	r->texture_frame->format = r->encoder->ctx->pix_fmt;
	r->texture_frame->width = r->encoder->ctx->width;
	r->texture_frame->height = r->encoder->ctx->height;

	int ret = av_frame_get_buffer(r->texture_frame, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}
}

void runner_free(runner* r) {
	// free linked list
	runner_pkt_node* node = r->pkt_list.head;
	while (node != NULL) {
		runner_pkt_node* tmp = node;
		node = node->next;
		av_packet_unref(tmp->pkt);
		av_packet_free(&tmp->pkt);
		free(tmp);
	}

	av_frame_free(&r->texture_frame);

	dc_capture_free(r->dc_capture);
	encoder_free(r->encoder);

	free(r);
}

void runner_start(runner* r) {
	// start threads
	pthread_create(&r->capture_thread, NULL, &runner_start_capture_loop, r);
	pthread_create(&r->encoder_thread, NULL, &runner_start_encoder_loop, r);
}

void runner_stop(runner* r) {
	pthread_mutex_lock(&r->mutex);
	r->exit_threads = true;
	pthread_mutex_unlock(&r->mutex);

	pthread_join(r->encoder_thread, NULL);
	pthread_join(r->capture_thread, NULL);
}

void runner_start_capture_loop(runner* r) {
	uint64_t nanoseconds_per_frame = util_get_nanoseconds_per_frame(r->fps);
	uint64_t init_time = util_get_system_time_ns();

	int max = 0;

	for (;;) {
		uint64_t start_time = util_get_system_time_ns();
		uint64_t time_target = start_time + nanoseconds_per_frame;
		dc_capture_tick(r->dc_capture);

		pthread_mutex_lock(&r->mutex);
		// copy frame data to texture frame
		av_frame_copy(r->texture_frame, r->dc_capture->frame);
		r->dc_capture->has_data = true;
		bool exit = r->exit_threads;
		pthread_mutex_unlock(&r->mutex);

		int frame_time = util_get_system_time_ns() - start_time;
		max = frame_time > max ? frame_time : max;
		printf("Capture time: %" PRIu64 "\n", frame_time);

		if (exit) {
			printf("Max capture time: %" PRIu64 "\n", max);
			return;
		}

		util_sleepto_ns(time_target);
	}
}

void runner_start_encoder_loop(runner* r) {
	uint64_t nanoseconds_per_frame = util_get_nanoseconds_per_frame(r->fps);
	uint64_t init_time = util_get_system_time_ns();

	int frameCount = 1;

	int max = 0;

	for (;;) {

		// wait for data to not be null
		if (!r->dc_capture->has_data) {
			continue;
		}

		uint64_t start_time = util_get_system_time_ns();
		uint64_t time_target = start_time + nanoseconds_per_frame;

		if (r->pkt_list.tail == NULL) {
			runner_add_pkt(r, av_packet_alloc());
		}

		pthread_mutex_lock(&r->mutex);
		// copy data from texture frame into encoder frame
		av_frame_copy(r->encoder->frame, r->texture_frame);
		bool exit = r->exit_threads;
		pthread_mutex_unlock(&r->mutex);

		// encode frame into last packet in list
		bool new_pkt = encoder_encode_frame(r->encoder, r->pkt_list.tail->pkt);

		// add new packet to list if packet is full
		if (new_pkt) {
			runner_add_pkt(r, av_packet_alloc());
		}

		int frame_time = util_get_system_time_ns() - start_time;
		max = frame_time > max ? frame_time : max;
		printf("Encoder time: %" PRIu64 "\n", frame_time);
		frameCount++;

		if (exit) {
			printf("Max encoder time: %" PRIu64 "\n", max);
			return;
		}

		util_sleepto_ns(time_target);
	}
}

void runner_add_pkt(runner* r, AVPacket* pkt) {
	runner_pkt_node* node = malloc(sizeof(runner_pkt_node));
	node->pkt = pkt;
	node->next = NULL;

	if (r->pkt_list.tail == NULL) {
		r->pkt_list.head = node;
		r->pkt_list.tail = node;
	}
	else {
		r->pkt_list.tail->next = node;
		r->pkt_list.tail = node;
	}
}

void runner_write_packets(runner* r, char* fileName) {
	AVFormatContext* formatContext;
	avformat_alloc_output_context2(&formatContext, NULL, NULL, fileName);

	// flush encoder
	libav_encode_frame(r->encoder->ctx, NULL, r->pkt_list.tail->pkt);

	FILE* outfile;
	outfile = fopen(fileName, "wb");

	runner_pkt_node* node = r->pkt_list.head;

	// iterate through list and write packets to file
	while (node != NULL) {
		// printf("Write packet %3"PRId64" (size=%5d)\n", node->pkt->pts, node->pkt->size);
		fwrite(node->pkt->data, 1, node->pkt->size, outfile);
		av_packet_unref(node->pkt);

		node = node->next;
	}

	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	fwrite(endcode, 1, sizeof(endcode), outfile);

	fclose(outfile);
}

