/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef FS_FAT_H
#define FS_FAT_H

#ifdef __cplusplus
 extern "C" {
#endif
 
int fatfs_register(unsigned char pdrv);
int fatfs_unregister(unsigned char pdrv);
 
#ifdef __cplusplus
 }
#endif

 
#endif
