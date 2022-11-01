#include "pch.h"
#include "MFCDemoDlg.h"
#include "video-frame.h"
#include "render-interface-wrapper.h"
#include <assert.h>
#include <vector>
#include <memory>
#include <map>

#define BORDER_THICKNESS 3

display_handle display = nullptr;
texture_handle texCanvas = nullptr;
texture_handle texGirl = nullptr;
texture_handle texShared = nullptr;
texture_handle texAlpha = nullptr;
texture_handle texImg = nullptr;
texture_handle texImg2 = nullptr;

CPoint posLBDown = {0, 0};
std::vector<RECT> renderRegion;

std::shared_ptr<FormatConvert_YUVToRGB> pI420_To_RGB = nullptr;
std::shared_ptr<FormatConvert_YUVToRGB> pYUYV_To_RGB = nullptr;
AVFrame *preFrame = nullptr;

bool InitGraphic(HWND hWnd);
void UnInitGraphic();

void InitRenderRect(RECT rc, int numH, int numV);
int GetSelectRegionIndex();

void RenderCustomFormat(SIZE canvasSize, RECT rc);

texture_handle yuyvCanvas = nullptr;
void RenderYUYVFormat();

void CMFCDemoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	posLBDown = point;
	CDialogEx::OnLButtonDown(nFlags, point);
}

