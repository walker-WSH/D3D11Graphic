#include "pch.h"
#include <assert.h>
#include "video-frame.h"

int width = 1920;
int height = 1080;

int lenY = width * height;
int lenU = (width / 2) * (height / 2);
int lenV = (width / 2) * (height / 2);

uint8_t *i420Y = new uint8_t[lenY];
uint8_t *i420U = new uint8_t[lenU];
uint8_t *i420V = new uint8_t[lenV];

AVFrame *frame = nullptr;

bool readVideo()
{
	FILE *fp = nullptr;
	fopen_s(&fp, "1080p.i420", "rb+");
	if (!fp) {
		assert(false);
		return false;
	}

	fread(i420Y, lenY, 1, fp);
	fread(i420U, lenU, 1, fp);
	fread(i420V, lenV, 1, fp);

	fclose(fp);
	return true;
}

bool initVideo()
{
	if (!readVideo())
		return false;

	frame = av_frame_alloc();

	frame->format = AV_PIX_FMT_YUV420P;
	frame->width = width;
	frame->height = height;

	int ret = av_frame_get_buffer(frame, 32);
	if (ret < 0) {
		assert(false);
		return false;
	}

	memmove(frame->data[0], i420Y, lenY);
	memmove(frame->data[1], i420U, lenU);
	memmove(frame->data[2], i420V, lenV);

	return true;
}
