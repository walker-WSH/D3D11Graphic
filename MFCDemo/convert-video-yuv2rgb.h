#pragma once
#include "convert-video-define.h"

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

class FormatConvert_YUVToRGB {
public:
	FormatConvert_YUVToRGB(video_convert_params params);
	virtual ~FormatConvert_YUVToRGB();

	bool InitConvertion();
	void UninitConvertion();

	void UpdateVideo(const AVFrame *av_frame);

	torgb_const_buffer *GetPSBuffer();
	std::vector<texture_handle> GetTextures();

private:
	void InitMatrix(enum video_range_type color_range, enum video_colorspace color_space);

	bool InitPlane();
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

	const video_convert_params original_video_info;
	std::vector<video_plane_info> video_plane_list;
	torgb_const_buffer ps_const_buffer;
};