unsigned __stdcall CMFCDemoDlg::ThreadFunc(void *pParam)
{
	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);
	pGraphic = self->m_pGraphic;

	bool saved = false;

	if (!InitGraphic(self->m_hWnd))
		return 1;

	assert(open_file() == 0);

	while (!self->m_bExit) {
		Sleep(20);

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		rc.right = (rc.right / 2) * 2;
		rc.bottom = (rc.bottom / 2) * 2;

		SIZE canvasSize(rc.right - rc.left, rc.bottom - rc.top);
		pGraphic->SetDisplaySize(display, canvasSize.cx, canvasSize.cy);

		InitRenderRect(rc, 3, 2);

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		if (!pGraphic->IsGraphicBuilt()) {
			if (!pGraphic->ReBuildGraphic())
				continue;
		}

		RenderYUYVFormat();

		if (pGraphic->RenderBegin_Canvas(texCanvas, ST_Color(1.0f, 1.0f, 1.0f, 1.0f))) {
			auto info = pGraphic->GetTextureInfo(texCanvas);

			auto img = pGraphic->GetTextureInfo(texImg);
			RenderTexture(std::vector<texture_handle>{texImg},
				      SIZE(info.width, info.height),
				      RECT(0, 0, img.width, img.height));

			FillRectangle(SIZE(info.width, info.height),
				      RECT(0, info.height / 10 * 9, info.width / 3, info.height),
				      ST_Color(1.0, 0, 0, 1.0));
			FillRectangle(SIZE(info.width, info.height),
				      RECT(info.width / 3, info.height / 10 * 9, info.width / 3 * 2,
					   info.height),
				      ST_Color(0, 1.0, 0, 1.0));
			FillRectangle(SIZE(info.width, info.height),
				      RECT(info.width / 3 * 2, info.height / 10 * 9, info.width,
					   info.height),
				      ST_Color(0, 0, 1, 1.0));

			RenderCustomFormat(canvasSize, rc);

			pGraphic->RenderEnd();

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

		if (pGraphic->RenderBegin_Display(display, ST_Color(0.3f, 0.3f, 0.3f, 1.0f))) {
			RECT rcLeft = rc;
			rcLeft.right = rcLeft.left + (rc.right - rc.left) / 2;
			RenderTexture(std::vector<texture_handle>{texImg2}, canvasSize, rcLeft);

			if (1) {
				if (texShared) {
					RenderTexture(std::vector<texture_handle>{texShared},
						      canvasSize,
						      RECT(20, 50, rc.right - 20,
							   rc.bottom - 20)); // 渲染共享纹理
				}

				RenderTexture(std::vector<texture_handle>{texAlpha}, canvasSize,
					      renderRegion[0]);

				RenderTexture(std::vector<texture_handle>{texGirl}, canvasSize,
					      renderRegion[1]);

				RenderTexture(std::vector<texture_handle>{texImg}, canvasSize,
					      renderRegion[2]);

				FillRectangle(canvasSize, renderRegion[3],
					      ST_Color(0, 0, 1.0, 1.0)); // 填充纯色矩形区域

				RenderTexture(
					std::vector<texture_handle>{texCanvas}, canvasSize,
					renderRegion[4]); // 画布也可以直接当作resource进行渲染

				if (1) {
					// 先把yuv转为RGB 再渲染到目标区域
					// 这个方法 清晰度明显好一些
					RenderTexture(std::vector<texture_handle>{yuyvCanvas},
						      canvasSize, renderRegion[5]);
				} else {
					// 直接将yuv转换并渲染到目标区域
					// 这个方法 清晰度明显低一些
					pYUYV_To_RGB->RenderVideo(frame_yuy2, canvasSize,
								  renderRegion[5]);
				}
			}

			int index = GetSelectRegionIndex();
			if (index >= 0) {
				RenderBorderWithSize(canvasSize, renderRegion[index],
						     BORDER_THICKNESS,
						     ST_Color(1.0f, 0.7f, 0.1f, 1.0f));
			}

			pGraphic->RenderEnd();
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
	info.width = 1920;
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

	info.usage = TextureType::CanvasTarget;
	texture_handle tex3 = pGraphic->CreateTexture(info);
	assert(tex3);
	pGraphic->DestroyGraphicObject(tex3);

	//------------------------------------------------------------------
	return true;
}

void UnInitGraphic()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	pI420_To_RGB->UninitConvertion();
	pI420_To_RGB.reset();

	pYUYV_To_RGB->UninitConvertion();
	pYUYV_To_RGB.reset();

	pGraphic->DestroyAllGraphicObject();
	pGraphic->UnInitializeGraphic();
}

void InitRenderRect(RECT rc, int numH, int numV)
{
	renderRegion.clear();

	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	int cx = width / numH;
	int cy = height / numV;

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
}

int GetSelectRegionIndex()
{
	for (size_t i = 0; i < renderRegion.size(); i++) {
		const auto &rc = renderRegion[i];
		if (posLBDown.x > rc.left && posLBDown.x < rc.right && posLBDown.y > rc.top &&
		    posLBDown.y < rc.bottom) {
			return (int)i;
		}
	}

	return -1;
}

void RenderCustomFormat(SIZE canvasSize, RECT rc)
{
	AVFrame *newFrame = decode_frame();
	if (newFrame) {
		if (preFrame) {
			av_frame_free(&preFrame);
			preFrame = nullptr;
		}

		preFrame = newFrame;
	}

	if (preFrame) {
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
}

void RenderYUYVFormat()
{
	if (!pYUYV_To_RGB) {
		video_convert_params params;
		params.graphic = pGraphic;
		params.width = frame_yuy2->width;
		params.height = frame_yuy2->height;
		params.format = (AVPixelFormat)frame_yuy2->format;

		pYUYV_To_RGB = std::make_shared<FormatConvert_YUVToRGB>(params);
		pYUYV_To_RGB->InitConvertion();

		ST_TextureInfo info;
		info.width = frame_yuy2->width;
		info.height = frame_yuy2->height;
		info.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		info.usage = TextureType::CanvasTarget;
		yuyvCanvas = pGraphic->CreateTexture(info);
	}

	if (pGraphic->RenderBegin_Canvas(yuyvCanvas, ST_Color(0, 0, 0, 0))) {
		pYUYV_To_RGB->RenderVideo(frame_yuy2, SIZE(frame_yuy2->width, frame_yuy2->height),
					  RECT(0, 0, frame_yuy2->width, frame_yuy2->height));
		pGraphic->RenderEnd();
	}
}
