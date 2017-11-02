/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stdint.h>

typedef uint32_t clockid_t;
typedef uint32_t key_t;         /* Used for interprocess communication. */
typedef uint32_t pid_t;         /* Used for process IDs and process group IDs. */
typedef signed long ssize_t;    /* Used for a count of bytes or an error indication. */

#endif /* _SYS_TYPES_H */
