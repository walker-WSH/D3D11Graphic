#pragma once
#include <dxgi.h>
#include <Windows.h>
#include <source_location>
#include <DX11VideoFrame.hpp>

using texture_handle = void *;
using display_handle = void *;

struct ST_TextureInfo {
	uint32_t width = 0;
	uint32_t height = 0;
	enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
};

class IDX11GraphicInstance;
class __declspec(dllexport) AutoGraphicContext {
public:
	AutoGraphicContext(IDX11GraphicInstance *graphic, const std::source_location &location);
	virtual ~AutoGraphicContext();

private:
	class impl;
	impl *self;
};

//------------------------------------------------------------------------------------------------
class IDX11GraphicInstance {
public:
	virtual ~IDX11GraphicInstance() = default;

	virtual bool InitializeGraphic(LUID luid) = 0;
	virtual void UnInitializeGraphic() = 0;

	virtual void ReleaseGraphicObject(void *&hdl) = 0;

	virtual texture_handle OpenTexture(HANDLE hSharedHanle) = 0;
	virtual texture_handle CreateReadTexture(uint32_t width, uint32_t height, enum DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM) = 0;
	virtual texture_handle CreateWriteTexture(uint32_t width, uint32_t height, enum DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM) = 0;
	virtual texture_handle CreateRenderCanvas(uint32_t width, uint32_t height, enum DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM) = 0;
	virtual ST_TextureInfo GetTextureInfo(texture_handle hdl) = 0;

	virtual display_handle CreateDisplay(HWND hWnd) = 0;
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height) = 0;

	virtual bool RenderBegin_Canvas(texture_handle hdl) = 0;
	virtual bool RenderBegin_Display(display_handle hdl) = 0;
	virtual void SetBackgroundColor(float red, float green, float blue, float alpha) = 0;

	virtual void SetVideoFrame(VIDEO_FRAME frame) {}
	virtual void SetVertexBuffer(void *buffer, size_t size) {}
	virtual void SetConstBuffer(void *vsBuffer, size_t vsSize, void *psBuffer, size_t psSize) {} // such as wvp matrix

	virtual void Draw()
	{
		//buffer[0] = m_pUsedShader->m_pVertexBuffer;
		//TestRenderModuleEngine::Instance()->m_pDeviceContext->IASetVertexBuffers(0, 1, buffer, &stride, &offset);
		//TestRenderModuleEngine::Instance()->m_pDeviceContext->IASetInputLayout(m_pUsedShader->m_pInputLayout);

		//TestRenderModuleEngine::Instance()->m_pDeviceContext->VSSetShader(m_pUsedShader->m_pVertexShader, NULL, 0);
		//if (m_pUsedShader->m_pVSBuffer.Get()) {
		//	buffer[0] = m_pUsedShader->m_pVSBuffer;
		//	TestRenderModuleEngine::Instance()->m_pDeviceContext->VSSetConstantBuffers(0, 1, buffer);
		//}

		//TestRenderModuleEngine::Instance()->m_pDeviceContext->PSSetShader(m_pUsedShader->m_pPixelShader, NULL, 0);
		//if (m_pUsedShader->m_pPSBuffer.Get()) {
		//	buffer[0] = m_pUsedShader->m_pPSBuffer;
		//	TestRenderModuleEngine::Instance()->m_pDeviceContext->PSSetConstantBuffers(0, 1, buffer);
		//}

		//TestRenderModuleEngine::Instance()->m_pDeviceContext->PSSetSamplers(0, 1, &sampleState);
		//TestRenderModuleEngine::Instance()->m_pDeviceContext->PSSetShaderResources(0, 1, views);
		//TestRenderModuleEngine::Instance()->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		//TestRenderModuleEngine::Instance()->m_pDeviceContext->Draw(TEXTURE_VERTEX_COUNT, 0);

		//m_pSwapChain->m_pSwapChain->Present(0, 0);
	}

	virtual void RenderEnd() = 0;
};
