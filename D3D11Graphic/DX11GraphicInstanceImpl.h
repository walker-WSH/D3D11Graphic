#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>
#include <DXDefine.h>
#include <DX11GraphicInstance.h>
#include "EnumAdapter.h"

// 保证了以下规则：
// DX11GraphicInstanceImpl 的操作函数（RunTaskXXX） 必须在 EnterContext和LeaveContext 之间执行
// 同一个线程中，不同的DX11GraphicInstanceImpl实例  必须保证上一个实例leave了 下一个实例才可以enter
// 同一个DX11GraphicInstanceImpl实例，在不同线程中可以多线程访问（entercontext时有锁）
class DX11GraphicInstanceImpl : public IDX11GraphicInstance {
	friend class AutoGraphicContext;

public:
	DX11GraphicInstanceImpl();
	virtual ~DX11GraphicInstanceImpl();

	virtual bool InitializeGraphic(LUID luid);
	virtual void UnInitializeGraphic();

	virtual void RunTask1();

protected:
	void EnterContext(const std::source_location &location);
	void LeaveContext(const std::source_location &location);
	bool CheckContext(const std::source_location &location);

	bool BuildDX();
	bool InitBlendState();
	bool InitSamplerState();

	void ReleaseDX();

private:
	CRITICAL_SECTION m_lockOperation;

	LUID m_adapterLuid = {0};

	ComPtr<ID3D11Device> m_pDX11Device = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
	ComPtr<ID3D11BlendState> m_pBlendState = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleState = nullptr;
};
