#pragma once

#include "ntapi.h"
#include "utils.h"

typedef struct HerpaderpInfo {
	HANDLE section;
	HANDLE process;
	HANDLE thread;
} HerpaderpInfo;

namespace herpaderp {
	HANDLE createProcessSection(HANDLE targetFile);
	HANDLE createProcessObject(HANDLE sectionHandle, HANDLE parentProcess);
	HANDLE createMainThread(HANDLE processHandle, PVOID entryPoint);

	PVOID readFile(HANDLE targetFile, DWORD& bufferSize);

	BOOL overwriteWithBuffer(HANDLE targetFile, PBYTE buffer, SIZE_T bufferSize);
	HerpaderpInfo herpaderp(LPCWSTR sourceFileName, LPCWSTR targetFileName, HANDLE parentProcess, PBYTE overwriteBuffer, SIZE_T overwriteBufferSize);

	BOOL succeeded(HerpaderpInfo& info);
}