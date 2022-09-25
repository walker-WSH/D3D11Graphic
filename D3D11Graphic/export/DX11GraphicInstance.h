#pragma once
#include <Windows.h>
#include <source_location>

class IDX11GraphicInstance {
public:
	virtual ~IDX11GraphicInstance() = default;

	virtual void EnterContext(const std::source_location &location = std::source_location::current()) = 0;
	virtual void LeaveContext(const std::source_location &location = std::source_location::current()) = 0;

	virtual void RunTask1() = 0;
	virtual void RunTask2() = 0;
};
