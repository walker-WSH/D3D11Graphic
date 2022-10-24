#pragma once
#include <DX11GraphicBase.h>

#define SWAPCHAIN_FORMAT DXGI_FORMAT_B8G8R8A8_UNORM

class DX11GraphicInstanceImpl;
struct DX11SwapChain : public DX11GraphicBase {
	friend class DX11GraphicInstanceImpl;

public:
	DX11SwapChain(DX11GraphicInstanceImpl &graphic, HWND hWnd);

	void SetDisplaySize(uint32_t width, uint32_t height);
	HRESULT TestResizeSwapChain();

	virtual bool BuildDX();
	virtual void ReleaseDX();
	virtual bool IsBuilt()
	{
		return m_pSwapChain && m_pSwapBackTexture2D && m_pRenderTargetView;
	}

private:
	HRESULT InitSwapChain();
	HRESULT CreateTargetView();

private:
	HWND m_hWnd = 0;
	uint32_t m_dwWidth = 0;
	uint32_t m_dwHeight = 0;

	ComPtr<IDXGISwapChain> m_pSwapChain = nullptr;
	ComPtr<ID3D11Texture2D> m_pSwapBackTexture2D = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;
};
