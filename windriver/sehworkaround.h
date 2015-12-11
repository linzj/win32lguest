#ifndef SEHWORKAROUND_H
#define SEHWORKAROUND_H
void init_sehworkaround(void);
int mysetjmp(void);
void myreleasejmp(void);
void myhandler(void);
#endif /* SEHWORKAROUND_H */
