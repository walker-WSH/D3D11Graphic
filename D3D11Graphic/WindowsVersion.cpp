#include "WindowsVersion.h"
#include <DX11GraphicBase.h>
#include <memory>

using get_file_version_info_size_wt = DWORD(WINAPI *)(LPCWSTR md, LPDWORD unused);
using get_file_version_info_wt = BOOL(WINAPI *)(LPCWSTR md, DWORD unused, DWORD len, LPVOID data);
using ver_query_value_wt = BOOL(WINAPI *)(LPVOID data, LPCWSTR subblock, LPVOID *buf,
					  PUINT sizeout);

bool RTCWindowVersion::SupportMonitorDuplicate()
{
	static RTCWindowVersion s_Ins;
	return s_Ins.SupportMonitorDuplicateInner();
}

bool InitFun(get_file_version_info_size_wt &pFunc1, get_file_version_info_wt &pFunc2,
	     ver_query_value_wt &pFunc3);
VS_FIXEDFILEINFO RTCWindowVersion::GetDllVersion(const wchar_t *pDllName)
{
	LPVOID data = nullptr;
	VS_FIXEDFILEINFO ret = {};

	do {
		get_file_version_info_size_wt pGetFileVersionInfoSize;
		get_file_version_info_wt pGetFileVersionInfo;
		ver_query_value_wt pVerQueryValue;

		if (!InitFun(pGetFileVersionInfoSize, pGetFileVersionInfo, pVerQueryValue))
			break;

		DWORD size = pGetFileVersionInfoSize(pDllName, nullptr);
		if (!size)
			break;

		data = HeapAlloc(GetProcessHeap(), 0, size);
		if (!pGetFileVersionInfo(L"kernel32", 0, size, data))
			break;

		uint32_t len = 0;
		VS_FIXEDFILEINFO *info = nullptr;
		pVerQueryValue(data, L"\\", (LPVOID *)&info, &len);

		ret = *info;
	} while (false);

	if (data) {
		HeapFree(GetProcessHeap(), 0, data);
	}

	return ret;
}

unsigned RTCWindowVersion::GetWinVersion()
{
	VS_FIXEDFILEINFO info = RTCWindowVersion::GetDllVersion(L"kernel32");

	auto major = (int)HIWORD(info.dwFileVersionMS);
	auto minor = (int)LOWORD(info.dwFileVersionMS);

	return (major << 8) | minor;
}

RTCWindowVersion::RTCWindowVersion()
{
	m_hLibrary = LoadLibraryW(L"version");
}

RTCWindowVersion::~RTCWindowVersion()
{
	if (m_hLibrary)
		FreeLibrary(m_hLibrary);
}

bool InitFun(get_file_version_info_size_wt &pFunc1, get_file_version_info_wt &pFunc2,
	     ver_query_value_wt &pFunc3)
{
	HMODULE ver = GetModuleHandleW(L"version");
	if (!ver)
		return false;

	pFunc1 = (get_file_version_info_size_wt)GetProcAddress(ver, "GetFileVersionInfoSizeW");
	pFunc2 = (get_file_version_info_wt)GetProcAddress(ver, "GetFileVersionInfoW");
	pFunc3 = (ver_query_value_wt)GetProcAddress(ver, "VerQueryValueW");

	if (!pFunc1 || !pFunc2 || !pFunc3)
		return false;

	return true;
}

bool RTCWindowVersion::SupportMonitorDuplicateInner()
{
	if (m_bInited)
		return m_bSupported;

	m_bInited = true;
	m_bSupported = false;

	VS_FIXEDFILEINFO info = GetDllVersion(L"kernel32");
	auto major = (int)HIWORD(info.dwFileVersionMS);
	auto minor = (int)LOWORD(info.dwFileVersionMS);

	if (major > 6 || (major == 6 && minor >= 2)) {
		m_bSupported = true;
	}

	return m_bSupported;
}
