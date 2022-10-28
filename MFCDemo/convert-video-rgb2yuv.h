#pragma once
#include "convert-video-define.h"

// 注意字节以16位对齐
struct toyuv_const_buffer {
	DirectX::XMFLOAT4 color_vec0;
	DirectX::XMFLOAT4 color_vec1; // this param may not be used for some case
};

class FormatConvert_RGBToYUV {
public:
	FormatConvert_RGBToYUV(video_convert_params params);
	virtual ~FormatConvert_RGBToYUV();

	bool InitConvertion();
	void UninitConvertion();

	void RenderConvertVideo(texture_handle tex);

private:
	void InitMatrix(enum video_range_type color_range, enum video_colorspace color_space);

	bool InitPlane();
	void SetPlanarI420();
	void SetPlanarNV12();

private:
	struct video_plane_info {
		uint32_t width = 0;
		uint32_t height = 0;
		enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		texture_handle canvas = 0;

		shader_handle shader = 0; // does not need to free
		toyuv_const_buffer ps_const_buffer;
	};

	DirectX::XMFLOAT4 color_vec_y;
	DirectX::XMFLOAT4 color_vec_u;
	DirectX::XMFLOAT4 color_vec_v;

	const video_convert_params original_video_info;
	std::vector<video_plane_info> video_plane_list;
};
