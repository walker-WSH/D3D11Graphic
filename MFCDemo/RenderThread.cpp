#include "pch.h"
#include "MFCDemoDlg.h"
#include "video-frame.h"
#include "render-interface-wrapper.h"
#include <assert.h>
#include <vector>
#include <memory>
#include <unordered_map>

#define BORDER_THICKNESS 3

display_handle display = nullptr;
texture_handle texCanvas = nullptr;
texture_handle texGirl = nullptr;
texture_handle texShared = nullptr;
texture_handle texAlpha = nullptr;
texture_handle texImg = nullptr;
texture_handle texImg2 = nullptr;
texture_handle texForD2D = nullptr;

struct texRegionInfo {
	RECT region = {0, 0, 0, 0};
	bool selected = false;
	bool fullscreen = false;
};
std::unordered_map<texture_handle, struct texRegionInfo> texRegions;

std::shared_ptr<FormatConvert_YUVToRGB> pI420_To_RGB = nullptr;
std::shared_ptr<FormatConvert_YUVToRGB> pYUYV_To_RGB = nullptr;
AVFrame *preFrame = nullptr;

bool InitGraphic(HWND hWnd);
void UnInitGraphic();

void InitRenderRect(RECT rc, int numH, int numV);

void RenderCustomFormat(SIZE canvasSize, RECT rc);

texture_handle yuyvCanvas = nullptr;
void RenderYUYVFormat();

void CMFCDemoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	for (auto &item : texRegions) {
		if (item.second.fullscreen) {
			item.second.fullscreen = false;
			break;
		}
	}

	for (auto &item : texRegions) {
		auto rc = item.second.region;
		if (point.x > rc.left && point.x < rc.right && point.y > rc.top &&
		    point.y < rc.bottom) {
			item.second.selected = true;
		} else {
			item.second.selected = false;
		}
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CMFCDemoDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	for (auto &item : texRegions) {
		if (item.second.selected) {
			item.second.fullscreen = !item.second.fullscreen;
			break;
		}
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}

void CMFCDemoDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	DWORD styles = (DWORD)GetWindowLongPtr(m_hWnd, GWL_STYLE);
	if (styles & WS_MAXIMIZE)
		ShowWindow(SW_RESTORE);
	else
		ShowWindow(SW_MAXIMIZE);

	CDialogEx::OnRButtonDown(nFlags, point);
}

bool bFullscreenCrop = false;
void CMFCDemoDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	bFullscreenCrop = !bFullscreenCrop;
	CDialogEx::OnMButtonDown(nFlags, point);
}

