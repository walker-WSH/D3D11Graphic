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

// 保证了以下规则：
// CDX11Instance 的操作函数（RunTaskXXX） 必须在 EnterContext和LeaveContext 之间执行
// 同一个线程中，不同的CDX11Instance实例  必须保证上一个实例leave了 下一个实例才可以enter
// 同一个CDX11Instance实例，在不同线程中可以多线程访问（entercontext时有锁）
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
