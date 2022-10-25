#include "pch.h"
#include "convert_video.h"

extern IDX11GraphicInstance *pGraphic;

void FormatConvert_YUVToRGB::InitConvertion(const AVFrame *av_frame,
					    enum video_range_type color_range,
					    enum video_colorspace color_space)
{
	InitPlanarTexture(av_frame);
	InitMatrix(color_range, color_space);

	m_stPSConstBuffer.width = (float)av_frame->width;
	m_stPSConstBuffer.height = (float)av_frame->height;
	m_stPSConstBuffer.half_width = (float)av_frame->width * 0.5f;
	m_stPSConstBuffer.full_range = (color_range == video_range_type::VIDEO_RANGE_FULL) ? 1 : 0;

	m_stPSConstBuffer.color_vec0.x = color_matrix[0];
	m_stPSConstBuffer.color_vec0.y = color_matrix[1];
	m_stPSConstBuffer.color_vec0.z = color_matrix[2];
	m_stPSConstBuffer.color_vec0.w = color_matrix[3];

	m_stPSConstBuffer.color_vec1.x = color_matrix[4];
	m_stPSConstBuffer.color_vec1.y = color_matrix[5];
	m_stPSConstBuffer.color_vec1.z = color_matrix[6];
	m_stPSConstBuffer.color_vec1.w = color_matrix[7];

	m_stPSConstBuffer.color_vec2.x = color_matrix[8];
	m_stPSConstBuffer.color_vec2.y = color_matrix[9];
	m_stPSConstBuffer.color_vec2.z = color_matrix[10];
	m_stPSConstBuffer.color_vec2.w = color_matrix[11];

	m_stPSConstBuffer.color_range_min.x = color_range_min[0];
	m_stPSConstBuffer.color_range_min.y = color_range_min[1];
	m_stPSConstBuffer.color_range_min.z = color_range_min[2];

	m_stPSConstBuffer.color_range_max.x = color_range_max[0];
	m_stPSConstBuffer.color_range_max.y = color_range_max[1];
	m_stPSConstBuffer.color_range_max.z = color_range_max[2];
}

void FormatConvert_YUVToRGB::UninitConvertion()
{
	for (auto &item : m_aVideoPlanes) {
		pGraphic->ReleaseGraphicObject(item.texture);
		item.texture = 0;
	}
}

void FormatConvert_YUVToRGB::UpdateVideo(const AVFrame *av_frame)
{
	for (size_t i = 0; i < min(MAX_VIDEO_PLANES, AV_NUM_DATA_POINTERS); i++) {
		auto &item = m_aVideoPlanes[i];
		if (!item.texture)
			break;

		D3D11_MAPPED_SUBRESOURCE mapData;
		if (pGraphic->MapTexture(item.texture, false, &mapData)) {
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
}

std::vector<texture_handle> FormatConvert_YUVToRGB::GetTextures()
{
	std::vector<texture_handle> ret;

	for (auto &item : m_aVideoPlanes) {
		if (item.texture)
			ret.push_back(item.texture);
	}

	return ret;
}

bool FormatConvert_YUVToRGB::InitPlanarTexture(const AVFrame *av_frame)
{
	for (auto &item : m_aVideoPlanes)
		item = ST_VideoPlaneInfo();

	switch (av_frame->format) {
	case AV_PIX_FMT_YUV420P:
		SetPlanarI420(av_frame);
		break;

	case AV_PIX_FMT_NV12:
		SetPlanarNV12(av_frame);
		break;

	case AV_PIX_FMT_YUYV422:
	case AV_PIX_FMT_UYVY422:
		SetPacked422Info(av_frame);
		break;

	default:
		return false;
	}

	for (auto &item : m_aVideoPlanes) {
		if (item.width && item.height) {
			ST_TextureInfo info;
			info.width = item.width;
			info.height = item.height;
			info.format = item.format;
			info.usage = TextureType::WriteTexture;

			item.texture = pGraphic->CreateTexture(info);
			assert(item.texture);
		}
	}

	return true;
}

void FormatConvert_YUVToRGB::InitMatrix(enum video_range_type color_range,
					enum video_colorspace color_space)
{
	if (color_space == video_colorspace::VIDEO_CS_DEFAULT)
		color_space = video_colorspace::VIDEO_CS_709;

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

void FormatConvert_YUVToRGB::SetPlanarI420(const AVFrame *av_frame)
{
	m_aVideoPlanes[0].width = av_frame->width;
	m_aVideoPlanes[0].height = av_frame->height;
	m_aVideoPlanes[0].format = DXGI_FORMAT_R8_UNORM;

	m_aVideoPlanes[1].width = (av_frame->width + 1) / 2;
	m_aVideoPlanes[1].height = (av_frame->height + 1) / 2;
	m_aVideoPlanes[1].format = DXGI_FORMAT_R8_UNORM;

	m_aVideoPlanes[2].width = (av_frame->width + 1) / 2;
	m_aVideoPlanes[2].height = (av_frame->height + 1) / 2;
	m_aVideoPlanes[2].format = DXGI_FORMAT_R8_UNORM;
}

void FormatConvert_YUVToRGB::SetPlanarNV12(const AVFrame *av_frame)
{
	m_aVideoPlanes[0].width = av_frame->width;
	m_aVideoPlanes[0].height = av_frame->height;
	m_aVideoPlanes[0].format = DXGI_FORMAT_R8_UNORM;

	m_aVideoPlanes[1].width = (av_frame->width + 1) / 2;
	m_aVideoPlanes[1].height = (av_frame->height + 1) / 2;
	m_aVideoPlanes[1].format = DXGI_FORMAT_R8G8_UNORM;
}

void FormatConvert_YUVToRGB::SetPacked422Info(const AVFrame *av_frame)
{
	m_aVideoPlanes[0].width = (av_frame->width + 1) / 2;
	m_aVideoPlanes[0].height = av_frame->height;
	m_aVideoPlanes[0].format = DXGI_FORMAT_R8G8B8A8_UNORM;
}