unsigned __stdcall CMFCDemoDlg::ThreadFunc(void *pParam)
{
	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);
	pGraphic = self->m_pGraphic;

	bool saved = false;

	if (!InitGraphic(self->m_hWnd))
		return 1;

	assert(open_file() == 0);

	{
		AUTO_GRAPHIC_CONTEXT(pGraphic);
		RenderYUYVFormat();
	}

	texRegions[texAlpha] = texRegionInfo();
	texRegions[texGirl] = texRegionInfo();
	texRegions[texImg] = texRegionInfo();
	texRegions[texCanvas] = texRegionInfo();
	texRegions[yuyvCanvas] = texRegionInfo();
	texRegions[texImg2] = texRegionInfo();
	texRegions[texForD2D] = texRegionInfo();

	while (!self->m_bExit) {
		Sleep(33);

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		rc.right = (rc.right / 2) * 2;
		rc.bottom = (rc.bottom / 2) * 2;

		InitRenderRect(rc, 3, 3);

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		SIZE canvasSize(rc.right - rc.left, rc.bottom - rc.top);
		pGraphic->SetDisplaySize(display, canvasSize.cx, canvasSize.cy);

		if (!pGraphic->IsGraphicBuilt()) {
			if (!pGraphic->ReBuildGraphic())
				continue;
		}

		if (pGraphic->BeginRenderCanvas(texCanvas)) {
			auto info = pGraphic->GetTextureInfo(texCanvas);
			RenderCustomFormat(SIZE(info.width, info.height),
					   RECT(0, 0, info.width, info.height));

			pGraphic->EndRender();

			if (!saved) {
				saved = true;

				video_convert_params params;
				params.graphic = pGraphic;
				params.width = info.width;
				params.height = info.height;
				params.format = AV_PIX_FMT_NV12;

				FormatConvert_RGBToYUV *toYUV = new FormatConvert_RGBToYUV(params);
				toYUV->InitConvertion();
				toYUV->ConvertVideo(texCanvas);
				toYUV->UninitConvertion();
				delete toYUV;
			}
		}

		if (pGraphic->BeginRenderWindow(display)) {
			ST_Color bkColor(0, 0, 0.5, 1.0);
			pGraphic->ClearBackground(&bkColor);
			pGraphic->SetBlendState(BlendStateType::Normal);

			if (texShared) {
				RenderTexture(std::vector<texture_handle>{texShared}, canvasSize,
					      RECT(20, 50, rc.right - 20,
						   rc.bottom - 20)); // 渲染共享纹理
			}

			texture_handle fullTex = 0;
			for (auto &item : texRegions) {
				if (!item.first)
					continue;

				RenderTexture(std::vector<texture_handle>{item.first}, canvasSize,
					      item.second.region);

				if (item.second.selected) {
					RenderBorderWithSize(canvasSize, item.second.region,
							     BORDER_THICKNESS,
							     ST_Color(1.0f, 0.7f, 0.1f, 1.0f));
				}

				if (item.second.fullscreen)
					fullTex = item.first;
			}

			if (fullTex) {
				RenderTexture(std::vector<texture_handle>{fullTex}, canvasSize, rc);
			}

			pGraphic->EndRender();
		}
	}

	UnInitGraphic();
	return 0;
}

bool InitGraphic(HWND hWnd)
{
	auto listGraphic = EnumGraphicCard();
	assert(!listGraphic->empty() && "no valid graphic");
	if (listGraphic->empty())
		return false;

	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(nullptr);
	assert(bOK);

	//------------------------------------------------------------------
	InitShader();

	//------------------------------------------------------------------
	display = pGraphic->CreateDisplay(hWnd);
	assert(display);
	RECT rc;
	::GetClientRect(hWnd, &rc);
	pGraphic->SetDisplaySize(display, rc.right - rc.left, rc.bottom - rc.top);

	//------------------------------------------------------------------
	texGirl = pGraphic->OpenImageTexture(L"testGirl.png");

	texAlpha = pGraphic->OpenImageTexture(L"testAlpha.png");

	texImg = pGraphic->OpenImageTexture(L"test.jpg");

	texImg2 = pGraphic->OpenImageTexture(L"test.png");

	texShared = pGraphic->OpenSharedTexture((HANDLE)0X0000000040003282);

	//------------------------------- test texutres --------------------
	ST_TextureInfo info;
	info.width = 1440;
	info.height = 1080;
	info.format = DXGI_FORMAT_B8G8R8A8_UNORM;

	info.usage = TextureType::CanvasTarget;
	texCanvas = pGraphic->CreateTexture(info);
	assert(texCanvas);

	info.usage = TextureType::ReadTexture;
	texture_handle tex1 = pGraphic->CreateTexture(info);
	assert(tex1);
	pGraphic->DestroyGraphicObject(tex1);

	info.usage = TextureType::WriteTexture;
	texture_handle tex2 = pGraphic->CreateTexture(info);
	assert(tex2);
	pGraphic->DestroyGraphicObject(tex2);

	info.width = 1920;
	info.height = 1080;
	info.usage = TextureType::CanvasTarget;
	texForD2D = pGraphic->CreateTexture(info);

	//------------------------------------------------------------------
	return true;
}

