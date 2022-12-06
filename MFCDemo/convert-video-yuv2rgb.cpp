#include "pch.h"
#include "convert-video-yuv2rgb.h"
#include "render-interface-wrapper.h"

FormatConvert_YUVToRGB::FormatConvert_YUVToRGB(video_convert_params params)
	: original_video_info(params)
{
}

FormatConvert_YUVToRGB::~FormatConvert_YUVToRGB()
{
	UninitConvertion();
}

bool FormatConvert_YUVToRGB::InitConvertion()
{
	UninitConvertion();

	InitMatrix(original_video_info.color_range, original_video_info.color_space);

	if (!InitPlane()) {
		UninitConvertion();
		return false;
	}

	return true;
}

void FormatConvert_YUVToRGB::UninitConvertion()
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	for (auto &item : video_plane_list) {
		if (item.texture)
			original_video_info.graphic->DestroyGraphicObject(item.texture);
	}

	video_plane_list.clear();
}

void FormatConvert_YUVToRGB::RenderVideo(const AVFrame *av_frame, SIZE canvas, RECT dest)
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	UpdateVideo(av_frame);

	std::vector<texture_handle> texs = GetTextures();
	SIZE resolution((LONG)ps_const_buffer.width, (LONG)ps_const_buffer.height);

	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, resolution, dest, true, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(resolution, false, false, outputVertex);

	original_video_info.graphic->SetVertexBuffer(convert_shader, outputVertex,
						     sizeof(outputVertex));
	original_video_info.graphic->SetVSConstBuffer(convert_shader, &(matrixWVP[0][0]),
						      sizeof(matrixWVP));
	original_video_info.graphic->SetPSConstBuffer(convert_shader, &ps_const_buffer,
						      sizeof(torgb_const_buffer));

	original_video_info.graphic->DrawTexture(convert_shader, FilterType::FilterAnisotropic,
						 texs);
}

void FormatConvert_YUVToRGB::UpdateVideo(const AVFrame *av_frame)
{
	if (av_frame->width != original_video_info.width ||
	    av_frame->height != original_video_info.height ||
	    av_frame->format != original_video_info.format) {
		assert(false);
		return;
	}

	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	for (size_t i = 0; i < min(video_plane_list.size(), AV_NUM_DATA_POINTERS); i++) {
		auto &item = video_plane_list[i];
		assert(item.texture);
		if (!item.texture)
			break;

		D3D11_MAPPED_SUBRESOURCE data;
		if (original_video_info.graphic->MapTexture(item.texture, MapTextureType::MapWrite,
							    &data)) {
			uint32_t stride = min(data.RowPitch, (uint32_t)av_frame->linesize[i]);
			uint8_t *src = av_frame->data[i];
			uint8_t *dest = (uint8_t *)data.pData;

			if (data.RowPitch == av_frame->linesize[i]) {
				memmove(dest, src, (size_t)stride * item.height);
			} else {
				for (size_t j = 0; j < item.height; j++) {
					memmove(dest, src, stride);
					dest += data.RowPitch;
					src += av_frame->linesize[i];
				}
			}

			original_video_info.graphic->UnmapTexture(item.texture);
		}
	}
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

void FormatConvert_YUVToRGB::InitMatrix(enum video_range_type color_range,
					enum video_colorspace color_space)
{
	std::array<float, 16> color_matrix{};
	std::array<float, 3> color_range_min{0.0f, 0.0f, 0.0f};
	std::array<float, 3> color_range_max{1.0f, 1.0f, 1.0f};
	GetVideoMatrix(color_range, color_space, &color_matrix, &color_range_min, &color_range_max);

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
			info.usage = TextureType::WriteTexture;

			item.texture = original_video_info.graphic->CreateTexture(info);
			if (!item.texture) {
				assert(false);
				return false;
			}
		}
	}

	return true;
}

void FormatConvert_YUVToRGB::SetPlanarI420()
{
	convert_shader = shaders[ShaderType::i420ToRGB];
	assert(convert_shader);

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
	convert_shader = shaders[ShaderType::nv12ToRGB];
	assert(convert_shader);

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
	convert_shader = shaders[ShaderType::yuy2ToRGB];
	assert(convert_shader);

	video_plane_list.push_back(video_plane_info());

	video_plane_list[0].width = (original_video_info.width + 1) / 2;
	video_plane_list[0].height = original_video_info.height;
	video_plane_list[0].format = DXGI_FORMAT_B8G8R8A8_UNORM;
}
