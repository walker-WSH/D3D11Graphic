#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>

#ifdef GRAPHIC_API_EXPORTS
#define GRAPHIC_API __declspec(dllexport)
#else
#define GRAPHIC_API __declspec(dllimport)
#endif

// ��֤�����¹���
// CDX11Instance �Ĳ���������RunTaskXXX�� ������ EnterContext��LeaveContext ֮��ִ��
// ͬһ���߳��У���ͬ��CDX11Instanceʵ��  ���뱣֤��һ��ʵ��leave�� ��һ��ʵ���ſ���enter
// ͬһ��CDX11Instanceʵ�����ڲ�ͬ�߳��п��Զ��̷߳��ʣ�entercontextʱ������
class CDX11Instance {
public:
	void EnterContext();
	void LeaveContext();
	bool CheckContext();

	void RunTask1();
	void RunTask2();

private:
	std::recursive_mutex m_lockOperation;
};
