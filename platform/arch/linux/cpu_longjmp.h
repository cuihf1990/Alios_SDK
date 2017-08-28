#ifndef CPU_LONGJMP_H
#define CPU_LONGJMP_H

#include <cpu_arch_longjmp.h>

extern int rhino_setjmp(jmp_buf);
extern void rhino_longjmp(jmp_buf, int);

#endif /* CPU_LONGJMP_H */

