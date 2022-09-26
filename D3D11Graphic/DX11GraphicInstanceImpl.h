#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>
#include <DXDefine.h>
#include <DX11GraphicInstance.h>
#include "EnumAdapter.h"

// ��֤�����¹���
// DX11GraphicInstanceImpl �Ĳ���������RunTaskXXX�� ������ EnterContext��LeaveContext ֮��ִ��
// ͬһ���߳��У���ͬ��DX11GraphicInstanceImplʵ��  ���뱣֤��һ��ʵ��leave�� ��һ��ʵ���ſ���enter
// ͬһ��DX11GraphicInstanceImplʵ�����ڲ�ͬ�߳��п��Զ��̷߳��ʣ�entercontextʱ������
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
