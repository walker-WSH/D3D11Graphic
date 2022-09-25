#include "DX11GraphicInstanceImpl.h"

static thread_local std::stack<DX11GraphicInstanceImpl *> g_stackContexts;

class AutoGraphicContext::impl {
public:
	impl(IDX11GraphicInstance *graphic, const std::source_location &location) : m_Location(location)
	{
		m_pGraphic = dynamic_cast<DX11GraphicInstanceImpl *>(graphic);
		m_pGraphic->EnterContext(m_Location);
	}

	virtual ~impl() { m_pGraphic->LeaveContext(m_Location); }

private:
	std::source_location m_Location;
	DX11GraphicInstanceImpl *m_pGraphic = nullptr;
};

AutoGraphicContext::AutoGraphicContext(IDX11GraphicInstance *graphic, const std::source_location &location)
{
	self = new impl(graphic, location);
}

AutoGraphicContext::~AutoGraphicContext()
{
	delete self;
}

//------------------------------------------------------------------------------------
void DX11GraphicInstanceImpl::EnterContext(const std::source_location &location)
{
	if (!g_stackContexts.empty() && g_stackContexts.top() != this) {
		assert(false && "you are in another context!");
		return;
	}

	EnterCriticalSection(&m_lockOperation);
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
	LeaveCriticalSection(&m_lockOperation);
}

bool DX11GraphicInstanceImpl::CheckContext()
{
	bool ret = (!g_stackContexts.empty() && this == g_stackContexts.top());
	assert(ret);
	return ret;
}
