#include "pch.h"
#include "convert-video-rgb2yuv.h"

FormatConvert_RGBToYUV::FormatConvert_RGBToYUV(video_convert_params params)
	: original_video_info(params)
{
	assert((params.width % 2) == 0);
	assert((params.height % 2) == 0);
}

FormatConvert_RGBToYUV::~FormatConvert_RGBToYUV()
{
	UninitConvertion();
}

bool FormatConvert_RGBToYUV::InitConvertion()
{
	UninitConvertion();

	if (!InitPlane()) {
		UninitConvertion();
		return false;
	}

	InitMatrix(original_video_info.color_range, original_video_info.color_space);
	return true;
}

void FormatConvert_RGBToYUV::UninitConvertion()
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	for (auto &item : video_plane_list) {
		if (item.texture) {
			original_video_info.graphic->ReleaseGraphicObject(item.texture);
		}
	}

	video_plane_list.clear();
}

void FormatConvert_RGBToYUV::RenderConvertVideo(texture_handle tex)
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	auto texInfo = original_video_info.graphic->GetTextureInfo(tex);
	if (texInfo.width != original_video_info.width ||
	    texInfo.height != original_video_info.height) {
		assert(false);
		return;
	}

	for (size_t i = 0; i < video_plane_list.size(); i++) {
		// TODO
	}
}

void FormatConvert_RGBToYUV::InitMatrix(enum video_range_type color_range,
					enum video_colorspace color_space)
{
	std::array<float, 16> color_matrix{};
	GetVideoMatrix(color_range, color_space, &color_matrix, nullptr, nullptr);
	VideoMatrixINV(color_matrix);

	// Y plane
	ps_const_buffer.color_vec0.x = color_matrix[4];
	ps_const_buffer.color_vec0.y = color_matrix[5];
	ps_const_buffer.color_vec0.z = color_matrix[6];
	ps_const_buffer.color_vec0.w = color_matrix[7];

	// U plane
	ps_const_buffer.color_vec1.x = color_matrix[0];
	ps_const_buffer.color_vec1.y = color_matrix[1];
	ps_const_buffer.color_vec1.z = color_matrix[2];
	ps_const_buffer.color_vec1.w = color_matrix[3];

	// V plane
	ps_const_buffer.color_vec2.x = color_matrix[8];
	ps_const_buffer.color_vec2.y = color_matrix[9];
	ps_const_buffer.color_vec2.z = color_matrix[10];
	ps_const_buffer.color_vec2.w = color_matrix[11];
}

bool FormatConvert_RGBToYUV::InitPlane()
{
	for (auto &item : video_plane_list)
		item = video_plane_info();

	switch (original_video_info.format) {
	case AV_PIX_FMT_YUV420P:
		SetPlanarI420();
		break;

	case AV_PIX_FMT_NV12:
		SetPlanarNV12();
		break;

	default:
		assert(false);
		return false;
	}

	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	for (auto &item : video_plane_list) {
		if (item.width && item.height) {
			ST_TextureInfo info;
			info.width = item.width;
			info.height = item.height;
			info.format = item.format;
			info.usage = TextureType::CanvasTarget;

			item.texture = original_video_info.graphic->CreateTexture(info);
			if (!item.texture) {
				assert(false);
				return false;
			}
		}
	}

	return true;
}

void FormatConvert_RGBToYUV::SetPlanarI420()
{
	video_plane_list.push_back(video_plane_info());
	video_plane_list.push_back(video_plane_info());
	video_plane_list.push_back(video_plane_info());

	video_plane_list[0].width = original_video_info.width;
	video_plane_list[0].height = original_video_info.height;
	video_plane_list[0].format = DXGI_FORMAT_R8_UNORM;

	video_plane_list[1].width = (original_video_info.width + 1) / 2;
	video_plane_list[1].height = (original_video_info.height + 1) / 2;
	video_plane_list[1].format = DXGI_FORMAT_R8_UNORM;

	video_plane_list[2].width = (original_video_info.width + 1) / 2;
	video_plane_list[2].height = (original_video_info.height + 1) / 2;
	video_plane_list[2].format = DXGI_FORMAT_R8_UNORM;
}

void FormatConvert_RGBToYUV::SetPlanarNV12()
{
	video_plane_list.push_back(video_plane_info());
	video_plane_list.push_back(video_plane_info());

	video_plane_list[0].width = original_video_info.width;
	video_plane_list[0].height = original_video_info.height;
	video_plane_list[0].format = DXGI_FORMAT_R8_UNORM;

	video_plane_list[1].width = (original_video_info.width + 1) / 2;
	video_plane_list[1].height = (original_video_info.height + 1) / 2;
	video_plane_list[1].format = DXGI_FORMAT_R8G8_UNORM;
}
