
#include "pch.h"
#include "MFCDemoDlg.h"

unsigned __stdcall CMFCDemoDlg::ThreadFunc(void *pParam)
{
	float outputMatrix[4][4] = {0};
	TransposeMatrixWVP(SIZE(1920, 1080), SIZE(200, 300), RECT(20, 20, 100, 100), outputMatrix);

	ST_TextureVertex outputBuffer[4] = {0};
	TextureVertexBuffer(SIZE(100, 100), false, false, outputBuffer);

	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);
	IDX11GraphicInstance *m_pGraphic = self->m_pGraphic;

	auto listGraphic = EnumGraphicCard();
	assert(!listGraphic->empty());

	texture_handle tex = nullptr;
	display_handle display = nullptr;
	ST_TextureInfo texInfo;

	{
		AUTO_GRAPHIC_CONTEXT(m_pGraphic);

		bool bOK = m_pGraphic->InitializeGraphic(listGraphic->at(0).adapterLuid);
		assert(bOK);

		display = m_pGraphic->CreateDisplay(self->m_hWnd);
		assert(display);

		tex = m_pGraphic->OpenImageTexture(L"testAlpha.png");
		assert(tex);
		texInfo = m_pGraphic->GetTextureInfo(tex);

		ST_TextureInfo info;
		info.width = 201;
		info.height = 201;
		info.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		info.usage = TextureType::ReadTexture;
		texture_handle tex1 = m_pGraphic->CreateTexture(info);

		info.usage = TextureType::WriteTexture;
		texture_handle tex2 = m_pGraphic->CreateTexture(info);

		info.usage = TextureType::CanvasTarget;
		texture_handle tex3 = m_pGraphic->CreateTexture(info);

		m_pGraphic->ReleaseGraphicObject(tex1);
		m_pGraphic->ReleaseGraphicObject(tex2);
		m_pGraphic->ReleaseGraphicObject(tex3);
	}

	while (!self->m_bExit) {
		Sleep(20);

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);
		RECT dest;
		dest.left = 100;
		dest.top = 100;
		dest.right = 300;
		dest.bottom = 300;

		float outputMatrix[4][4];
		TransposeMatrixWVP(SIZE(rc.right - rc.left, rc.bottom - rc.top), SIZE(texInfo.width, texInfo.height), dest, outputMatrix);

		ST_TextureVertex outputVertex[4];
		TextureVertexBuffer(SIZE(texInfo.width, texInfo.height), false, false, outputVertex);

		std::vector<texture_handle> texs;
		texs.push_back(tex);

		AUTO_GRAPHIC_CONTEXT(m_pGraphic);
		m_pGraphic->SetVertexBuffer(0, outputVertex, 4 * sizeof(ST_TextureVertex));
		m_pGraphic->SetVSConstBuffer(0, &(outputMatrix[0][0]), 16 * sizeof(float));
		m_pGraphic->RenderBegin_Display(display, ST_Color(1.0, 0, 0, 1.0));
		m_pGraphic->DrawTexture(0, texs);
		m_pGraphic->RenderEnd();
	}

	{
		AUTO_GRAPHIC_CONTEXT(m_pGraphic);

		if (tex)
			m_pGraphic->ReleaseGraphicObject(tex);

		if (display)
			m_pGraphic->ReleaseGraphicObject(display);

		m_pGraphic->UnInitializeGraphic();
	}

	return 0;
}
