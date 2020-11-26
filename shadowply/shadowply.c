#include <stdio.h>
#include "shadowply.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include "capture.h"
#include "util.h"

static char* windowN = "Windows PowerShell";

int main()
{
	const AVCodec* codec;
	AVCodecContext *context = NULL;
	
	codec = avcodec_find_encoder_by_name("libx264");
	context = avcodec_alloc_context3(codec);

	struct capture_capture* c = malloc(sizeof(struct capture_capture));

	// init
	capture_init(c, windowN, 60);
	capture_start_capture_loop(c);

	struct capture_bmp_node* node = c->bmp_node_first;

	int count = 0;
	for (;;) {
		if (node == NULL) {
			break;
		}

		char buf[20];
		snprintf(buf, 20, "test%d.bmp", count);
		// util_write_bitmap(node->bmp, buf);

		node = node->next;
		count++;
	}

	capture_free(c);

	avcodec_free_context(&context);

	return 0;
}

