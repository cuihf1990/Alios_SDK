/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef YOS_TYPES_API_H
#define YOS_TYPES_API_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef WITH_LWIP
#define POLLIN  0x1
#define POLLOUT 0x2
#define POLLERR 0x4
struct pollfd {
    int fd;
    short events;
    short revents;
};
#else
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#endif /* YOS_TYPES_API_H */

