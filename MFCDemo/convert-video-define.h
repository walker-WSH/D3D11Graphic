#pragma once
#include <assert.h>
#include "DX11GraphicAPI.h"
#include "convert-video-color.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

using namespace matrix;

struct video_convert_params {
	IDX11GraphicInstance *graphic = nullptr;
	uint32_t width = 0;
	uint32_t height = 0;
	enum AVPixelFormat format = AVPixelFormat::AV_PIX_FMT_NONE;
	enum video_range_type color_range = video_range_type::VIDEO_RANGE_PARTIAL;
	enum video_colorspace color_space = video_colorspace::VIDEO_CS_601;
};
