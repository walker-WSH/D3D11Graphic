#include "DX11GraphicInstanceImpl.h"

DX11GraphicInstanceImpl::DX11GraphicInstanceImpl()
{
	InitializeCriticalSection(&m_lockOperation);
}

DX11GraphicInstanceImpl::~DX11GraphicInstanceImpl()
{
	DeleteCriticalSection(&m_lockOperation);
}

void DX11GraphicInstanceImpl::RunTask1()
{
	if (CheckContext())
		return;

	// TODO
}

void DX11GraphicInstanceImpl::RunTask2()
{
	if (CheckContext())
		return;

	// TODO
}
