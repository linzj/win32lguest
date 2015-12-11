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
    __try {
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

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
        pDrvObj->MajorFunction[IRP_MJ_READ]                     = lguestNtRead;
        pDrvObj->MajorFunction[IRP_MJ_WRITE]                    = lguestNtWrite;
        return STATUS_SUCCESS;
    }
    return STATUS_NOT_IMPLEMENTED;
}
