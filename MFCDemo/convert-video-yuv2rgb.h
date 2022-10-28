#pragma once
#include <assert.h>
#include "DX11GraphicAPI.h"
#include "convert-video-color.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

using namespace matrix;

// 注意字节以16位对齐
struct torgb_const_buffer {
	DirectX::XMFLOAT4 color_vec0;
	DirectX::XMFLOAT4 color_vec1;
	DirectX::XMFLOAT4 color_vec2;
	DirectX::XMFLOAT4 color_range_min; //only x,y,z valid
	DirectX::XMFLOAT4 color_range_max; //only x,y,z valid

	float width;
	float height;
	float half_width;
	uint32_t full_range;
};

struct video_convert_params {
	uint32_t width = 0;
	uint32_t height = 0;
	enum AVPixelFormat format = AVPixelFormat::AV_PIX_FMT_NONE;
	enum video_range_type color_range = video_range_type::VIDEO_RANGE_FULL;
	enum video_colorspace color_space = video_colorspace::VIDEO_CS_709;
};

class FormatConvert_YUVToRGB {
public:
	FormatConvert_YUVToRGB(video_convert_params params);
	virtual ~FormatConvert_YUVToRGB();

	bool InitConvertion();
	void UninitConvertion();

	void UpdateVideo(const AVFrame *av_frame);

	const torgb_const_buffer *GetPSBuffer() { return &ps_const_buffer; }
	std::vector<texture_handle> GetTextures();

private:
	bool InitPlane();
	void InitMatrix(enum video_range_type color_range, enum video_colorspace color_space);

	void SetPlanarI420();
	void SetPlanarNV12();
	void SetPacked422Info();

private:
	struct video_plane_info {
		uint32_t width = 0;
		uint32_t height = 0;
		enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		texture_handle texture = 0;
	};

	std::array<float, 16> color_matrix{};
	std::array<float, 3> color_range_min{0.0f, 0.0f, 0.0f};
	std::array<float, 3> color_range_max{1.0f, 1.0f, 1.0f};

	video_convert_params original_video_info;
	torgb_const_buffer ps_const_buffer;
	std::vector<video_plane_info> video_plane_list;
};
