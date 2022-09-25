#pragma once
#include <Windows.h>
#include <source_location>

class IDX11GraphicInstance {
public:
	virtual ~IDX11GraphicInstance() = default;

	virtual void RunTask1() = 0;
	virtual void RunTask2() = 0;
};

class __declspec(dllexport) AutoGraphicContext {
public:
	AutoGraphicContext(IDX11GraphicInstance *graphic, const std::source_location &location);
	virtual ~AutoGraphicContext();

private:
	class impl;
	impl *self;
};
