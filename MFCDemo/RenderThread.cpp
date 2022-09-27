
#include "pch.h"
#include "MFCDemoDlg.h"

unsigned __stdcall CMFCDemoDlg::ThreadFunc(void *pParam)
{
	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);
	IDX11GraphicInstance *m_pGraphic = self->m_pGraphic;

	auto listGraphic = EnumGraphicCard();
	assert(!listGraphic->empty());

	{
		AUTO_GRAPHIC_CONTEXT(m_pGraphic);
		m_pGraphic->InitializeGraphic(listGraphic->at(0).adapterLuid);
	}

	while (!self->m_bExit) {
		Sleep(20);

		AUTO_GRAPHIC_CONTEXT(m_pGraphic);

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

	{
		AUTO_GRAPHIC_CONTEXT(m_pGraphic);
		m_pGraphic->UnInitializeGraphic();
	}

	return 0;
}
