#include <wdm.h>
#include "sehworkaround.h"

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
    DbgPrint("lguestDrvFastRead: irql = %d, pid = %u, issystemprocess.\n", KeGetCurrentIrql(), PsGetCurrentProcessId(), PsIsSystemThread(PsGetCurrentThread()));
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

FAST_IO_DISPATCH const g_lguestDrvFastIoDispatch =
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
