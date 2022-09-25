#include "DX11Instance.h"

static thread_local std::stack<CDX11Instance *> g_stackContexts;

void CDX11Instance::EnterContext(const std::source_location &location)
{
	if (!g_stackContexts.empty() && g_stackContexts.top() != this) {
		assert(false && "you are in another context!");
		return;
	}

	m_lockOperation.lock();
	g_stackContexts.push(this);
}

void CDX11Instance::LeaveContext(const std::source_location &location)
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
