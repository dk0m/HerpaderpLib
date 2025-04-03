#include "utils.h"

DWORD utils::getFileSize(HANDLE fileHandle) {
	LARGE_INTEGER fileSize;
	GetFileSizeEx(fileHandle, &fileSize);
	return fileSize.QuadPart;
}

HANDLE utils::copyFile(LPCWSTR sourceFileName, LPCWSTR targetFileName) {
	HANDLE srcFile = CreateFileW(
		sourceFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
	);

	auto srcFileSize = utils::getFileSize(srcFile);
	auto contentBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, srcFileSize);

	BOOL success = ReadFile(
		srcFile,
		contentBuffer,
		srcFileSize,
		&srcFileSize,
		NULL
	);

	if (!success)
		return NULL;

	HANDLE targetFile = CreateFileW(
		targetFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		0,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		0
	);

	success = WriteFile(
		targetFile,
		contentBuffer,
		srcFileSize,
		&srcFileSize,
		NULL
	);
	
	if (!success)
		return NULL;

	return targetFile;
}

DWORD utils::getEntryPointRva(HANDLE fileHandle) {
	DWORD fileSize = utils::getFileSize(fileHandle);

	ULARGE_INTEGER mappingSize;
	mappingSize.QuadPart = fileSize;
	
	HANDLE mappingObj = CreateFileMappingW(
		fileHandle,
		NULL,
		PAGE_READONLY,
		mappingSize.HighPart,
		mappingSize.LowPart,
		NULL
	);

	LPVOID fileBuffer = MapViewOfFile(
		mappingObj,
		FILE_MAP_READ,
		0,
		0,
		mappingSize.LowPart
	);

	PIMAGE_DOS_HEADER dosHdr = (PIMAGE_DOS_HEADER)fileBuffer;
	PIMAGE_NT_HEADERS ntHdrs = (PIMAGE_NT_HEADERS)((ULONG_PTR)fileBuffer + dosHdr->e_lfanew);

	return ntHdrs->OptionalHeader.AddressOfEntryPoint;
}

PROCESS_BASIC_INFORMATION utils::getPbi(HANDLE processHandle) {
	PROCESS_BASIC_INFORMATION pbi{ 0 };

	NTSTATUS status = NtQueryInformationProcess(
		processHandle,
		ProcessBasicInformation,
		&pbi,
		sizeof(PROCESS_BASIC_INFORMATION),
		NULL
	);
	
	return pbi;
}

PEB utils::readRemotePeb(HANDLE processHandle) {
	auto pbi = utils::getPbi(processHandle);
	PEB processPeb{ 0 };

	ReadProcessMemory(
		processHandle,
		pbi.PebBaseAddress,
		&processPeb,
		sizeof(PEB),
		NULL
	);

	return processPeb;
}

PRTL_USER_PROCESS_PARAMETERS utils::initProcessParameters(std::wstring& imagePathName, std::wstring& targetCmdLine) {
	PRTL_USER_PROCESS_PARAMETERS params;

	UNICODE_STRING uimagePathName;

	RtlInitUnicodeString(&uimagePathName, imagePathName.c_str());
	
	UNICODE_STRING uDesktopInfo;
	RtlInitUnicodeString(&uDesktopInfo, L"WinSta0\\Default");

	UNICODE_STRING uCommandLine;
	RtlInitUnicodeString(&uCommandLine, targetCmdLine.c_str());

	UNICODE_STRING uWindowTitle;
	RtlInitUnicodeString(&uWindowTitle, imagePathName.c_str());
	
	NTSTATUS status = RtlCreateProcessParametersEx(
		&params,
		&uimagePathName,
		NULL,
		NULL,
		&uCommandLine,
		NtCurrentPeb()->ProcessParameters->Environment,
		&uWindowTitle,
		&uDesktopInfo,
		NULL,
		NULL,
		0
	);

	if (!NT_SUCCESS(status)) {
		return NULL;
	}
	else {
		return params;
	}

}


BOOL utils::writeRemoteProcessParameters(HANDLE processHandle, PRTL_USER_PROCESS_PARAMETERS params) {
	auto length = params->MaximumLength + params->EnvironmentSize;

	auto remoteBuffer = VirtualAllocEx(
		processHandle,
		NULL,
		length,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE
	);

	params->Environment = (PWCHAR)((ULONG_PTR)remoteBuffer + params->Length);

	auto success = WriteProcessMemory(
		processHandle,
		remoteBuffer,
		params,
		length,
		NULL
	);

	auto pbi = utils::getPbi(processHandle);
	
	success = WriteProcessMemory(
		processHandle,
		(PVOID)((ULONG_PTR)pbi.PebBaseAddress + FIELD_OFFSET(PEB, ProcessParameters)),
		&remoteBuffer,
		sizeof(remoteBuffer),
		NULL
	);

	return success;
}