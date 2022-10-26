#include "pch.h"
#include "convert_video.h"

extern IDX11GraphicInstance *pGraphic;

FormatConvert_YUVToRGB::FormatConvert_YUVToRGB(video_convert_params params)
	: original_video_info(params)
{
	InitConvertion();
}

FormatConvert_YUVToRGB::~FormatConvert_YUVToRGB()
{
	UninitConvertion();
}

bool FormatConvert_YUVToRGB::InitConvertion()
{
	UninitConvertion();

	if (!InitPlane()) {
		UninitConvertion();
		return false;
	}

	InitMatrix(original_video_info.color_range, original_video_info.color_space);

	ps_const_buffer.width = (float)original_video_info.width;
	ps_const_buffer.height = (float)original_video_info.height;
	ps_const_buffer.half_width = (float)original_video_info.width * 0.5f;
	ps_const_buffer.full_range =
		(original_video_info.color_range == video_range_type::VIDEO_RANGE_FULL) ? 1 : 0;

	ps_const_buffer.color_vec0.x = color_matrix[0];
	ps_const_buffer.color_vec0.y = color_matrix[1];
	ps_const_buffer.color_vec0.z = color_matrix[2];
	ps_const_buffer.color_vec0.w = color_matrix[3];

	ps_const_buffer.color_vec1.x = color_matrix[4];
	ps_const_buffer.color_vec1.y = color_matrix[5];
	ps_const_buffer.color_vec1.z = color_matrix[6];
	ps_const_buffer.color_vec1.w = color_matrix[7];

	ps_const_buffer.color_vec2.x = color_matrix[8];
	ps_const_buffer.color_vec2.y = color_matrix[9];
	ps_const_buffer.color_vec2.z = color_matrix[10];
	ps_const_buffer.color_vec2.w = color_matrix[11];

	ps_const_buffer.color_range_min.x = color_range_min[0];
	ps_const_buffer.color_range_min.y = color_range_min[1];
	ps_const_buffer.color_range_min.z = color_range_min[2];

	ps_const_buffer.color_range_max.x = color_range_max[0];
	ps_const_buffer.color_range_max.y = color_range_max[1];
	ps_const_buffer.color_range_max.z = color_range_max[2];

	return true;
}

void FormatConvert_YUVToRGB::UninitConvertion()
{
	for (auto &item : video_plane_list) {
		if (!item.texture)
			break;

		pGraphic->ReleaseGraphicObject(item.texture);
	}

	video_plane_list.clear();
}

void FormatConvert_YUVToRGB::UpdateVideo(const AVFrame *av_frame)
{
	assert(av_frame->width == original_video_info.width);
	assert(av_frame->height == original_video_info.height);
	assert(av_frame->format == original_video_info.format);

	for (size_t i = 0; i < min(video_plane_list.size(), AV_NUM_DATA_POINTERS); i++) {
		auto &item = video_plane_list[i];
		assert(item.texture);
		if (!item.texture)
			break;

		D3D11_MAPPED_SUBRESOURCE mapData;
		if (pGraphic->MapTexture(item.texture, MapTextureType::MapWrite, &mapData)) {
			uint32_t stride = min(mapData.RowPitch, (uint32_t)av_frame->linesize[i]);
			uint8_t *src = av_frame->data[i];
			uint8_t *dest = (uint8_t *)mapData.pData;

			if (mapData.RowPitch == av_frame->linesize[i]) {
				memmove(dest, src, stride * item.height);
			} else {
				for (size_t j = 0; j < item.height; j++) {
					memmove(dest, src, stride);
					dest += mapData.RowPitch;
					src += av_frame->linesize[i];
				}
			}

			pGraphic->UnmapTexture(item.texture);
		}
	}

#if 1
	FILE *fp = 0;
	fopen_s(&fp, "d:/test.yuv", "wb+");

	if (!fp)
		return;

	int total2 = 0;
	for (auto &item : video_plane_list) {
		if (!item.texture)
			break;

		ST_TextureInfo info = pGraphic->GetTextureInfo(item.texture);
		info.usage = TextureType::ReadTexture;
		int total = 0;

		auto tex = pGraphic->CreateTexture(info);
		pGraphic->CopyTexture(tex, item.texture);
		D3D11_MAPPED_SUBRESOURCE mapData;
		if (pGraphic->MapTexture(tex, MapTextureType::MapRead, &mapData)) {
			for (size_t i = 0; i < item.height; i++) {
				total += item.width;
				fwrite((char *)mapData.pData + i * mapData.RowPitch, item.width, 1,
				       fp);
			}
			pGraphic->UnmapTexture(tex);
			assert(total == item.width * item.height);
			total2 += total;
		}
		pGraphic->ReleaseGraphicObject(tex);
	}

	fclose(fp);
#endif
}

std::vector<texture_handle> FormatConvert_YUVToRGB::GetTextures()
{
	std::vector<texture_handle> ret;

	for (auto &item : video_plane_list) {
		if (item.texture)
			ret.push_back(item.texture);
	}

	return ret;
}

bool FormatConvert_YUVToRGB::InitPlane()
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

	case AV_PIX_FMT_YUYV422:
	case AV_PIX_FMT_UYVY422:
		SetPacked422Info();
		break;

	default:
		return false;
	}

	for (auto &item : video_plane_list) {
		if (item.width && item.height) {
			ST_TextureInfo info;
			info.width = item.width;
			info.height = item.height;
			info.format = item.format;
			info.usage = TextureType::WriteTexture;

			item.texture = pGraphic->CreateTexture(info);
			if (!item.texture) {
				assert(false);
				return false;
			}
		}
	}

	return true;
}

void FormatConvert_YUVToRGB::InitMatrix(enum video_range_type color_range,
					enum video_colorspace color_space)
{
	bool is_full_range = (color_range == video_range_type::VIDEO_RANGE_FULL);
	for (size_t i = 0; i < NUM_FORMATS; i++) {
		if (format_info[i].color_space == color_space) {
			if (is_full_range) {
				color_range_min = {0.0f, 0.0f, 0.0f};
				color_range_max = {1.0f, 1.0f, 1.0f};
				color_matrix = format_info[i].matrix[1];
			} else {
				color_range_min = format_info[i].float_range_min;
				color_range_max = format_info[i].float_range_max;
				color_matrix = format_info[i].matrix[0];
			}
			return;
		}
	}

	assert(false);
}

void FormatConvert_YUVToRGB::SetPlanarI420()
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

void FormatConvert_YUVToRGB::SetPlanarNV12()
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

void FormatConvert_YUVToRGB::SetPacked422Info()
{
	video_plane_list.push_back(video_plane_info());

	video_plane_list[0].width = (original_video_info.width + 1) / 2;
	video_plane_list[0].height = original_video_info.height;
	video_plane_list[0].format = DXGI_FORMAT_R8G8B8A8_UNORM;
}
