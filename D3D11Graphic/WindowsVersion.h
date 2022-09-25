#pragma once
#include <Windows.h>

class RTCWindowVersion {
public:
	virtual ~RTCWindowVersion();

	static VS_FIXEDFILEINFO GetDllVersion(const wchar_t *dll_name);
	static bool SupportMonitorDuplicate();
	static unsigned GetWinVersion();

private:
	RTCWindowVersion();
	bool SupportMonitorDuplicateInner();

	HMODULE m_hLibrary = nullptr;
	bool m_bInited = false;
	bool m_bSupported = false;
};
