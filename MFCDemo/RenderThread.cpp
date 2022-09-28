
#include "pch.h"
#include "MFCDemoDlg.h"
#include <assert.h>
#include <vector>
#include <map>

enum class ShaderType {
	shaderTexture = 0,
	shaderFillRect,
	shaderBorderRect,
};

float matrixWVP[4][4];

IDX11GraphicInstance *pGraphic = nullptr;
std::vector<DX11GraphicObject *> graphicList;
std::map<ShaderType, shader_handle> shaders;
display_handle display = nullptr;
texture_handle texGirl = nullptr;
texture_handle texShared = nullptr;
texture_handle texAlpha = nullptr;

void InitGraphic(HWND hWnd);
void UnInitGraphic();

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest);
void RenderRect(SIZE canvas, RECT drawDest);
void RenderBorder(SIZE canvas, RECT drawDest, ST_Color clr);

unsigned __stdcall CMFCDemoDlg::ThreadFunc(void *pParam)
{
	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);
	pGraphic = self->m_pGraphic;

	InitGraphic(self->m_hWnd);

	while (!self->m_bExit) {
		Sleep(20);

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
		tex2DestRect.left = texDestRect.right + 20;
		tex2DestRect.top = 50;
		tex2DestRect.right = tex2DestRect.left + 250;
		tex2DestRect.bottom = tex2DestRect.top + 400;

		RECT tex3DestRect;
		tex3DestRect.left = 20;
		tex3DestRect.top = 50;
		tex3DestRect.right = rc.right - 20;
		tex3DestRect.bottom = rc.bottom - 20;

		RECT rectFill;
		rectFill.left = 20;
		rectFill.top = 10;
		rectFill.right = rc.right - 20;
		rectFill.bottom = rectFill.top + 30;

		AUTO_GRAPHIC_CONTEXT(pGraphic);
		if (pGraphic->RenderBegin_Display(display, ST_Color(0.3f, 0.3f, 0.3f, 1.0f))) {
			if (texShared)
				RenderTexture(std::vector<texture_handle>{texShared}, canvasSize, tex3DestRect);

			RenderTexture(std::vector<texture_handle>{texGirl}, canvasSize, texDestRect);
			RenderBorder(canvasSize, texDestRect, ST_Color(1.0, 0, 0, 1.0));
			texDestRect.left -= 1;
			texDestRect.top -= 1;
			texDestRect.right -= 1;
			texDestRect.bottom -= 1;
			RenderBorder(canvasSize, texDestRect, ST_Color(1.0, 0, 0, 1.0));

			RenderTexture(std::vector<texture_handle>{texAlpha}, canvasSize, tex2DestRect);
			RenderBorder(canvasSize, tex2DestRect, ST_Color(1.0, 1.0, 0, 1.0));

			RenderRect(canvasSize, rectFill);

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

void RenderRect(SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ShaderType type = ShaderType::shaderFillRect;
	SIZE texSize(drawDest.right - drawDest.left, drawDest.bottom - drawDest.top);
	TransposeMatrixWVP(canvas, texSize, drawDest, matrixWVP);

	ST_TextureVertex outputVertex[TEXTURE_VERTEX_COUNT];
	VertexList_RectTriangle(texSize, false, false, outputVertex);

	ST_Color fillColor;
	fillColor.red = 1.0;
	fillColor.alpha = 1.0;

	pGraphic->SetVertexBuffer(shaders[type], outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shaders[type], &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->SetPSConstBuffer(shaders[type], &fillColor, sizeof(fillColor));
	pGraphic->DrawTopplogy(shaders[type], D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
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

void InitGraphic(HWND hWnd)
{
	auto listGraphic = EnumGraphicCard();
	assert(!listGraphic->empty());

	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(listGraphic->at(0).adapterLuid);
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
	assert(texGirl);
	graphicList.push_back(texGirl);

	texAlpha = pGraphic->OpenImageTexture(L"testAlpha.png");
	assert(texAlpha);
	graphicList.push_back(texAlpha);

	texShared = pGraphic->OpenSharedTexture((HANDLE)0X0000000040003282);
	if (texShared)
		graphicList.push_back(texShared);

	//------------------------------- test texutres --------------------
	ST_TextureInfo info;
	info.width = 201;
	info.height = 201;
	info.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	info.usage = TextureType::ReadTexture;
	texture_handle tex1 = pGraphic->CreateTexture(info);

	info.usage = TextureType::WriteTexture;
	texture_handle tex2 = pGraphic->CreateTexture(info);

	info.usage = TextureType::CanvasTarget;
	texture_handle tex3 = pGraphic->CreateTexture(info);

	pGraphic->ReleaseGraphicObject(tex1);
	pGraphic->ReleaseGraphicObject(tex2);
	pGraphic->ReleaseGraphicObject(tex3);
	//------------------------------------------------------------------
}

void UnInitGraphic()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	for (auto &item : graphicList)
		pGraphic->ReleaseGraphicObject(item);

	graphicList.clear();
	pGraphic->UnInitializeGraphic();
}
