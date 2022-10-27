﻿#include "pch.h"
#include "MFCDemoDlg.h"
#include "video-frame.h"
#include "render-interface-wrapper.h"
#include <assert.h>
#include <vector>
#include <memory>
#include <map>

#define BORDER_THICKNESS 3

std::vector<DX11GraphicObject *> graphicList;
std::vector<RECT> renderRegion;
display_handle display = nullptr;
texture_handle texCanvas = nullptr;
texture_handle texGirl = nullptr;
texture_handle texShared = nullptr;
texture_handle texAlpha = nullptr;
texture_handle texImg = nullptr;
texture_handle texImg2 = nullptr;

bool InitGraphic(HWND hWnd);
void UnInitGraphic();
void InitRenderRect(RECT rc, int numH, int numV);

unsigned __stdcall CMFCDemoDlg::ThreadFunc(void *pParam)
{
	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);
	pGraphic = self->m_pGraphic;

	if (!InitGraphic(self->m_hWnd))
		return 1;

	{
		initVideo();

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		video_convert_params params;
		params.width = frame->width;
		params.height = frame->height;
		params.format = (AVPixelFormat)frame->format;

		pI4202RGB = std::make_shared<FormatConvert_YUVToRGB>(params);
		pI4202RGB->InitConvertion();
		pI4202RGB->UpdateVideo(frame);
	}

	while (!self->m_bExit) {
		Sleep(50);

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		SIZE canvasSize(rc.right - rc.left, rc.bottom - rc.top);
		pGraphic->SetDisplaySize(display, canvasSize.cx, canvasSize.cy);

		InitRenderRect(rc, 3, 2);

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		if (!pGraphic->IsGraphicBuilt()) {
			if (!pGraphic->ReBuildGraphic())
				continue;
		}

		if (pGraphic->RenderBegin_Canvas(texCanvas, ST_Color(1.0f, 1.0f, 1.0f, 1.0f))) {
			auto info = pGraphic->GetTextureInfo(texCanvas);
			FillRect(SIZE(info.width, info.height),
				 RECT(0, 0, info.width / 2, info.height / 2),
				 ST_Color(1.0, 0, 0, 1.0));
			pGraphic->RenderEnd();
		}

		if (pGraphic->RenderBegin_Display(display, ST_Color(0.3f, 0.3f, 0.3f, 1.0f))) {
			RECT rcLeft = rc;
			rcLeft.right = rcLeft.left + (rc.right - rc.left) / 2;
			RenderTexture(std::vector<texture_handle>{texImg2}, canvasSize, rcLeft);

			RECT rcRight = rc;
			rcRight.left = (rc.right - rc.left) / 2;
			YUV_To_RGB(canvasSize, rcRight);

			if (1) {
				if (texShared) {
					RenderTexture(std::vector<texture_handle>{texShared},
						      canvasSize,
						      RECT(20, 50, rc.right - 20,
							   rc.bottom - 20)); // 渲染共享纹理
				}

				RenderTexture(std::vector<texture_handle>{texAlpha}, canvasSize,
					      renderRegion[0]);
				RenderBorderWithSize(canvasSize, renderRegion[0], BORDER_THICKNESS,
						     ST_Color(1.0, 1.0, 0, 1.0));

				RenderTexture(std::vector<texture_handle>{texGirl}, canvasSize,
					      renderRegion[1]);

				RenderTexture(std::vector<texture_handle>{texImg}, canvasSize,
					      renderRegion[2]);

				FillRect(canvasSize, renderRegion[3],
					 ST_Color(0, 0, 1.0, 1.0)); // 填充纯色矩形区域

				RenderTexture(
					std::vector<texture_handle>{texCanvas}, canvasSize,
					renderRegion[4]); // 画布也可以直接当作resource进行渲染
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
	ST_ShaderInfo shaderInfo;
	shaderInfo.vsFile = L"defaultVS.cso";
	shaderInfo.psFile = L"defaultPS.cso";
	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = 0;
	shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
	shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
	shader_handle texShader = pGraphic->CreateShader(shaderInfo);
	assert(texShader);
	graphicList.push_back(texShader);
	shaders[ShaderType::shaderTexture] = texShader;

	shaderInfo.vsFile = L"rectVS.cso";
	shaderInfo.psFile = L"rectPS.cso";
	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = sizeof(ST_Color);
	shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
	shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
	shader_handle rectShader = pGraphic->CreateShader(shaderInfo);
	assert(rectShader);
	graphicList.push_back(rectShader);
	shaders[ShaderType::shaderFillRect] = rectShader;

	shaderInfo.vsFile = L"defaultVS.cso";
	shaderInfo.psFile = L"convertToRGB_PS.cso";
	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = sizeof(torgb_const_buffer);
	shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
	shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
	shader_handle i420Shader = pGraphic->CreateShader(shaderInfo);
	assert(i420Shader);
	graphicList.push_back(i420Shader);
	shaders[ShaderType::yuv420ToRGB] = i420Shader;

	//------------------------------------------------------------------
	display = pGraphic->CreateDisplay(hWnd);
	assert(display);
	RECT rc;
	::GetClientRect(hWnd, &rc);
	pGraphic->SetDisplaySize(display, rc.right - rc.left, rc.bottom - rc.top);
	graphicList.push_back(display);

	//------------------------------------------------------------------
	texGirl = pGraphic->OpenImageTexture(L"testGirl.png");
	graphicList.push_back(texGirl);

	texAlpha = pGraphic->OpenImageTexture(L"testAlpha.png");
	graphicList.push_back(texAlpha);

	texImg = pGraphic->OpenImageTexture(L"test.jpg");
	graphicList.push_back(texImg);

	texImg2 = pGraphic->OpenImageTexture(L"test.png");
	graphicList.push_back(texImg2);

	texShared = pGraphic->OpenSharedTexture((HANDLE)0X0000000040003282);
	if (texShared)
		graphicList.push_back(texShared);

	//------------------------------- test texutres --------------------
	ST_TextureInfo info;
	info.width = 400;
	info.height = 400;
	info.format = DXGI_FORMAT_B8G8R8A8_UNORM;

	info.usage = TextureType::CanvasTarget;
	texCanvas = pGraphic->CreateTexture(info);
	assert(texCanvas);
	graphicList.push_back(texCanvas);

	info.usage = TextureType::ReadTexture;
	texture_handle tex1 = pGraphic->CreateTexture(info);
	assert(tex1);
	pGraphic->ReleaseGraphicObject(tex1);

	info.usage = TextureType::WriteTexture;
	texture_handle tex2 = pGraphic->CreateTexture(info);
	assert(tex2);
	pGraphic->ReleaseGraphicObject(tex2);

	info.usage = TextureType::CanvasTarget;
	texture_handle tex3 = pGraphic->CreateTexture(info);
	assert(tex3);
	pGraphic->ReleaseGraphicObject(tex3);

	//------------------------------------------------------------------
	return true;
}

void UnInitGraphic()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	for (auto &item : graphicList)
		pGraphic->ReleaseGraphicObject(item);
	graphicList.clear();

	pI4202RGB->UninitConvertion();
	pI4202RGB.reset();

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
