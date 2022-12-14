#pragma once
#include <dxgi.h>
#include <Windows.h>
#include <memory>
#include <IDX11GraphicDefine.h>

#define COMBINE2(a, b) a##b
#define COMBINE1(a, b) COMBINE2(a, b)
#define AUTO_GRAPHIC_CONTEXT(graphic) \
	AutoGraphicContext COMBINE1(autoContext, __LINE__)(graphic, std::source_location::current())

class IDX11GraphicSession {
public:
	virtual ~IDX11GraphicSession() = default;

	// graphic : Select graphic automatically if it's null. NVIDIA > AMD > INTEL > BAISC > ANY
	virtual bool InitializeGraphic(const ST_GraphicCardInfo *graphic = nullptr) = 0;
	virtual void UnInitializeGraphic() = 0;

	virtual void RegisterCallback(std::weak_ptr<DX11GraphicCallback> cb) = 0;
	virtual void UnRegisterCallback(DX11GraphicCallback *cb) = 0;

	virtual bool IsGraphicBuilt() = 0;
	virtual bool ReBuildGraphic() = 0;

	virtual void DestroyGraphicObject(DX11GraphicObject *&hdl) = 0;
	virtual void DestroyAllGraphicObject() = 0;

	virtual display_handle CreateDisplay(HWND hWnd) = 0;
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height) = 0;
	virtual bool CopyDisplay(texture_handle dest, display_handle src) = 0;
	virtual ST_DisplayInfo GetDisplayInfo(display_handle hdl) = 0;

	virtual shader_handle CreateShader(const ST_ShaderInfo &info) = 0;
	virtual void SetVertexBuffer(shader_handle hdl, const void *buffer, size_t size) = 0;
	virtual void SetVSConstBuffer(shader_handle hdl, const void *vsBuffer, size_t vsSize) = 0;
	virtual void SetPSConstBuffer(shader_handle hdl, const void *psBuffer, size_t psSize) = 0;

	virtual texture_handle OpenSharedTexture(HANDLE hSharedHanle) = 0;
	virtual texture_handle OpenImageTexture(const WCHAR *fullPath) = 0;
	virtual texture_handle CreateTexture(const ST_TextureInfo &info) = 0;
	virtual ST_TextureInfo GetTextureInfo(texture_handle tex) = 0;
	virtual HANDLE GetSharedHandle(texture_handle tex) = 0;
	virtual bool CopyTexture(texture_handle dest, texture_handle src) = 0;
	virtual bool MapTexture(texture_handle tex, MapTextureType type,
				D3D11_MAPPED_SUBRESOURCE *) = 0;
	virtual void UnmapTexture(texture_handle tex) = 0;

	//----------------------------------------------------------------------
	virtual bool BeginRenderCanvas(texture_handle hdl) = 0;
	virtual bool BeginRenderWindow(display_handle hdl) = 0;
	virtual void ClearBackground(const ST_Color *bkClr) = 0;
	virtual void SetBlendState(BlendStateType type) = 0;
	virtual void DrawTopplogy(shader_handle hdl, D3D11_PRIMITIVE_TOPOLOGY type) = 0;
	virtual void DrawTexture(shader_handle hdl, FilterType flt,
				 const std::vector<texture_handle> &) = 0;
	virtual void EndRender() = 0;
};
