#include <windows.h>
#include <stdio.h>
#include <winternl.h>
#include <ntstatus.h>
#include <iocodes.h>

NTSTATUS NTAPI NtDeviceIoControlFile(HANDLE FileHandle,HANDLE Event,PIO_APC_ROUTINE ApcRoutine,PVOID ApcContext,PIO_STATUS_BLOCK IoStatusBlock,ULONG IoControlCode,PVOID InputBuffer,ULONG InputBufferLength,PVOID OutputBuffer,ULONG OutputBufferLength);
#define DEVICE_NAME_NT_SYS      L"\\Device\\win32lguest"
#define RTNT_IO_STATUS_BLOCK_INITIALIZER  { STATUS_FAILED_DRIVER_ENTRY, ~(uintptr_t)42 }
#define FILE_OPEN                       0x00000001
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#define FILE_DEVICE_UNKNOWN             0x00000022
#define METHOD_NEITHER                  3
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe

int main()
{
    HANDLE hFile;
    char buf[256];
    DWORD read;
    OBJECT_ATTRIBUTES   ObjAttr;
    IO_STATUS_BLOCK     Ios   = RTNT_IO_STATUS_BLOCK_INITIALIZER;
    NTSTATUS rcNt;

    static const WCHAR  s_wszName[] = DEVICE_NAME_NT_SYS;
    UNICODE_STRING      NtName;

    NtName.Buffer        = (PWSTR)s_wszName;
    NtName.Length        = sizeof(s_wszName) - sizeof(WCHAR);
    NtName.MaximumLength = NtName.Length;

    InitializeObjectAttributes(&ObjAttr, &NtName, OBJ_CASE_INSENSITIVE, NULL /*hRootDir*/, NULL /*pSecDesc*/);

    hFile = INVALID_HANDLE_VALUE;

    rcNt = NtCreateFile(&hFile,
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
        printf("fails to open device: %x.\n", rcNt);
        return 1;
    }
    printf("my pid: %d, buf: %p.\n", GetCurrentProcessId(), buf);
    rcNt = NtDeviceIoControlFile(hFile, NULL /*hEvent*/, NULL /*ApcRoutine*/, NULL /*ApcContext*/, &Ios,
            LGUEST_IOCTL_FAST_DO_TEST,
            NULL, 0,
            buf, (ULONG)(256));
    if (!NT_SUCCESS(rcNt)) {
        printf("fails to read file: %x.\n", rcNt);
        return 1;
    }
    CloseHandle(hFile);
    printf("%s.\n", buf);
    return 0;
}
