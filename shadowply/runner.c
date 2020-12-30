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

	r->fps = fps;

	r->dc_capture = malloc(sizeof(dc_capture));

	dc_capture_init(r->dc_capture, window_title);

	r->encoder = malloc(sizeof(encoder));
	encoder_init(r->encoder, r->dc_capture->width, r->dc_capture->height, fps, AV_CODEC_ID_H264, bit_rate);

	pthread_mutex_init(&r->mutex, NULL);
}

void runner_free(runner* r) {
	// free linked list
	runner_pkt_node* node = r->pkt_list.head;
	while (node != NULL) {
		runner_pkt_node* tmp = node;
		node = node->next;
		av_free_packet(tmp->pkt);
		free(tmp);
	}

	dc_capture_free(r->dc_capture);
	encoder_free(r->encoder);

	free(r->texture);
	free(r);
}

void runner_start_capture_loop(runner* r) {
	uint64_t nanoseconds_per_frame = util_get_nanoseconds_per_frame(r->fps);
	uint64_t init_time = util_get_system_time_ns();

	for (;;) {
		uint64_t start_time = util_get_system_time_ns();
		uint64_t time_target = start_time + nanoseconds_per_frame;
		dc_capture_tick(r->dc_capture);

		pthread_mutex_lock(&r->mutex);
		av_frame_copy(r->encoder->frame, r->dc_capture->frame);
		r->dc_capture->has_data = true;
		pthread_mutex_unlock(&r->mutex);

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

		if (frameCount / r->fps > 10) {
			printf("Max time: %" PRIu64 "\n", max);
			return;
		}

		uint64_t start_time = util_get_system_time_ns();
		uint64_t time_target = start_time + nanoseconds_per_frame;

		if (r->pkt_list.tail == NULL) {
			runner_add_pkt(r, av_packet_alloc());
		}

		// encode frame into last packet in list
		pthread_mutex_lock(&r->mutex);
		bool new_pkt = encoder_encode_frame(r->encoder, r->pkt_list.tail->pkt);
		pthread_mutex_unlock(&r->mutex);

		// add new packet to list if packet is full
		if (new_pkt) {
			runner_add_pkt(r, av_packet_alloc());
		}

		int frame_time = util_get_system_time_ns() - start_time;
		max = frame_time > max ? frame_time : max;
		printf("Frame time: %" PRIu64 "\n", frame_time);
		frameCount++;
		util_sleepto_ns(time_target);
	}
}

//void runner_start_loop(runner* r) {
//	uint64_t nanoseconds_per_frame = util_get_nanoseconds_per_frame(r->fps);
//	uint64_t init_time = util_get_system_time_ns();
//	int frameCount = 0;
//
//	for (;;) {
//		if (frameCount >= r->fps * 5) {
//			break;
//		}
//		uint64_t start_time = util_get_system_time_ns();
//		uint64_t time_target = start_time + nanoseconds_per_frame;
//
//		// TODO: handle if frame capture takes longer than time target
//		runner_tick(r);
//
//		uint64_t frame_time = util_get_system_time_ns() - start_time;
//
//		uint64_t ctime = util_get_system_time_ns();
//		frameCount++;
//		if (ctime > time_target) {
//			printf("Overwhelmed by:% " PRIu64 " ns\n", ctime - time_target);
//		}
//		else {
//			util_sleepto_ns(time_target);
//		}
//	}
//
//	uint64_t end_time = util_get_system_time_ns() - init_time;
//	printf("Total Time: %" PRIu64 "\n", end_time);
//}

//void runner_tick(runner* r) {
//	dc_capture_tick(r->dc_capture);
//
//	if (r->pkt_list.tail == NULL) {
//		runner_add_pkt(r, av_packet_alloc());
//	}
//
//	bool new_pkt = encoder_encode_rgb(r->encoder, r->pkt_list.tail->pkt, r->dc_capture->rgb);
//
//	// add new packet to list if packet is full
//	if (new_pkt) {
//		runner_add_pkt(r, av_packet_alloc());
//	}
//}

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

