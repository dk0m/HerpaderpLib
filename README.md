
# HerpaderpLib

An Easy To Use Process Herpaderping Library Tailored For Malware & Exploits.

## Example

```cpp
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
```

## Notes
You may need to add size checks to ``herpaderp::overwriteWithBuffer``, Depending on your needs (OverwriteFileSize < TargetFileSize).


## Credits

[Process Herpaderping](https://github.com/jxy-s/herpaderping) by [jxy-s](https://github.com/jxy-s), I just basically simplified his implementation and made it operate like a library instead, So It can be included in any of your projects easily without worrying about dependencies.
