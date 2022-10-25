#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

extern int width;
extern int height;

extern int lenY;
extern int lenU;
extern int lenV;

extern uint8_t *i420Y;
extern uint8_t *i420U;
extern uint8_t *i420V;

bool initVideo();
