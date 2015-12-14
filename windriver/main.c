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
        return rcNt;
    }
    return rcNt;
}

static void lguestDestroyDevices(void)
{
    UNICODE_STRING DosName;

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

#if 0
static NTSTATUS _stdcall lguestNtRead(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    PIO_STACK_LOCATION  pStack   = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT        pFileObj = pStack->FileObject;
    (void)pFileObj;
    MDL userMDL;
    PVOID systemBuffer;

    DbgPrint("irql = %d, pid = %u.\n", KeGetCurrentIrql(), PsGetCurrentProcessId());
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
    DbgPrint("calling MmProbeAndLockPages.\n");
    MmProbeAndLockPages(&userMDL, UserMode, IoWriteAccess);
    DbgPrint("calling MmGetSystemAddressForMdl.\n");
    systemBuffer = MmGetSystemAddressForMdl(&userMDL);
    memcpy(systemBuffer, "hello world.", 13);
    DbgPrint("calling MmUnlockPages.\n");
    MmUnlockPages(&userMDL);
    __except1;
    DbgPrint("out exception handling.\n");
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
#endif

static BOOLEAN
NTAPI lguestDrvFastRead(
  IN FILE_OBJECT *FileObject,
  IN PLARGE_INTEGER FileOffset,
  IN ULONG Length,
  IN BOOLEAN Wait,
  IN ULONG LockKey,
  OUT PVOID Buffer,
  OUT PIO_STATUS_BLOCK IoStatus,
  IN DEVICE_OBJECT *DeviceObject)
{
    DbgPrint("irql = %d, pid = %u.\n", KeGetCurrentIrql(), PsGetCurrentProcessId());
    if (Length < 13) {
        goto fail_exit;
    }
    if (mysetjmp()) {
        BACK_FROM_EXCEPTION;
        goto fail_exit_release;
    }
    __try1(myhandler);
    memcpy(Buffer, "hello world.", 13);
    __except1;
    IoStatus->Information = 13;
    IoStatus->Status = STATUS_SUCCESS;
    return TRUE;

fail_exit_release:
    myreleasejmp();
fail_exit:
    IoStatus->Information = 0;
    IoStatus->Status = STATUS_ACCESS_VIOLATION;

    return FALSE;
}

static FAST_IO_DISPATCH const g_lguestDrvFastIoDispatch =
{
    /* .SizeOfFastIoDispatch            = */ sizeof(g_lguestDrvFastIoDispatch),
    /* .FastIoCheckIfPossible           = */ NULL,
    /* .FastIoRead                      = */ lguestDrvFastRead,
    /* .FastIoWrite                     = */ NULL,
    /* .FastIoQueryBasicInfo            = */ NULL,
    /* .FastIoQueryStandardInfo         = */ NULL,
    /* .FastIoLock                      = */ NULL,
    /* .FastIoUnlockSingle              = */ NULL,
    /* .FastIoUnlockAll                 = */ NULL,
    /* .FastIoUnlockAllByKey            = */ NULL,
    /* .FastIoDeviceControl             = */ NULL,
    /* .AcquireFileForNtCreateSection   = */ NULL,
    /* .ReleaseFileForNtCreateSection   = */ NULL,
    /* .FastIoDetachDevice              = */ NULL,
    /* .FastIoQueryNetworkOpenInfo      = */ NULL,
    /* .AcquireForModWrite              = */ NULL,
    /* .MdlRead                         = */ NULL,
    /* .MdlReadComplete                 = */ NULL,
    /* .PrepareMdlWrite                 = */ NULL,
    /* .MdlWriteComplete                = */ NULL,
    /* .FastIoReadCompressed            = */ NULL,
    /* .FastIoWriteCompressed           = */ NULL,
    /* .MdlReadCompleteCompressed       = */ NULL,
    /* .MdlWriteCompleteCompressed      = */ NULL,
    /* .FastIoQueryOpen                 = */ NULL,
    /* .ReleaseForModWrite              = */ NULL,
    /* .AcquireForCcFlush               = */ NULL,
    /* .ReleaseForCcFlush               = */ NULL,
};

ULONG _stdcall DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pRegPath)
{
    NTSTATUS rcNt;
    DbgPrint("linzj's entry, sizeof(wchar_t)=%u.\n", sizeof(wchar_t));
    rcNt = lguestCreateDevice(pDrvObj);
    if (NT_SUCCESS(rcNt)) {
        DbgPrint("linzj's lguestCreateDevice fine");
        pDrvObj->DriverUnload = lguestDrvUnload;
        pDrvObj->MajorFunction[IRP_MJ_CREATE]                   = lguestNtCreate;
        pDrvObj->MajorFunction[IRP_MJ_CLEANUP]                  = lguestNtCleanup;
        pDrvObj->MajorFunction[IRP_MJ_CLOSE]                    = lguestNtClose;
        // pDrvObj->MajorFunction[IRP_MJ_READ]                     = lguestNtRead;
        // pDrvObj->MajorFunction[IRP_MJ_WRITE]                    = lguestNtWrite;
        pDrvObj->FastIoDispatch = (PFAST_IO_DISPATCH)&g_lguestDrvFastIoDispatch;
        return STATUS_SUCCESS;
    }
    return STATUS_NOT_IMPLEMENTED;
}
