#ifndef SEHWORKAROUND_H
#define SEHWORKAROUND_H
void init_sehworkaround(void);
int mysetjmp(void);
void myreleasejmp(void);
void myhandler(void);
#ifdef __i386__
#define BACK_FROM_EXCEPTION \
  __asm__ __volatile__ ("movl -8(%%esp),%%eax;movl %%eax,%%fs:0;" \
  : : : "%eax");

#elif defined(__x86_64__)
#define BACK_FROM_EXCEPTION
#endif
#endif /* SEHWORKAROUND_H */
