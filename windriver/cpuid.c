/*
 * This source code is a part of coLinux source package.
 *
 * Dan Aloni <da-x@colinux.org>, 2004 (c)
 * Copied some stuff over from Linux's arch/i386.
 *
 * The code is licensed under the GPL. See the COPYING file at
 * the root directory.
 */

#include "cpuid.h"

co_rc_t co_i386_get_cpuid_capabilities(unsigned long *caps)
{
	cpuid_t cpuid;
	unsigned long highest_op;

	co_i386_get_cpuid(0, &cpuid);
	highest_op = cpuid.highest_op;
	if (highest_op < 0x00000001) {
		/* What's wrong with this CPU?! */
		return CO_RC(ERROR);
	}

	co_i386_get_cpuid(0x00000001, &cpuid);

	caps[0] = cpuid.edx;

	/* Inspired by Linux's arch/i386/kernel/cpu/common.c: */
	co_i386_get_cpuid(0x80000000, &cpuid);

	/* Get the AMD caps: */
	if ((cpuid.eax & 0xffff0000) == 0x80000000) {
		if (cpuid.eax >= 0x80000001) {
			co_i386_get_cpuid(0x80000001, &cpuid);
			caps[1] = cpuid.edx;
		}
	}

	return CO_RC(OK);
}