void UnInitGraphic()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	if (pI420_To_RGB) {
		pI420_To_RGB->UninitConvertion();
		pI420_To_RGB.reset();
	}

	if (pYUYV_To_RGB) {
		pYUYV_To_RGB->UninitConvertion();
		pYUYV_To_RGB.reset();
	}

	close_file();
	if (preFrame)
		av_frame_free(&preFrame);

	pGraphic->DestroyAllGraphicObject();
	pGraphic->UnInitializeGraphic();
}

void InitRenderRect(RECT rc, int numH, int numV)
{
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	int cx = width / numH;
	int cy = height / numV;

	std::vector<RECT> renderRegion;
	for (int j = 0; j < numV; j++) {
		for (int i = 0; i < numH; i++) {
			RECT temp;
			temp.left = cx * i;
			temp.top = cy * j;
			temp.right = temp.left + cx;
			temp.bottom = temp.top + cy;

			temp.left += BORDER_THICKNESS;
			temp.top += BORDER_THICKNESS;
			temp.right -= BORDER_THICKNESS;
			temp.bottom -= BORDER_THICKNESS;

			renderRegion.push_back(temp);
		}
	}

	assert(texRegions.size() <= renderRegion.size());
	auto itr = texRegions.begin();
	for (size_t i = 0; i < texRegions.size() && i < renderRegion.size(); i++) {
		itr->second.region = renderRegion[i];
		++itr;
	}
}

void RenderCustomFormat(SIZE canvasSize, RECT rc)
{
	AVFrame *newFrame = decode_frame();
	if (!newFrame)
		return;

	if (preFrame)
		av_frame_free(&preFrame);

	preFrame = newFrame;
	if (!pI420_To_RGB) {
		video_convert_params params;
		params.graphic = pGraphic;
		params.width = preFrame->width;
		params.height = preFrame->height;
		params.format = (AVPixelFormat)preFrame->format;
		assert(params.format == destFormat);

		pI420_To_RGB = std::make_shared<FormatConvert_YUVToRGB>(params);
		pI420_To_RGB->InitConvertion();
	}

	pI420_To_RGB->RenderVideo(preFrame, canvasSize, rc);

	if (0) {
		FILE *fp = 0;
		fopen_s(&fp, "yuv", "wb+");
		if (fp) {
			for (size_t i = 0; i < preFrame->height; i++) {
				fwrite(preFrame->data[0] + i * preFrame->linesize[0],
				       preFrame->width, 1, fp);
			}

			for (size_t i = 0; i < preFrame->height / 2; i++) {
				fwrite(preFrame->data[1] + i * preFrame->linesize[1],
				       preFrame->width / 2, 1, fp);
			}

			for (size_t i = 0; i < preFrame->height / 2; i++) {
				fwrite(preFrame->data[2] + i * preFrame->linesize[2],
				       preFrame->width / 2, 1, fp);
			}

			fclose(fp);
		}
	}
}

void RenderYUYVFormat()
{
	if (!pYUYV_To_RGB) {
		video_convert_params params;
		params.graphic = pGraphic;
		params.width = frame_yuyv->width;
		params.height = frame_yuyv->height;
		params.format = (AVPixelFormat)frame_yuyv->format;

		pYUYV_To_RGB = std::make_shared<FormatConvert_YUVToRGB>(params);
		pYUYV_To_RGB->InitConvertion();

		ST_TextureInfo info;
		info.width = frame_yuyv->width;
		info.height = frame_yuyv->height;
		info.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		info.usage = TextureType::CanvasTarget;
		yuyvCanvas = pGraphic->CreateTexture(info);
	}

	if (pGraphic->BeginRenderCanvas(yuyvCanvas)) {
		pYUYV_To_RGB->RenderVideo(frame_yuyv, SIZE(frame_yuyv->width, frame_yuyv->height),
					  RECT(0, 0, frame_yuyv->width, frame_yuyv->height));
		pGraphic->EndRender();
	}
}
