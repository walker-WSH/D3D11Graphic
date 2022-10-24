#include "DX11SwapChain.h"
#include "DX11GraphicInstanceImpl.h"

DX11SwapChain::DX11SwapChain(DX11GraphicInstanceImpl &graphic, HWND hWnd)
	: DX11GraphicBase(graphic), m_hWnd(hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	m_dwWidth = rc.right - rc.left;
	m_dwHeight = rc.bottom - rc.top;

	BuildDX();
}

bool DX11SwapChain::BuildDX()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	HRESULT hr = InitSwapChain();
	if (FAILED(hr)) {
		CheckDXError(hr);
		ReleaseDX();
		return false;
	}

	return true;
}

void DX11SwapChain::ReleaseDX()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	m_pSwapChain = nullptr;
	m_pSwapBackTexture2D = nullptr;
	m_pRenderTargetView = nullptr;
}

void DX11SwapChain::SetDisplaySize(uint32_t width, uint32_t height)
{
	m_dwWidth = width;
	m_dwHeight = height;
}

HRESULT DX11SwapChain::TestResizeSwapChain()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	if (!m_pSwapChain || !m_dwWidth || !m_dwHeight)
		return S_FALSE;

	D3D11_TEXTURE2D_DESC desc;
	m_pSwapBackTexture2D->GetDesc(&desc);

	if (desc.Width != m_dwWidth || desc.Height != m_dwHeight) {
		ID3D11RenderTargetView *pRenderView = NULL;
		m_graphic.DXContext()->OMSetRenderTargets(1, &pRenderView, NULL);

		m_pRenderTargetView = nullptr;
		m_pSwapBackTexture2D = nullptr;

		HRESULT hr =
			m_pSwapChain->ResizeBuffers(1, m_dwWidth, m_dwHeight, SWAPCHAIN_FORMAT, 0);
		if (FAILED(hr)) {
			CheckDXError(hr);
			return hr;
		}

		return CreateTargetView();
	}

	return S_OK;
}

HRESULT DX11SwapChain::InitSwapChain()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	assert(m_dwWidth > 0 && m_dwHeight > 0);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferDesc.Width = m_dwWidth;
	sd.BufferDesc.Height = m_dwHeight;
	sd.BufferDesc.Format = SWAPCHAIN_FORMAT;
	sd.SampleDesc.Count = 1;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.Windowed = TRUE;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	HRESULT hr = m_graphic.DXFactory()->CreateSwapChain(m_graphic.DXDevice(), &sd,
							    m_pSwapChain.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return hr;
	}

	return CreateTargetView();
}

HRESULT DX11SwapChain::CreateTargetView()
{
	HRESULT hr =
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
					reinterpret_cast<void **>(m_pSwapBackTexture2D.Assign()));
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return hr;
	}

	hr = m_graphic.DXDevice()->CreateRenderTargetView(m_pSwapBackTexture2D, nullptr,
							  m_pRenderTargetView.Assign());
	if (FAILED(hr)) {
		CheckDXError(hr);
		assert(false);
		return hr;
	}

	return S_OK;
}
