#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>
#include <DX11GraphicInstance.h>

// 保证了以下规则：
// DX11GraphicInstanceImpl 的操作函数（RunTaskXXX） 必须在 EnterContext和LeaveContext 之间执行
// 同一个线程中，不同的DX11GraphicInstanceImpl实例  必须保证上一个实例leave了 下一个实例才可以enter
// 同一个DX11GraphicInstanceImpl实例，在不同线程中可以多线程访问（entercontext时有锁）
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
