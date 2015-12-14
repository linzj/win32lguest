#include <wdm.h>
#include "sehworkaround.h"

#define DEVICE_NAME_NT_SYS      L"\\Device\\win32lguest"

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
        g_pDevObjSys->Flags = DO_DIRECT_IO;
        /* Done. */
        DbgPrint("Device Reference: %ld.\n", g_pDevObjSys->ReferenceCount);
        return rcNt;
    }
    return rcNt;
}

static void lguestDestroyDevices(void)
{
    UNICODE_STRING DosName;

    DbgPrint("Device Reference: %ld.\n", g_pDevObjSys->ReferenceCount);
    IoDeleteDevice(g_pDevObjSys);
    g_pDevObjSys = NULL;
}

static void _stdcall lguestDrvUnload(PDRIVER_OBJECT pDrvObj)
{
    lguestDestroyDevices();
}

static NTSTATUS _stdcall lguestNtCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    PIO_STACK_LOCATION  pStack = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT        pFileObj = pStack->FileObject;
    (void)pFileObj;

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

    NTSTATUS rcNt;
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

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

static NTSTATUS _stdcall lguestNtRead(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    PIO_STACK_LOCATION  pStack   = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT        pFileObj = pStack->FileObject;
    (void)pFileObj;
    MDL userMDL;
    PVOID systemBuffer;

    if (!pIrp->MdlAddress) {
        DbgPrint("pIrp->MdlAddress is empty.\n");
        goto fail_exit;
    }
    if (mysetjmp()) {
        BACK_FROM_EXCEPTION;
        goto fail_exit_release;
    }
    __try1(myhandler);
    userMDL = *pIrp->MdlAddress;
    MmProbeAndLockPages(&userMDL, UserMode, IoWriteAccess);
    systemBuffer = MmGetSystemAddressForMdl(&userMDL);
    memcpy(systemBuffer, "hello world.", 13);
    MmUnlockPages(&userMDL);
    __except1;
    pIrp->IoStatus.Information = 13;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

fail_exit_release:
    myreleasejmp();
fail_exit:
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_ACCESS_VIOLATION;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_ACCESS_VIOLATION;
}

ULONG _stdcall DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pRegPath)
{
    NTSTATUS rcNt;
    DbgPrint("linzj's entry, sizeof(wchar_t)=%u.\n", sizeof(wchar_t));
    rcNt = lguestCreateDevice(pDrvObj);
    if (NT_SUCCESS(rcNt)) {
        DbgPrint("linzj's lguestCreateDevice fine.\n");
        pDrvObj->DriverUnload = lguestDrvUnload;
        pDrvObj->MajorFunction[IRP_MJ_CREATE]                   = lguestNtCreate;
        pDrvObj->MajorFunction[IRP_MJ_CLEANUP]                  = lguestNtCleanup;
        pDrvObj->MajorFunction[IRP_MJ_CLOSE]                    = lguestNtClose;
        pDrvObj->MajorFunction[IRP_MJ_READ]                     = lguestNtRead;
        // pDrvObj->MajorFunction[IRP_MJ_WRITE]                    = lguestNtWrite;
        extern FAST_IO_DISPATCH const g_lguestDrvFastIoDispatch;
        pDrvObj->FastIoDispatch = (PFAST_IO_DISPATCH)&g_lguestDrvFastIoDispatch;
        return STATUS_SUCCESS;
    }
    return STATUS_NOT_IMPLEMENTED;
}
