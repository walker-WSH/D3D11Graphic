#pragma once
#include <DX11GraphicBase.h>

class DX11GraphicInstanceImpl;
class DX11Texture2D : public DX11GraphicBase {
	friend class DX11GraphicInstanceImpl;

public:
	DX11Texture2D(DX11GraphicInstanceImpl &graphic, const ST_TextureInfo &info, TextureType type);
	DX11Texture2D(DX11GraphicInstanceImpl &graphic, HANDLE handle);

	virtual bool BuildDX();
	virtual void ReleaseDX();
	virtual bool IsBuilt() { return m_pTexture2D; }

protected:
	bool InitWriteTexture();
	bool InitReadTexture();
	bool InitTargetTexture();
	bool InitSharedTexture();
	bool InitResourceView();

protected:
	D3D11_TEXTURE2D_DESC m_descTexture;
	ComPtr<ID3D11Texture2D> m_pTexture2D = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pTextureResView = nullptr;

	HANDLE m_hSharedHandle = 0;
	ST_TextureInfo m_textureInfo;
	enum TextureType m_usage;
};
