#include <wdm.h>
#include <ntddk.h>
#include <iocodes.h>

#define DEVICE_NAME_NT_SYS      L"\\Device\\win32lguest"

static PDEVICE_OBJECT g_pDevObjSys = NULL;

static NTSTATUS lguestCreateDevice(PDRIVER_OBJECT pDrvObj)
{
    /*
     * System device.
     */
    UNICODE_STRING DevName;
    NTSTATUS rcNt;

    RtlInitUnicodeString(&DevName, DEVICE_NAME_NT_SYS);
    rcNt = IoCreateDevice(pDrvObj, 0, &DevName, FILE_DEVICE_UNKNOWN, 0, FALSE, &g_pDevObjSys);
    if (NT_SUCCESS(rcNt))
    {
        g_pDevObjSys->Flags = DO_DIRECT_IO;
        /* Done. */
        DbgPrint("Device Reference: %ld.\n", g_pDevObjSys->ReferenceCount);
        return rcNt;
    }
    return rcNt;
}

static void lguestDestroyDevices(void)
{
    DbgPrint("lguestDestroyDevices: Device Reference: %ld.\n", g_pDevObjSys->ReferenceCount);
    IoDeleteDevice(g_pDevObjSys);
    g_pDevObjSys = NULL;
}

static void _stdcall lguestDrvUnload(PDRIVER_OBJECT pDrvObj)
{
    (void)pDrvObj;
    lguestDestroyDevices();
}

static NTSTATUS _stdcall lguestNtCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    PIO_STACK_LOCATION  pStack = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT        pFileObj = pStack->FileObject;
    NTSTATUS rcNt;
    (void)pFileObj;
    (void)pDevObj;

    /*
     * We are not remotely similar to a directory...
     * (But this is possible.)
     */
    if (pStack->Parameters.Create.Options & FILE_DIRECTORY_FILE)
    {
        pIrp->IoStatus.Status       = STATUS_NOT_A_DIRECTORY;
        pIrp->IoStatus.Information  = 0;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        return STATUS_NOT_A_DIRECTORY;
    }

    if (pIrp->RequestorMode == KernelMode)
        rcNt = STATUS_SUCCESS;
    else
    {
        /* User Mode. */
        rcNt = STATUS_SUCCESS;
    }

    pIrp->IoStatus.Information  = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return rcNt;
}

/**
 * Clean up file handle entry point.
 *
 * @param   pDevObj     Device object.
 * @param   pIrp        Request packet.
 */
static NTSTATUS _stdcall lguestNtCleanup(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    PIO_STACK_LOCATION  pStack   = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT        pFileObj = pStack->FileObject;
    (void)pFileObj;
    (void)pDevObj;

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}


/**
 * Close file entry point.
 *
 * @param   pDevObj     Device object.
 * @param   pIrp        Request packet.
 */
static NTSTATUS _stdcall lguestNtClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    PIO_STACK_LOCATION  pStack   = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT        pFileObj = pStack->FileObject;
    (void)pFileObj;
    (void)pDevObj;

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

static NTSTATUS _stdcall lguestNtDeviceIoControl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    PIO_STACK_LOCATION  pStack   = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT        pFileObj = pStack->FileObject;
    (void)pFileObj;

    DbgPrint("lguestNtRead: Device ReferenceCount: %ld, Current IRQL: %d\n", pDevObj->ReferenceCount, KeGetCurrentIrql());
    if (pStack->Parameters.DeviceIoControl.IoControlCode != LGUEST_IOCTL_FAST_DO_TEST) {
        DbgPrint("DeviceIoControl is not for test.\n");
        goto fail_exit;
    }

    if (pStack->Parameters.DeviceIoControl.OutputBufferLength < 13) {
        DbgPrint("DeviceIoControl.OutputBufferLength is too small for test.\n");
        goto fail_exit;
    }
    if (!pIrp->UserBuffer) {
        DbgPrint("pIrp->UserBuffer is empty.\n");
        goto fail_exit;
    }

    __try {
        ProbeForWrite(pIrp->UserBuffer, 13, 0);
        memcpy(pIrp->UserBuffer, "hello world.", 13);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        goto fail_exit;
    }
    pIrp->IoStatus.Information = 13;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

fail_exit:
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_ACCESS_VIOLATION;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_ACCESS_VIOLATION;
}

NTSTATUS _stdcall DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pRegPath)
{
    NTSTATUS rcNt;
    (void)pRegPath;
    DbgPrint("linzj's entry, sizeof(wchar_t)=%u.\n", sizeof(wchar_t));
    rcNt = lguestCreateDevice(pDrvObj);
    if (NT_SUCCESS(rcNt)) {
        DbgPrint("linzj's lguestCreateDevice fine.\n");
        pDrvObj->DriverUnload = lguestDrvUnload;
        pDrvObj->MajorFunction[IRP_MJ_CREATE]                   = lguestNtCreate;
        pDrvObj->MajorFunction[IRP_MJ_CLEANUP]                  = lguestNtCleanup;
        pDrvObj->MajorFunction[IRP_MJ_CLOSE]                    = lguestNtClose;
        pDrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL]           = lguestNtDeviceIoControl;
        return STATUS_SUCCESS;
    }
    return STATUS_NOT_IMPLEMENTED;
}
