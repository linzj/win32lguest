/*
 * This source code is a part of coLinux source package.
 *
 * Dan Aloni <da-x@colinux.org>, 2003 (c)
 *
 * The code is licensed under the GPL. See the COPYING file at
 * the root directory.
 *
 */

#include "ddk.h"

#include <colinux/os/kernel/misc.h>
typedef struct _SYSTEM_BASIC_INFORMATION {
    unsigned char Reserved1[4];
    ULONG MaximumIncrement;
    ULONG PhysicalPageSize;
    ULONG NumberOfPhysicalPages;
    ULONG LowestPhysicalPage;
    ULONG HighestPhysicalPage;
    ULONG AllocationGranularity;
    ULONG_PTR LowestUserAddress;
    ULONG_PTR HighestUserAddress;
    ULONG_PTR ActiveProcessors;
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION,*PSYSTEM_BASIC_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemProcessorInformation = 1,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemHandleInformation = 16,
    SystemPagefileInformation = 18,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;

NTSTATUS NTAPI NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);

unsigned long co_os_virt_to_phys(void *addr)
{
	PHYSICAL_ADDRESS pa;

	pa = MmGetPhysicalAddress((PVOID)addr);

	return pa.QuadPart;
}

co_rc_t co_os_physical_memory_pages(unsigned long *pages)
{
	SYSTEM_BASIC_INFORMATION sbi;
	NTSTATUS status;
	co_rc_t rc = CO_RC(OK);

	status = NtQuerySystemInformation(SystemBasicInformation, &sbi, sizeof(sbi), NULL);
	if (status != STATUS_SUCCESS)
		return CO_RC(ERROR);

	*pages = sbi.NumberOfPhysicalPages;

	/*
	 * Round to 16 MB boundars, since Windows doesn't return the
	 * exact amount but a bit lower.
	 */
	*pages = ~0xfff & ((*pages) + 0xfff);

	return rc;
}
