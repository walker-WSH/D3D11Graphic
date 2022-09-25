#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>
#include <DX11GraphicInstance.h>

// ��֤�����¹���
// DX11GraphicInstanceImpl �Ĳ���������RunTaskXXX�� ������ EnterContext��LeaveContext ֮��ִ��
// ͬһ���߳��У���ͬ��DX11GraphicInstanceImplʵ��  ���뱣֤��һ��ʵ��leave�� ��һ��ʵ���ſ���enter
// ͬһ��DX11GraphicInstanceImplʵ�����ڲ�ͬ�߳��п��Զ��̷߳��ʣ�entercontextʱ������
class DX11GraphicInstanceImpl : public IDX11GraphicInstance {
	friend class AutoGraphicContext;

public:
	DX11GraphicInstanceImpl();
	virtual ~DX11GraphicInstanceImpl();

	virtual void RunTask1();
	virtual void RunTask2();

protected:
	void EnterContext(const std::source_location &location);
	void LeaveContext(const std::source_location &location);
	bool CheckContext();

private:
	CRITICAL_SECTION m_lockOperation;
};
