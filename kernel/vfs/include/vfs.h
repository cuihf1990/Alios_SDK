#ifndef YOS_VFS_H
#define YOS_VFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <vfs_conf.h>

int vfs_init(void);

int vfs_device_init(void);

#ifdef __cplusplus
}
#endif

#endif
