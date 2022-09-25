#include "DX11GraphicInstanceImpl.h"

static thread_local std::stack<DX11GraphicInstanceImpl *> g_stackContexts;

void DX11GraphicInstanceImpl::EnterContext(const std::source_location &location)
{
	if (!g_stackContexts.empty() && g_stackContexts.top() != this) {
		assert(false && "you are in another context!");
		return;
	}

	m_lockOperation.lock();
	g_stackContexts.push(this);
}

void DX11GraphicInstanceImpl::LeaveContext(const std::source_location &location)
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

bool DX11GraphicInstanceImpl::CheckContext()
{
	bool ret = (!g_stackContexts.empty() && this == g_stackContexts.top());
	assert(ret);
	return ret;
}
