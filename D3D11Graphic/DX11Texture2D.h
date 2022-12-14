#pragma once
#include <DX11GraphicBase.h>

class DX11GraphicSession;
class DX11Texture2D : public DX11GraphicBase {
	friend class DX11GraphicSession;

public:
	DX11Texture2D(DX11GraphicSession &graphic, const ST_TextureInfo &info);
	DX11Texture2D(DX11GraphicSession &graphic, HANDLE handle);
	DX11Texture2D(DX11GraphicSession &graphic, const WCHAR *fullPath);

	virtual const char *GetName() { return "texture"; }
	virtual bool BuildDX();
	virtual void ReleaseDX();
	virtual bool IsBuilt() { return m_pTexture2D; }

protected:
	bool InitWriteTexture();
	bool InitReadTexture();
	bool InitTargetTexture();
	bool InitSharedTexture();
	bool InitImageTexture();
	bool InitResourceView();

protected:
	D3D11_TEXTURE2D_DESC m_descTexture = {};
	ComPtr<ID3D11Texture2D> m_pTexture2D = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pTextureResView = nullptr;

	ComPtr<ID2D1Factory> m_pD2D1Factory = nullptr;
	ComPtr<ID2D1RenderTarget> m_pD2D1RenderTarget = nullptr;
	ComPtr<ID2D1SolidColorBrush> m_pD2D1SolidBrush = nullptr;
	void testD2D();

	HANDLE m_hSharedHandle = 0;
	std::wstring m_strImagePath = L"";
	ST_TextureInfo m_textureInfo = {};
};
