#include "pch.h"
#include "convert-video-rgb2yuv.h"
#include "render-interface-wrapper.h"

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

	InitMatrix(original_video_info.color_range, original_video_info.color_space);

	if (!InitPlane()) {
		UninitConvertion();
		return false;
	}

	return true;
}

void FormatConvert_RGBToYUV::UninitConvertion()
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	for (auto &item : video_plane_list) {
		if (item.canvas_tex)
			original_video_info.graphic->DestroyGraphicObject(item.canvas_tex);

		if (item.read_tex)
			original_video_info.graphic->DestroyGraphicObject(item.read_tex);
	}

	video_plane_list.clear();
}

bool FormatConvert_RGBToYUV::ConvertVideo(texture_handle tex)
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	auto texInfo = original_video_info.graphic->GetTextureInfo(tex);
	if (texInfo.width != original_video_info.width ||
	    texInfo.height != original_video_info.height) {
		assert(false);
		return false;
	}

	std::vector<texture_handle> textures{tex};
	for (const auto &item : video_plane_list) {
		if (!original_video_info.graphic->BeginRenderCanvas(item.canvas_tex)) {
			assert(false);
			return false;
		}

		SIZE canvas(item.width, item.height);
		SIZE texSize(texInfo.width, texInfo.height);
		RECT drawDest(0, 0, item.width, item.height);
		float matrixWVP[4][4];
		TransposeMatrixWVP(canvas, texSize, drawDest, TextureRenderMode::FitToRect,
				   matrixWVP);

		ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
		VertexList_RectTriangle(texSize, false, false, outputVertex);

		original_video_info.graphic->SetVertexBuffer(item.shader, outputVertex,
							     sizeof(outputVertex));

		original_video_info.graphic->SetVSConstBuffer(item.shader, &(matrixWVP[0][0]),
							      sizeof(matrixWVP));

		original_video_info.graphic->SetPSConstBuffer(item.shader, &item.ps_const_buffer,
							      sizeof(toyuv_const_buffer));

		original_video_info.graphic->DrawTexture(item.shader, FilterType::FilterPoint,
							 textures);
		original_video_info.graphic->EndRender();

		// copy it to read video
		original_video_info.graphic->CopyTexture(item.read_tex, item.canvas_tex);
	}

	if (1) {
		FILE *fp = 0;
		fopen_s(&fp, "d:/1080p.i420", "wb+");

		if (!fp)
			return false;

		for (const auto &item : video_plane_list) {
			D3D11_MAPPED_SUBRESOURCE data;
			if (original_video_info.graphic->MapTexture(
				    item.read_tex, MapTextureType::MapRead, &data)) {

				if (item.expect_linesize == data.RowPitch) {
					fwrite(data.pData,
					       (size_t)item.expect_linesize * item.height, 1, fp);

				} else {
					auto linesize = min(item.expect_linesize, data.RowPitch);
					for (size_t i = 0; i < item.height; i++) {
						fwrite((char *)data.pData + i * data.RowPitch,
						       linesize, 1, fp);
					}
				}

				original_video_info.graphic->UnmapTexture(item.read_tex);
			}
		}

		fclose(fp);
	}

	return true;
}

void FormatConvert_RGBToYUV::InitMatrix(enum video_range_type color_range,
					enum video_colorspace color_space)
{
	std::array<float, 16> color_matrix{};
	GetVideoMatrix(color_range, color_space, &color_matrix, nullptr, nullptr);
	VideoMatrixINV(color_matrix);

	// Y plane
	color_vec_y.x = color_matrix[4];
	color_vec_y.y = color_matrix[5];
	color_vec_y.z = color_matrix[6];
	color_vec_y.w = color_matrix[7];

	// U plane
	color_vec_u.x = color_matrix[0];
	color_vec_u.y = color_matrix[1];
	color_vec_u.z = color_matrix[2];
	color_vec_u.w = color_matrix[3];

	// V plane
	color_vec_v.x = color_matrix[8];
	color_vec_v.y = color_matrix[9];
	color_vec_v.z = color_matrix[10];
	color_vec_v.w = color_matrix[11];
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
			item.canvas_tex = original_video_info.graphic->CreateTexture(info);
			if (!item.canvas_tex) {
				assert(false);
				return false;
			}

			info.usage = TextureType::ReadTexture;
			item.read_tex = original_video_info.graphic->CreateTexture(info);
			if (!item.read_tex) {
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
	video_plane_list[0].shader = shaders[ShaderType::yuvOnePlane];
	video_plane_list[0].ps_const_buffer.color_vec0 = color_vec_y;
	video_plane_list[0].expect_linesize = video_plane_list[0].width * 1;

	video_plane_list[1].width = (original_video_info.width + 1) / 2;
	video_plane_list[1].height = (original_video_info.height + 1) / 2;
	video_plane_list[1].format = DXGI_FORMAT_R8_UNORM;
	video_plane_list[1].shader = shaders[ShaderType::yuvOnePlane];
	video_plane_list[1].ps_const_buffer.color_vec0 = color_vec_u;
	video_plane_list[1].expect_linesize = video_plane_list[1].width * 1;

	video_plane_list[2].width = (original_video_info.width + 1) / 2;
	video_plane_list[2].height = (original_video_info.height + 1) / 2;
	video_plane_list[2].format = DXGI_FORMAT_R8_UNORM;
	video_plane_list[2].shader = shaders[ShaderType::yuvOnePlane];
	video_plane_list[2].ps_const_buffer.color_vec0 = color_vec_v;
	video_plane_list[2].expect_linesize = video_plane_list[2].width * 1;
}

void FormatConvert_RGBToYUV::SetPlanarNV12()
{
	video_plane_list.push_back(video_plane_info());
	video_plane_list.push_back(video_plane_info());

	video_plane_list[0].width = original_video_info.width;
	video_plane_list[0].height = original_video_info.height;
	video_plane_list[0].format = DXGI_FORMAT_R8_UNORM;
	video_plane_list[0].shader = shaders[ShaderType::yuvOnePlane];
	video_plane_list[0].ps_const_buffer.color_vec0 = color_vec_y;
	video_plane_list[0].expect_linesize = video_plane_list[0].width * 1;

	video_plane_list[1].width = (original_video_info.width + 1) / 2;
	video_plane_list[1].height = (original_video_info.height + 1) / 2;
	video_plane_list[1].format = DXGI_FORMAT_R8G8_UNORM;
	video_plane_list[1].shader = shaders[ShaderType::uvPlane];
	video_plane_list[1].ps_const_buffer.color_vec0 = color_vec_u;
	video_plane_list[1].ps_const_buffer.color_vec1 = color_vec_v;
	video_plane_list[1].expect_linesize = video_plane_list[1].width * 2;
}
