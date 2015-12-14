#include <windows.h>
#include <stdio.h>
#include <winternl.h>
#include <ntstatus.h>

#define DEVICE_NAME_NT_SYS      L"\\Device\\win32lguest"
#define RTNT_IO_STATUS_BLOCK_INITIALIZER  { STATUS_FAILED_DRIVER_ENTRY, ~(uintptr_t)42 }

int main()
{
    HANDLE hFile;
    char buf[256];
    DWORD read;

    static const WCHAR  s_wszName[] = DEVICE_NAME_NT_SYS;
    UNICODE_STRING      NtName;
    NtName.Buffer        = (PWSTR)s_wszName;
    NtName.Length        = sizeof(s_wszName) - sizeof(WCHAR);
    NtName.MaximumLength = NtName.Length;

    OBJECT_ATTRIBUTES   ObjAttr;
    InitializeObjectAttributes(&ObjAttr, &NtName, OBJ_CASE_INSENSITIVE, NULL /*hRootDir*/, NULL /*pSecDesc*/);

    hFile = INVALID_HANDLE_VALUE;

    IO_STATUS_BLOCK     Ios   = RTNT_IO_STATUS_BLOCK_INITIALIZER;
    NTSTATUS rcNt = NtCreateFile(&hFile,
            GENERIC_READ | GENERIC_WRITE, /* No SYNCHRONIZE. */
            &ObjAttr,
            &Ios,
            NULL /* Allocation Size*/,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN,
            FILE_NON_DIRECTORY_FILE, /* No FILE_SYNCHRONOUS_IO_NONALERT! */
            NULL /*EaBuffer*/,
            0 /*EaLength*/);
    if (NT_SUCCESS(rcNt))
        rcNt = Ios.Status;
    if (NT_SUCCESS(rcNt))
    {
        /*
         * We're good.
         */
    }
    else {
        return 1;
    }
    if (!ReadFile(hFile, buf, 256, &read, NULL)) {
        printf("fails to read file: %u.\n", GetLastError());
        return 1;
    }
    CloseHandle(hFile);
    printf("%s.\n", buf);
    return 0;
}