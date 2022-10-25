﻿#include "pch.h"
#include "MFCDemoDlg.h"
#include "decode_video.h"
#include "convert_video.h"
#include <assert.h>
#include <vector>
#include <map>

enum class ShaderType {
	shaderTexture = 0,
	shaderFillRect,
	shaderBorderRect,
	yuv420ToRGB,
};

float matrixWVP[4][4];

IDX11GraphicInstance *pGraphic = nullptr;
std::vector<DX11GraphicObject *> graphicList;
std::map<ShaderType, shader_handle> shaders;
display_handle display = nullptr;
texture_handle texGirl = nullptr;
texture_handle texShared = nullptr;
texture_handle texAlpha = nullptr;
texture_handle texImg = nullptr;
texture_handle texCanvas = nullptr;

FormatConvert_YUVToRGB i420Convert;

bool InitGraphic(HWND hWnd);
void UnInitGraphic();

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest);
void RenderRect(SIZE canvas, RECT drawDest, ST_Color clr);
void RenderBorder(SIZE canvas, RECT drawDest, ST_Color clr);
void RenderBorderWithSize(SIZE canvas, RECT drawDest, long borderSize, ST_Color clr);
void YUV2RGB(SIZE canvas, RECT drawDest);

unsigned __stdcall CMFCDemoDlg::ThreadFunc(void *pParam)
{
	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);
	pGraphic = self->m_pGraphic;

	if (!InitGraphic(self->m_hWnd))
		return 1;

	initVideo();

	{
		AUTO_GRAPHIC_CONTEXT(pGraphic);

		i420Convert.InitConvertion(frame, video_range_type::VIDEO_RANGE_FULL,
					   video_colorspace::VIDEO_CS_709);
		i420Convert.UpdateVideo(frame);
	}

	while (!self->m_bExit) {
		Sleep(50);

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		SIZE canvasSize(rc.right - rc.left, rc.bottom - rc.top);
		pGraphic->SetDisplaySize(display, canvasSize.cx, canvasSize.cy);

		RECT texDestRect;
		texDestRect.left = 50;
		texDestRect.top = 50;
		texDestRect.right = texDestRect.left + 250;
		texDestRect.bottom = texDestRect.top + 400;

		RECT tex2DestRect;
		tex2DestRect.left = texDestRect.right + 30;
		tex2DestRect.top = 50;
		tex2DestRect.right = tex2DestRect.left + 400;
		tex2DestRect.bottom = tex2DestRect.top + 300;

		RECT tex3DestRect;
		tex3DestRect.left = tex2DestRect.right + 30;
		tex3DestRect.top = 50;
		tex3DestRect.right = tex3DestRect.left + 500;
		tex3DestRect.bottom = tex3DestRect.top + 500;

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		if (!pGraphic->IsGraphicBuilt()) {
			if (!pGraphic->ReBuildGraphic())
				continue;
		}

		if (pGraphic->RenderBegin_Canvas(texCanvas, ST_Color(1.0f, 1.0f, 1.0f, 1.0f))) {
			auto info = pGraphic->GetTextureInfo(texCanvas);
			RenderRect(SIZE(info.width, info.height),
				   RECT(0, 0, info.width / 2, info.height / 2),
				   ST_Color(1.0, 0, 0, 1.0));
			pGraphic->RenderEnd();
		}

		if (pGraphic->RenderBegin_Display(display, ST_Color(0.3f, 0.3f, 0.3f, 1.0f))) {
			if (texShared) {
				// 渲染共享纹理
				RenderTexture(std::vector<texture_handle>{texShared}, canvasSize,
					      RECT(20, 50, rc.right - 20, rc.bottom - 20));
			}

			RenderTexture(std::vector<texture_handle>{texGirl}, canvasSize,
				      texDestRect);
			// 利用linestrip画矩形边框
			RenderBorder(canvasSize, texDestRect, ST_Color(1.0, 0, 0, 1.0));

			RenderTexture(std::vector<texture_handle>{texAlpha}, canvasSize,
				      tex2DestRect);
			// 利用填充矩形画指定厚度的矩形边框
			RenderBorderWithSize(canvasSize, tex2DestRect, 4, ST_Color(1.0, 0, 0, 1.0));

			RenderTexture(std::vector<texture_handle>{texImg}, canvasSize,
				      tex3DestRect);

			// 填充纯色矩形区域
			RenderRect(canvasSize, RECT(20, 10, rc.right - 20, 30),
				   ST_Color(0, 0, 1.0, 1.0));

			// 画布也可以直接当作resource进行渲染
			RenderTexture(std::vector<texture_handle>{texCanvas}, canvasSize,
				      RECT(10, 500, 410, 700));

			YUV2RGB(canvasSize, rc);

			pGraphic->RenderEnd();
		}
	}

	UnInitGraphic();

	return 0;
}

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ShaderType type = ShaderType::shaderTexture;
	ST_TextureInfo texInfo = pGraphic->GetTextureInfo(texs.at(0));
	SIZE texSize(texInfo.width, texInfo.height);

	TransposeMatrixWVP(canvas, texSize, drawDest, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, outputVertex);

	pGraphic->SetVertexBuffer(shaders[type], outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shaders[type], &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->DrawTexture(shaders[type], texs);
}

