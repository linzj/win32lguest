#include "sehworkaround.h"
#include <setjmp.h>
#include <wdm.h>
static jmp_buf g_buf;
static FAST_MUTEX g_mutex;

void myhandler(void)
{
    longjmp(g_buf, 1);
}

int mysetjmp(void)
{
    ExAcquireFastMutex(&g_mutex);
    return setjmp(g_buf);
}

void myreleasejmp(void)
{
    ExReleaseFastMutex(&g_mutex);
}

void init_sehworkaround(void)
{
    ExInitializeFastMutex(&g_mutex);
}
