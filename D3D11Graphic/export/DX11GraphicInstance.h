#pragma once
#include <stack>
#include <mutex>
#include <assert.h>
#include <Windows.h>
#include <source_location>

class DX11Graphic {
public:
	virtual ~DX11Graphic() = default;

	virtual void EnterContext(const std::source_location &location = std::source_location::current()) = 0;
	virtual void LeaveContext(const std::source_location &location = std::source_location::current()) = 0;

	virtual void RunTask1() = 0;
	virtual void RunTask2() = 0;
};
