#pragma once

#include "ntapi.h"
#include<string>

namespace utils {
	DWORD getFileSize(HANDLE fileHandle);
	HANDLE copyFile(LPCWSTR sourceFileName, LPCWSTR targetFileName);

	DWORD getEntryPointRva(HANDLE fileHandle);

	PROCESS_BASIC_INFORMATION getPbi(HANDLE processHandle);
	PEB readRemotePeb(HANDLE processHandle);

	PRTL_USER_PROCESS_PARAMETERS initProcessParameters(std::wstring& imagePathName, std::wstring& targetFileName);
	BOOL writeRemoteProcessParameters(HANDLE processHandle, PRTL_USER_PROCESS_PARAMETERS params);
}