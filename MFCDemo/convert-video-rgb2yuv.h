#pragma once
#include "convert-video-define.h"

// 注意字节以16位对齐
struct toyuv_const_buffer {
	DirectX::XMFLOAT4 color_vec0; // Y
	DirectX::XMFLOAT4 color_vec1; // U
	DirectX::XMFLOAT4 color_vec2; // V
};

class FormatConvert_RGBToYUV {
public:
	FormatConvert_RGBToYUV(video_convert_params params);
	virtual ~FormatConvert_RGBToYUV();

	bool InitConvertion();
	void UninitConvertion();

	void RenderConvertVideo(texture_handle tex);
	const toyuv_const_buffer *GetPSBuffer() { return &ps_const_buffer; }

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
		texture_handle texture = 0;
	};

	const video_convert_params original_video_info;
	std::vector<video_plane_info> video_plane_list;
	toyuv_const_buffer ps_const_buffer;
};
