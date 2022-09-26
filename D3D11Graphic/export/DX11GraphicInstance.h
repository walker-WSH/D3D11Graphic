#pragma once
#include <dxgi.h>
#include <Windows.h>
#include <source_location>

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
	void virtual SetBackgroundColor(float red, float green, float blue, float alpha) = 0;
	virtual void RenderEnd() = 0;
};
