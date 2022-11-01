#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int open_file();
AVFrame *decode_frame();
