#ifndef __DUMPSYSH__
#define __DUMPSYSH__

uint32_t dumpsys_task_func(char *buf, uint32_t len, int detail);
uint32_t dumpsys_func(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                      char **argv);

#endif
