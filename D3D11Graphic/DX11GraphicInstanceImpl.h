#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <vector>
#include <Windows.h>
#include <DX11GraphicBase.h>
#include <DX11GraphicInstance.h>
#include "EnumAdapter.h"
#include "DX11GraphicBase.h"
#include "DX11ShaderBorder.h"
#include "DX11ShaderTexture.h"
#include "DX11Texture2D.h"
#include "DX11SwapChain.h"

// 保证了以下规则：
// DX11GraphicInstanceImpl 的操作函数（RunTaskXXX） 必须在 EnterContext和LeaveContext 之间执行
// 同一个线程中，不同的DX11GraphicInstanceImpl实例  必须保证上一个实例leave了 下一个实例才可以enter
// 同一个DX11GraphicInstanceImpl实例，在不同线程中可以多线程访问（entercontext时有锁）
#define CHECK_GRAPHIC_CONTEXT CheckContext(std::source_location::current())
#define CHECK_GRAPHIC_CONTEXT_EX(x) x.CheckContext(std::source_location::current())

class DX11GraphicInstanceImpl : public IDX11GraphicInstance {
	friend class AutoGraphicContext;

public:
	static HMODULE s_hDllModule;

	DX11GraphicInstanceImpl();
	virtual ~DX11GraphicInstanceImpl();

	//--------------------------------------------------- IDX11GraphicInstance ---------------------------------------------------
	virtual bool InitializeGraphic(LUID luid);
	virtual void UnInitializeGraphic();

	virtual void ReleaseGraphicObject(DX11GraphicObject *&hdl);

	virtual display_handle CreateDisplay(HWND hWnd);
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height);

	virtual texture_handle OpenTexture(HANDLE hSharedHanle);
	virtual texture_handle CreateTexture(TextureType type, const ST_TextureInfo &info);
	virtual ST_TextureInfo GetTextureInfo(texture_handle tex);
	virtual bool CopyTexture(texture_handle dest, texture_handle src);
	virtual bool MapTexture(texture_handle tex, bool isRead, D3D11_MAPPED_SUBRESOURCE *mapData);
	virtual void UnmapTexture(texture_handle tex);

	virtual bool RenderBegin_Canvas(texture_handle hdl, ST_Color bkClr);
	virtual bool RenderBegin_Display(display_handle hdl, ST_Color bkClr);
	virtual void SetVertexBuffer(shader_handle hdl, void *buffer, size_t size);
	virtual void SetVSConstBuffer(shader_handle hdl, void *vsBuffer, size_t vsSize);
	virtual void SetPSConstBuffer(shader_handle hdl, void *psBuffer, size_t psSize);
	virtual void DrawTriangleStrip(shader_handle hdl);
	virtual void DrawTexture(shader_handle hdl, const std::vector<texture_handle> &textures);
	virtual void RenderEnd();

	//------------------------------------------------------------------------------------------------------
	void EnterContext(const std::source_location &location);
	void LeaveContext(const std::source_location &location);
	bool CheckContext(const std::source_location &location);

	ComPtr<IDXGIFactory1> DXFactory();
	ComPtr<ID3D11Device> DXDevice();
	ComPtr<ID3D11DeviceContext> DXContext();

	void PushObject(DX11GraphicBase *obj);
	void RemoveObject(DX11GraphicBase *obj);

protected:
	void ReleaseAllDX();
	bool BuildAllDX();
	bool InitBlendState();
	bool InitSamplerState();

	void SetRenderTarget(ComPtr<ID3D11RenderTargetView> target, uint32_t width, uint32_t height, ST_Color bkClr);
	void UpdateShaderBuffer(ComPtr<ID3D11Buffer> buffer, void *data, size_t size);
	bool GetResource(const std::vector<texture_handle> &textures, std::vector<ID3D11ShaderResourceView *> &resources);
	void ApplyShader(DX11Shader *shader);

private:
	LUID m_adapterLuid = {0};

	CRITICAL_SECTION m_lockOperation;
	bool m_bBuildSuccessed = false;
	ComPtr<IDXGIFactory1> m_pDX11Factory = nullptr;
	ComPtr<ID3D11Device> m_pDX11Device = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
	ComPtr<ID3D11BlendState> m_pBlendState = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleState = nullptr;
	std::vector<DX11GraphicBase *> m_listObject; // Here we do not hold its lifetime.

	ID3D11RenderTargetView *m_pCurrentRenderTarget = nullptr;
	IDXGISwapChain *m_pCurrentSwapChain = nullptr;
};
