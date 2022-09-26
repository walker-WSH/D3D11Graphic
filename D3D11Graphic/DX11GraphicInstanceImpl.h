#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <vector>
#include <Windows.h>
#include <DXDefine.h>
#include <DX11GraphicInstance.h>
#include "EnumAdapter.h"
#include "DX11Object.h"
#include "DX11ShaderBorder.h"
#include "DX11ShaderTexture.h"

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

	// IDX11GraphicInstance
	//------------------------------------------------------------------------------------------------------
	virtual bool InitializeGraphic(LUID luid);
	virtual void UnInitializeGraphic();

	virtual void ReleaseGraphicObject(void *&hdl);

	virtual texture_handle OpenTexture(HANDLE hSharedHanle);
	virtual texture_handle CreateReadTexture(uint32_t width, uint32_t height, enum DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
	virtual texture_handle CreateWriteTexture(uint32_t width, uint32_t height, enum DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
	virtual texture_handle CreateRenderTarget(uint32_t width, uint32_t height, enum DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
	virtual ST_TextureInfo GetTextureInfo(texture_handle hdl);

	virtual display_handle CreateDisplay(HWND hWnd);
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height);

	//------------------------------------------------------------------------------------------------------
	void EnterContext(const std::source_location &location);
	void LeaveContext(const std::source_location &location);
	bool CheckContext(const std::source_location &location);

	ComPtr<IDXGIFactory1> DXFactory();
	ComPtr<ID3D11Device> DXDevice();
	ComPtr<ID3D11DeviceContext> DXContext();

	void RemoveObject(DX11Object *obj);
	void PushObject(DX11Object *obj);

protected:
	void ReleaseDX();
	bool BuildDX();
	bool InitBlendState();
	bool InitSamplerState();

private:
	LUID m_adapterLuid = {0};

	CRITICAL_SECTION m_lockOperation;
	ComPtr<IDXGIFactory1> m_pDX11Factory = nullptr;
	ComPtr<ID3D11Device> m_pDX11Device = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
	ComPtr<ID3D11BlendState> m_pBlendState = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleState = nullptr;
	std::shared_ptr<DX11ShaderTexture> m_pShaderDefault = nullptr;
	std::shared_ptr<DX11ShaderTexture> m_pShaderBorder = nullptr;
	std::vector<DX11Object *> m_listObject; // Here we do not hold its lifetime.
};
