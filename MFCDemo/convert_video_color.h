#pragma once
#include <DX11GraphicAPI.h>
#include <DirectXMath.h>
#include <array>
#include <vector>

enum class video_range_type {
	VIDEO_RANGE_FULL,
	VIDEO_RANGE_PARTIAL,
};

enum class video_colorspace {
	VIDEO_CS_601,
	VIDEO_CS_709,
};

struct camera_format_info {
	enum video_colorspace const color_space;
	std::array<float, 3> float_range_min;
	std::array<float, 3> float_range_max;
	std::array<std::array<float, 16>, 2> matrix;
};

const std::array<camera_format_info, 2> format_info = {{
	{video_colorspace::VIDEO_CS_601,
	 {16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f},
	 {235.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f},
	 {{{1.164384f, 0.000000f, 1.596027f, -0.874202f, 1.164384f, -0.391762f, -0.812968f,
	    0.531668f, 1.164384f, 2.017232f, 0.000000f, -1.085631f, 0.000000f, 0.000000f, 0.000000f,
	    1.000000f},
	   {1.000000f, 0.000000f, 1.407520f, -0.706520f, 1.000000f, -0.345491f, -0.716948f,
	    0.533303f, 1.000000f, 1.778976f, 0.000000f, -0.892976f, 0.000000f, 0.000000f, 0.000000f,
	    1.000000f}}}},

	{video_colorspace::VIDEO_CS_709,
	 {16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f},
	 {235.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f},
	 {{{1.164384f, 0.000000f, 1.792741f, -0.972945f, 1.164384f, -0.213249f, -0.532909f,
	    0.301483f, 1.164384f, 2.112402f, 0.000000f, -1.133402f, 0.000000f, 0.000000f, 0.000000f,
	    1.000000f},
	   {1.000000f, 0.000000f, 1.581000f, -0.793600f, 1.000000f, -0.188062f, -0.469967f,
	    0.330305f, 1.000000f, 1.862906f, 0.000000f, -0.935106f, 0.000000f, 0.000000f, 0.000000f,
	    1.000000f}}}},
}};

#define NUM_FORMATS (sizeof(format_info) / sizeof(format_info[0]))
