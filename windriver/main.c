#include <wdm.h>

#define DEVICE_NAME_NT_SYS      L"\\Device\\win32lguest"
/** Win Symlink name for system access. */
#define DEVICE_NAME_DOS_SYS     L"\\DosDevices\\win32lguest"

static PDEVICE_OBJECT g_pDevObjSys = NULL;

static NTSTATUS lguestCreateDevice(PDRIVER_OBJECT pDrvObj)
{
    /*
     * System device.
     */
    UNICODE_STRING DevName;
    RtlInitUnicodeString(&DevName, DEVICE_NAME_NT_SYS);
    NTSTATUS rcNt = IoCreateDevice(pDrvObj, 0, &DevName, FILE_DEVICE_UNKNOWN, 0, FALSE, &g_pDevObjSys);
    if (NT_SUCCESS(rcNt))
    {
        UNICODE_STRING DosName;
        RtlInitUnicodeString(&DosName, DEVICE_NAME_DOS_SYS);
        rcNt = IoCreateSymbolicLink(&DosName, &DevName);
        if (NT_SUCCESS(rcNt))
        {
            /* Done. */
            return rcNt;
        }
        IoDeleteDevice(g_pDevObjSys);
        g_pDevObjSys = NULL;
    }
    return rcNt;
}

static void lguestDestroyDevices(void)
{
    UNICODE_STRING DosName;
    RtlInitUnicodeString(&DosName, DEVICE_NAME_DOS_SYS);
    NTSTATUS rcNt = IoDeleteSymbolicLink(&DosName);

    IoDeleteDevice(g_pDevObjSys);
    g_pDevObjSys = NULL;
}

static void _stdcall lguestDrvUnload(PDRIVER_OBJECT pDrvObj)
{
    lguestDestroyDevices();
}

ULONG _stdcall DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pRegPath)
{
    NTSTATUS rcNt;
    DbgPrint("linzj's entry, sizeof(wchar_t)=%u.\n", sizeof(wchar_t));
    rcNt = lguestCreateDevice(pDrvObj);
    if (NT_SUCCESS(rcNt)) {
        DbgPrint("linzj's lguestCreateDevice fine");
        pDrvObj->DriverUnload = lguestDrvUnload;
        return STATUS_SUCCESS;
    }
    return STATUS_NOT_IMPLEMENTED;
}
