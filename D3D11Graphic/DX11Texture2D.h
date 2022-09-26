#pragma once
#include <DX11Object.h>

enum class TextureType {
	RenderTarget = 0,
	ReadTexture,
	WriteTexture,
	SharedHandle,
};

class DX11GraphicInstanceImpl;
class DX11Texture2D : public DX11Object {
	friend class DX11GraphicInstanceImpl;

public:
	DX11Texture2D(DX11GraphicInstanceImpl &graphic, uint32_t width, uint32_t height, enum DXGI_FORMAT format, TextureType type);
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
	uint32_t m_dwWidth = 0;
	uint32_t m_dwHeight = 0;
	enum DXGI_FORMAT m_format = DXGI_FORMAT_B8G8R8A8_UNORM;
	enum TextureType m_usage;
};
