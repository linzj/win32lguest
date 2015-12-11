#include <wdm.h>

ULONG _stdcall DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pRegPath)
{
    DbgPrint("linzj's entry.\n");
    return STATUS_NOT_IMPLEMENTED;
}
