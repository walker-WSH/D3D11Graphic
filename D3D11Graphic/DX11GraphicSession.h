#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <vector>
#include <Windows.h>
#include <DX11GraphicBase.h>
#include <IDX11GraphicSession.h>
#include "EnumAdapter.h"
#include "DX11GraphicBase.h"
#include "DX11Shader.h"
#include "DX11Texture2D.h"
#include "DX11SwapChain.h"

// 保证了以下规则：
// DX11GraphicSession 的操作函数（RunTaskXXX） 必须在 EnterContext和LeaveContext 之间执行
// 同一个线程中，不同的DX11GraphicSession实例  必须保证上一个实例leave了 下一个实例才可以enter
// 同一个DX11GraphicSession实例，在不同线程中可以多线程访问（entercontext时有锁）
#define CHECK_GRAPHIC_CONTEXT CheckContext(std::source_location::current())
#define CHECK_GRAPHIC_CONTEXT_EX(x) x.CheckContext(std::source_location::current())

class DX11GraphicSession : public IDX11GraphicSession {
	friend class AutoGraphicContext;

public:
	static HMODULE s_hDllModule;

	DX11GraphicSession();
	virtual ~DX11GraphicSession();

	//--------------------------------------------------- IDX11GraphicSession ---------------------------------------------------
	virtual bool InitializeGraphic(const ST_GraphicCardInfo *graphic = nullptr);
	virtual void UnInitializeGraphic();

	virtual void RegisterCallback(std::weak_ptr<DX11GraphicCallback> cb);
	virtual void UnRegisterCallback(DX11GraphicCallback *cb);

	virtual bool IsGraphicBuilt();
	virtual bool ReBuildGraphic();

	virtual void DestroyGraphicObject(DX11GraphicObject *&hdl);
	virtual void DestroyAllGraphicObject();

	virtual display_handle CreateDisplay(HWND hWnd);
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height);

	virtual shader_handle CreateShader(const ST_ShaderInfo &info);

	virtual texture_handle OpenSharedTexture(HANDLE hSharedHanle);
	virtual texture_handle OpenImageTexture(const WCHAR *fullPath);
	virtual texture_handle CreateTexture(const ST_TextureInfo &info);
	virtual ST_TextureInfo GetTextureInfo(texture_handle tex);
	virtual HANDLE GetSharedHandle(texture_handle tex);
	virtual bool CopyTexture(texture_handle dest, texture_handle src);
	virtual bool MapTexture(texture_handle tex, MapTextureType type,
				D3D11_MAPPED_SUBRESOURCE *mapData);
	virtual void UnmapTexture(texture_handle tex);

	virtual bool BeginRenderCanvas(texture_handle hdl);
	virtual bool BeginRenderWindow(display_handle hdl);
	virtual void ClearBackground(const ST_Color *bkClr);
	virtual void SetBlendState(BlendStateType type);
	virtual void SetVertexBuffer(shader_handle hdl, const void *buffer, size_t size);
	virtual void SetVSConstBuffer(shader_handle hdl, const void *vsBuffer, size_t vsSize);
	virtual void SetPSConstBuffer(shader_handle hdl, const void *psBuffer, size_t psSize);
	virtual void DrawTopplogy(shader_handle hdl, D3D11_PRIMITIVE_TOPOLOGY type);
	virtual void DrawTexture(shader_handle hdl, FilterType flt,
				 const std::vector<texture_handle> &textures);
	virtual void EndRender();

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
	void HandleDXHResult(HRESULT hr,
			     std::source_location location = std::source_location::current());

	void SetRenderTarget(ComPtr<ID3D11RenderTargetView> target, uint32_t width,
			     uint32_t height);
	void UpdateShaderBuffer(ComPtr<ID3D11Buffer> buffer, const void *data, size_t size);
	bool GetResource(const std::vector<texture_handle> &textures,
			 std::vector<ID3D11ShaderResourceView *> &resources);
	void ApplyShader(DX11Shader *shader);

	bool IsGraphicObjectAlive(DX11GraphicObject *obj);

private:
	ST_GraphicCardInfo m_destGraphic;

	CRITICAL_SECTION m_lockOperation;
	bool m_bBuildSuccessed = false;
	ComPtr<IDXGIAdapter1> m_pAdapter = nullptr;
	ComPtr<IDXGIFactory1> m_pDX11Factory = nullptr;
	ComPtr<ID3D11Device> m_pDX11Device = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
	ComPtr<ID3D11BlendState> m_pBlendState = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleStateAnisotropic = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleStateLinear = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleStatePoint = nullptr;
	std::vector<DX11GraphicBase *> m_listObject; // Here we do not hold its lifetime.
	std::vector<std::weak_ptr<DX11GraphicCallback>> m_pGraphicCallbacks;

	ID3D11RenderTargetView *m_pCurrentRenderTarget = nullptr;
	IDXGISwapChain *m_pCurrentSwapChain = nullptr;
};