void YUV2RGB(SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	std::vector<texture_handle> texs = i420Convert.GetTextures();
	const ST_PSConstBuffer *psBuf = i420Convert.GetPSBuffer();

	ShaderType type = ShaderType::yuv420ToRGB;
	SIZE texSize((LONG)i420Convert.GetPSBuffer()->width,
		     (LONG)i420Convert.GetPSBuffer()->height);

	TransposeMatrixWVP(canvas, texSize, drawDest, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, outputVertex);

	pGraphic->SetVertexBuffer(shaders[type], outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shaders[type], &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->SetPSConstBuffer(shaders[type], psBuf, sizeof(ST_PSConstBuffer));
	pGraphic->DrawTexture(shaders[type], texs);
}

void RenderRect(SIZE canvas, RECT drawDest, ST_Color clr)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ShaderType type = ShaderType::shaderFillRect;
	SIZE texSize(drawDest.right - drawDest.left, drawDest.bottom - drawDest.top);
	TransposeMatrixWVP(canvas, texSize, drawDest, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, outputVertex);

	pGraphic->SetVertexBuffer(shaders[type], outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shaders[type], &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->SetPSConstBuffer(shaders[type], &clr, sizeof(ST_Color));
	pGraphic->DrawTopplogy(shaders[type], D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void RenderBorderWithSize(SIZE canvas, RECT drawDest, long borderSize, ST_Color clr)
{
	RECT rcLeft;
	RECT rcRight;
	RECT rcTop;
	RECT rcBottom;

	rcLeft.right = drawDest.left;
	rcLeft.left = drawDest.left - borderSize;
	rcLeft.top = drawDest.top - borderSize;
	rcLeft.bottom = drawDest.bottom + borderSize;

	rcRight.left = drawDest.right;
	rcRight.right = drawDest.right + borderSize;
	rcRight.top = drawDest.top - borderSize;
	rcRight.bottom = drawDest.bottom + borderSize;

	rcTop.left = drawDest.left - borderSize;
	rcTop.right = drawDest.right + borderSize;
	rcTop.top = drawDest.top - borderSize;
	rcTop.bottom = drawDest.top;

	rcBottom.left = drawDest.left - borderSize;
	rcBottom.right = drawDest.right + borderSize;
	rcBottom.top = drawDest.bottom;
	rcBottom.bottom = drawDest.bottom + borderSize;

	RenderRect(canvas, rcLeft, clr);
	RenderRect(canvas, rcRight, clr);
	RenderRect(canvas, rcTop, clr);
	RenderRect(canvas, rcBottom, clr);
}

void RenderBorder(SIZE canvas, RECT drawDest, ST_Color clr)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ShaderType type = ShaderType::shaderBorderRect;
	SIZE texSize(drawDest.right - drawDest.left, drawDest.bottom - drawDest.top);
	TransposeMatrixWVP(canvas, texSize, drawDest, matrixWVP);

	ST_TextureVertex outputVertex[RECT_LINE_VERTEX_COUNT];
	VertexList_RectLine(texSize, outputVertex);

	pGraphic->SetVertexBuffer(shaders[type], outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shaders[type], &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->SetPSConstBuffer(shaders[type], &clr, sizeof(ST_Color));
	pGraphic->DrawTopplogy(shaders[type], D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
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

	//------------------------------------------------------------------
	shaderInfo.vsFile = L"rectVS.cso";
	shaderInfo.psFile = L"rectPS.cso";
	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = sizeof(float) * 4;
	shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
	shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
	shader_handle rectShader = pGraphic->CreateShader(shaderInfo);
	assert(rectShader);
	graphicList.push_back(rectShader);
	shaders[ShaderType::shaderFillRect] = rectShader;

	shaderInfo.vsFile = L"defaultVS.cso";
	shaderInfo.psFile = L"convertToRGB_PS.cso";
	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = sizeof(ST_PSConstBuffer);
	shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
	shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
	shader_handle i420Shader = pGraphic->CreateShader(shaderInfo);
	assert(i420Shader);
	graphicList.push_back(i420Shader);
	shaders[ShaderType::yuv420ToRGB] = i420Shader;

	shaderInfo.vsFile = L"rectVS.cso";
	shaderInfo.psFile = L"rectPS.cso";
	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = sizeof(float) * 4;
	shaderInfo.vertexCount = RECT_LINE_VERTEX_COUNT;
	shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
	shader_handle borderShader = pGraphic->CreateShader(shaderInfo);
	assert(borderShader);
	graphicList.push_back(borderShader);
	shaders[ShaderType::shaderBorderRect] = borderShader;

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

	texShared = pGraphic->OpenSharedTexture((HANDLE)0X0000000040003282);
	if (texShared)
		graphicList.push_back(texShared);

	//------------------------------- test texutres --------------------
	ST_TextureInfo info;
	info.width = 400;
	info.height = 400;
	info.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	info.usage = TextureType::ReadTexture;
	texture_handle tex1 = pGraphic->CreateTexture(info);

	info.usage = TextureType::WriteTexture;
	texture_handle tex2 = pGraphic->CreateTexture(info);

	info.usage = TextureType::CanvasTarget;
	texture_handle tex3 = pGraphic->CreateTexture(info);

	info.usage = TextureType::CanvasTarget;
	texCanvas = pGraphic->CreateTexture(info);
	assert(texCanvas);
	graphicList.push_back(texCanvas);

	pGraphic->ReleaseGraphicObject(tex1);
	pGraphic->ReleaseGraphicObject(tex2);
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
	i420Convert.UninitConvertion();

	pGraphic->UnInitializeGraphic();
}
