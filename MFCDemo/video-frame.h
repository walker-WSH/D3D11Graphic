#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
}

static enum AVPixelFormat destFormat = AVPixelFormat::AV_PIX_FMT_YUV420P;
//static enum AVPixelFormat destFormat = AVPixelFormat::AV_PIX_FMT_NV12;
//static enum AVPixelFormat destFormat = AVPixelFormat::AV_PIX_FMT_YUYV422;

extern AVFrame *frame_i420;
extern AVFrame *frame_yuy2;

int open_file();
AVFrame *decode_frame();

bool SaveBitmapFile(const wchar_t *path, const uint8_t *data, int linesize, int width, int height,
		    int pixelSize, bool flip);
