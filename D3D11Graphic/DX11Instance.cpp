#include "DX11Instance.h"

static thread_local std::stack<CDX11Instance *> g_stackContexts;

void CDX11Instance::EnterContext()
{
	if (!g_stackContexts.empty() && g_stackContexts.top() != this) {
		assert(false && "you are in another context!");
		return;
	}

	m_lockOperation.lock();
	g_stackContexts.push(this);
}

void CDX11Instance::LeaveContext()
{
	if (g_stackContexts.empty()) {
		assert(false && "you are not in any context!");
		return;
	} else {
		if (g_stackContexts.top() != this) {
			assert(false && "you are in another context!");
			return;
		}
	}

	g_stackContexts.pop();
	m_lockOperation.unlock();
}

bool CDX11Instance::CheckContext()
{
	bool ret = (!g_stackContexts.empty() && this == g_stackContexts.top());
	assert(ret);
	return ret;
}

void CDX11Instance::RunTask1()
{
	if (CheckContext())
		return;

	// TODO
}

void CDX11Instance::RunTask2()
{
	if (CheckContext())
		return;

	// TODO
}
