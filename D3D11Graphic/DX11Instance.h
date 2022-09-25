#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>
#include <DX11GraphicInstance.h>

// ��֤�����¹���
// CDX11Instance �Ĳ���������RunTaskXXX�� ������ EnterContext��LeaveContext ֮��ִ��
// ͬһ���߳��У���ͬ��CDX11Instanceʵ��  ���뱣֤��һ��ʵ��leave�� ��һ��ʵ���ſ���enter
// ͬһ��CDX11Instanceʵ�����ڲ�ͬ�߳��п��Զ��̷߳��ʣ�entercontextʱ������
class CDX11Instance : public DX11Graphic {
public:
	virtual void EnterContext(const std::source_location &location = std::source_location::current());
	virtual void LeaveContext(const std::source_location &location = std::source_location::current());

	virtual void RunTask1();
	virtual void RunTask2();

protected:
	bool CheckContext();

private:
	std::recursive_mutex m_lockOperation;
};
