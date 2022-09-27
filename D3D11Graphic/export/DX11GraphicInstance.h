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

	virtual display_handle CreateDisplay(HWND hWnd) = 0;
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height) = 0;

	virtual texture_handle OpenSharedTexture(HANDLE hSharedHanle) = 0;
	virtual texture_handle OpenImageTexture(const WCHAR* fullPath) = 0;
	virtual texture_handle CreateTexture(const ST_TextureInfo &info) = 0;
	virtual ST_TextureInfo GetTextureInfo(texture_handle tex) = 0;
	virtual bool CopyTexture(texture_handle dest, texture_handle src) = 0;
	virtual bool MapTexture(texture_handle tex, bool isRead, D3D11_MAPPED_SUBRESOURCE *mapData) = 0;
	virtual void UnmapTexture(texture_handle tex) = 0;

	//--------------------------------------------------------------------------------------------
	virtual bool RenderBegin_Canvas(texture_handle hdl, ST_Color bkClr) = 0;
	virtual bool RenderBegin_Display(display_handle hdl, ST_Color bkClr) = 0;
	virtual void SetVertexBuffer(shader_handle hdl, void *buffer, size_t size) = 0;
	virtual void SetVSConstBuffer(shader_handle hdl, void *vsBuffer, size_t vsSize) = 0;
	virtual void SetPSConstBuffer(shader_handle hdl, void *psBuffer, size_t psSize) = 0;
	virtual void DrawTriangle(shader_handle hdl) = 0;
	virtual void DrawTexture(shader_handle hdl, const std::vector<texture_handle> &textures) = 0;
	virtual void RenderEnd() = 0;
};
