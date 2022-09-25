#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>
#include <DX11GraphicInstance.h>

// 保证了以下规则：
// CDX11Instance 的操作函数（RunTaskXXX） 必须在 EnterContext和LeaveContext 之间执行
// 同一个线程中，不同的CDX11Instance实例  必须保证上一个实例leave了 下一个实例才可以enter
// 同一个CDX11Instance实例，在不同线程中可以多线程访问（entercontext时有锁）
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
