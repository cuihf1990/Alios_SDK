#ifndef __YOC_SYSDEP_H__
#define __YOC_SYSDEP_H__

#include <k_api.h>

#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>

/* fs */
#include <poll.h>
#include <fcntl.h>

/* network */
#ifndef WITH_LWIP
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/eventfd.h>
#else
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#endif

/* for read/write/close */
#include <unistd.h>

#define HAL_ARCH_USE_NATIVE_GETTIMEOFDAY 1

#define ARCH_MAX_NOFILE 64

extern uint32_t yoc_get_free_heap_size(void);
#define HEAP_CHECK \
  printf("[DEBUG] MEMORY USAGE %s:%i, FREE %i\n", __FILE__, __LINE__, yoc_get_free_heap_size());

#endif

