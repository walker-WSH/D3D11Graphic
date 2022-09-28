
#include "pch.h"
#include "MFCDemoDlg.h"
#include <assert.h>
#include <vector>

IDX11GraphicInstance *pGraphic = nullptr;
std::vector<DX11GraphicObject *> graphicList;

shader_handle texShader = nullptr;
shader_handle rectShader = nullptr;
texture_handle texGirl = nullptr;
texture_handle texShared = nullptr;
texture_handle texAlpha = nullptr;
display_handle display = nullptr;

void InitGraphic(HWND hWnd);
void UnInitGraphic();

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest);
void RenderRect(SIZE canvas, RECT drawDest);

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
		pGraphic->RenderBegin_Display(display, ST_Color(0.3f, 0.3f, 0.3f, 1.0f));

		if (texShared)
			RenderTexture(std::vector<texture_handle>{texShared}, canvasSize, tex3DestRect);

		RenderTexture(std::vector<texture_handle>{texGirl}, canvasSize, texDestRect);
		RenderTexture(std::vector<texture_handle>{texAlpha}, canvasSize, tex2DestRect);
		RenderRect(canvasSize, rectFill);

		pGraphic->RenderEnd();
	}

	UnInitGraphic();

	return 0;
}

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	ST_TextureInfo texInfo = pGraphic->GetTextureInfo(texs.at(0));
	SIZE texSize(texInfo.width, texInfo.height);

	float outputMatrix[4][4];
	TransposeMatrixWVP(canvas, texSize, drawDest, outputMatrix);

	ST_TextureVertex outputVertex[4];
	TextureVertexBuffer(texSize, false, false, outputVertex);

	pGraphic->SetVertexBuffer(texShader, outputVertex, 4 * sizeof(ST_TextureVertex));
	pGraphic->SetVSConstBuffer(texShader, &(outputMatrix[0][0]), 16 * sizeof(float));
	pGraphic->DrawTexture(texShader, texs);
}

void RenderRect(SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	SIZE texSize(drawDest.right - drawDest.left, drawDest.bottom - drawDest.top);

	float outputMatrix[4][4];
	TransposeMatrixWVP(canvas, texSize, drawDest, outputMatrix);

	ST_TextureVertex outputVertex[4];
	TextureVertexBuffer(texSize, false, false, outputVertex);

	ST_Color fillColor;
	fillColor.red = 1.0;
	fillColor.alpha = 1.0;

	pGraphic->SetVertexBuffer(rectShader, outputVertex, 4 * sizeof(ST_TextureVertex));
	pGraphic->SetVSConstBuffer(rectShader, &(outputMatrix[0][0]), 16 * sizeof(float));
	pGraphic->SetPSConstBuffer(rectShader, &fillColor, 4 * sizeof(float));
	pGraphic->FillRectangle(rectShader);
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
	shaderInfo.vsBufferSize = sizeof(float) * 16;
	shaderInfo.psBufferSize = 0;
	shaderInfo.vertexCount = 4;
	shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
	texShader = pGraphic->CreateShader(shaderInfo);
	assert(texShader);
	graphicList.push_back(texShader);

	//------------------------------------------------------------------
	shaderInfo.vsFile = L"rectVS.cso";
	shaderInfo.psFile = L"rectPS.cso";
	shaderInfo.vsBufferSize = sizeof(float) * 16;
	shaderInfo.psBufferSize = sizeof(float) * 4;
	shaderInfo.vertexCount = 4;
	shaderInfo.perVertexSize = sizeof(ST_TextureVertex);
	rectShader = pGraphic->CreateShader(shaderInfo);
	assert(rectShader);
	graphicList.push_back(rectShader);

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
