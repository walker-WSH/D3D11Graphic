#pragma once
#include <DX11GraphicAPI.h>
#include <DirectXMath.h>
#include <assert.h>
#include "convert_video_color.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;
using float4x4 = DirectX::XMFLOAT4X4;

const auto MAX_VIDEO_PLANES = 3;

// 注意字节以16位对齐
struct ST_PSConstBuffer {
	float width;
	float height;
	float half_width;
	uint32_t full_range;

	float4 color_vec0;
	float4 color_vec1;
	float4 color_vec2;
	float4 color_range_min; //only x,y,z valid
	float4 color_range_max; //only x,y,z valid
};

class FormatConvert_YUVToRGB {
public:
	void InitParams(const AVFrame *av_frame, enum video_range_type, enum video_colorspace);
	const ST_PSConstBuffer *GetPSBuffer() { return &m_stPSConstBuffer; }

	void UpdateVideo(const AVFrame *av_frame);

private:
	bool InitPlanarTexture(const AVFrame *av_frame);
	void InitMatrix(enum video_range_type color_range, enum video_colorspace color_space);

	void SetPlanarI420(const AVFrame *av_frame);
	void SetPlanarNV12(const AVFrame *av_frame);
	void SetPacked422Info(const AVFrame *av_frame);

private:
	struct ST_VideoPlaneInfo {
		uint32_t width = 0;
		uint32_t height = 0;
		enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		texture_handle texture = 0;
	};

	std::array<float, 16> color_matrix{};
	std::array<float, 3> color_range_min{};
	std::array<float, 3> color_range_max{};

	ST_VideoPlaneInfo m_aVideoPlanes[MAX_VIDEO_PLANES];
	ST_PSConstBuffer m_stPSConstBuffer;
};
