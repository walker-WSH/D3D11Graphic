#pragma once
#include <dxgi.h>
#include <Windows.h>
#include <DX11VideoFrame.hpp>
#include <DX11GraphicDefine.h>

class IDX11GraphicInstance {
public:
	virtual ~IDX11GraphicInstance() = default;

	virtual bool InitializeGraphic(LUID luid) = 0;
	virtual void UnInitializeGraphic() = 0;

	virtual void ReleaseGraphicObject(DX11GraphicObject *&hdl) = 0;

	virtual texture_handle OpenTexture(HANDLE hSharedHanle) = 0;
	virtual texture_handle CreateTexture2D(TextureType type, uint32_t width, uint32_t height, enum DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM) = 0;
	virtual ST_TextureInfo GetTextureInfo(texture_handle hdl) = 0;

	virtual display_handle CreateDisplay(HWND hWnd) = 0;
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height) = 0;

	virtual bool RenderBegin_Canvas(texture_handle hdl, ST_Color bkClr) = 0;
	virtual bool RenderBegin_Display(display_handle hdl, ST_Color bkClr) = 0;
	virtual void SetVertexBuffer(shader_handle hdl, void *buffer, size_t size) = 0;
	virtual void SetVSConstBuffer(shader_handle hdl, void *vsBuffer, size_t vsSize) = 0;
	virtual void SetPSConstBuffer(shader_handle hdl, void *psBuffer, size_t psSize) = 0;
	virtual void DrawTriangleStrip(shader_handle hdl) = 0;
	virtual void DrawTexture(shader_handle hdl, const std::vector<texture_handle> &textures) = 0;
	virtual void RenderEnd() = 0;
};
