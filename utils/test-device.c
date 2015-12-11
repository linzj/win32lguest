#include <windows.h>
#include <stdio.h>

#define DEVICE_NAME_USR         "\\\\.\\win32lguest"

int main()
{
    HANDLE hFile;
    char buf[256];
    DWORD read;

    hFile = CreateFile(DEVICE_NAME_USR, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("fails to create file: %u.\n", GetLastError());
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
