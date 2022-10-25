#include "pch.h"
#include "decode_video.h"

int width = 1920;
int height = 1080;

int lenY = width * height;
int lenU = (width / 2) * (height / 2);
int lenV = (width / 2) * (height / 2);

uint8_t *i420Y = new uint8_t[lenY];
uint8_t *i420U = new uint8_t[lenU];
uint8_t *i420V = new uint8_t[lenV];

bool initVideo()
{
	FILE *fp = nullptr;
	fopen_s(&fp, "1080p.i420", "wb+");
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
