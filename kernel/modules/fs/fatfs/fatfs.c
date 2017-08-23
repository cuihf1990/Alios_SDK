/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diskio.h"
#include "ff.h"
#include "vfs_inode.h"
#include "vfs_register.h"
#include <string.h>
#include <sys/fcntl.h>


/* Definitions of physical drive number for each drive */
#define DEV_RAMDISK     0
#define DEV_SDCARD      1

DSTATUS ff_disk_status(BYTE pdrv)
{
    switch (pdrv) {
        case DEV_RAMDISK:
            // return ramdisk_status();

        case DEV_SDCARD:
            // return sdcard_status();
        default:
            break;
    }
    return STA_NOINIT;
}

DSTATUS ff_disk_initialize(BYTE pdrv)
{
    switch (pdrv) {
        case DEV_RAMDISK:
            // return ramdisk_initialize();

        case DEV_SDCARD:
            // return sdcard_initialize();
        default:
            break;
    }
    return STA_NOINIT;
}

DRESULT ff_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    switch (pdrv) {
        case DEV_RAMDISK:
            // return ramdisk_read();

        case DEV_SDCARD:
            // return sdcard_read();
        default:
            break;
    }
    return RES_PARERR;
}

DRESULT ff_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    switch (pdrv) {
        case DEV_RAMDISK:
            // return ramdisk_write();

        case DEV_SDCARD:
            // return sdcard_write();
        default:
            break;
    }
    return RES_PARERR;
}

DRESULT ff_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    switch (pdrv) {
        case DEV_RAMDISK:
            // return ramdisk_ioctl();

        case DEV_SDCARD:
            // return sdcard_ioctl();
        default:
            break;
    }
    return RES_PARERR;
}

#if FF_USE_LFN == 0
#define MAX_NAME_LEN    12
#else
#define MAX_NAME_LEN    FF_MAX_LFN
#endif

typedef struct _fsid_map_t
{
    const char *root;
    const char *id;
}fsid_map_t;

typedef struct _fat_dir_t
{
    yos_dir_t       dir;
    FF_DIR          ffdir;
    FILINFO         filinfo;
    yos_dirent_t    cur_dirent;
}fat_dir_t;

static fsid_map_t g_fsid[] = {
        { "/ramdisk",   "0:" },
        { "/sdcard",    "1:" }
};

static FATFS *g_fatfs[FF_VOLUMES] = {0};

static char* translate_relative_path(const char *path)
{
    int len, prefix_len;
    char *relpath, *p;
    BYTE pdrv;

    if (!path)
        return NULL;

    len = strlen(path);
    prefix_len = strlen(g_fsid[pdrv].root);
    for (pdrv = 0; pdrv < FF_VOLUMES; pdrv++) {
        if (strncmp(g_fsid[pdrv].root, path, prefix_len) == 0)
            break;
    }

    if (pdrv == FF_VOLUMES)
        return NULL;

    relpath = (char *)yos_malloc(len + 1);
    if (!relpath)
        return NULL;

    memset(relpath, 0, len + 1);
    memcpy(relpath, g_fsid[pdrv].id, strlen(g_fsid[pdrv].id));
    p = (char *)(path + strlen(g_fsid[pdrv].root) + 1);
    memcpy(relpath + strlen(g_fsid[pdrv].id), p, len - prefix_len - 1);
    relpath[len] = '\0';
    
    return relpath;
}

static int fatfs_mode_conv(int m)
{
    int res = 0;
    int acc_mode = m & O_ACCMODE;
    if (acc_mode == O_RDONLY) {
        res |= FA_READ;
    } else if (acc_mode == O_WRONLY) {
        res |= FA_WRITE;
    } else if (acc_mode == O_RDWR) {
        res |= FA_READ | FA_WRITE;
    }
    if ((m & O_CREAT) && (m & O_EXCL)) {
        res |= FA_CREATE_NEW;
    } else if ((m & O_CREAT) && (m & O_TRUNC)) {
        res |= FA_CREATE_ALWAYS;
    } else if (m & O_APPEND) {
        res |= FA_OPEN_ALWAYS;
    } else {
        res |= FA_OPEN_EXISTING;
    }
    return res;
}


static int fatfs_open(file_t *fp, const char *path, int flags)
{
    int ret;
    FIL *f = NULL;
    char *relpath = NULL;

    f = (FIL *)yos_malloc(sizeof(FIL));
    if (!f)
        return -1;

    relpath = translate_relative_path(path);
    if (!relpath) {
        yos_free(f);
        return -1;
    }

    ret = f_open(f, relpath, fatfs_mode_conv(flags));
    if (ret == FR_OK) {
        fp->f_arg = (void *)f;
        yos_free(relpath);
        return ret;
    }

    yos_free(relpath);
    yos_free(f);
    return -1;
}

static int fatfs_close(file_t *fp)
{
    int ret = -1;
    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        ret = f_close(f);
        if (ret == FR_OK) {
            yos_free(f);
            fp->f_arg = NULL;
        }
    }

    return ret;
}

static ssize_t fatfs_read(file_t *fp, char *buf, size_t len)
{
    ssize_t nbytes;
    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        if (f_read(f, buf, len, &nbytes) == FR_OK)
            return nbytes;
    }

    return -1;
}

static ssize_t fatfs_write(file_t *fp, const char *buf, size_t len)
{
    ssize_t nbytes;
    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        if (f_write(f, buf, len, &nbytes) == FR_OK)
            return nbytes;
    }

    return -1;
}

static int fatfs_sync(file_t *fp)
{
    int ret = -1;
    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        ret = f_sync(f);
    }

    return ret;
}

static int fatfs_stat(file_t *fp, const char *path, struct stat *st)
{
    char *relpath = NULL;
    FILINFO info;
    int ret;

    relpath = translate_relative_path(path);
    if (!relpath)
        return -1;

    if ((ret = f_stat(relpath, &info)) == FR_OK) {
        st->st_size = info.fsize;
        st->st_mode = S_IRWXU | S_IRWXG | S_IRWXO |
                    ((info.fattrib & AM_DIR) ? S_IFDIR : S_IFREG);
    }

    yos_free(relpath);
    return ret;
}

static int fatfs_unlink(file_t *fp, const char *path)
{
    char *relpath = NULL;
    int ret;

    relpath = translate_relative_path(path);
    if (!relpath)
        return -1;

    ret = f_unlink(relpath);

    yos_free(relpath);
    return ret;
}

static int fatfs_rename(file_t *fp, const char *oldpath, const char *newpath)
{
    int ret;
    char *oldname = NULL;
    char *newname = NULL;

    oldname = translate_relative_path(oldpath);
    if (!oldname)
        return -1;

    newname = translate_relative_path(newpath);
    if (!newname) {
        yos_free(oldname);
        return -1;
    }

    ret = f_rename(oldname, newname);

    yos_free(oldname);
    yos_free(newname);
    return ret;
}

static yos_dir_t* fatfs_opendir(file_t *fp, const char *path)
{
    fat_dir_t *dp = NULL;
    char *relpath = NULL;

    relpath = translate_relative_path(path);
    if (!relpath)
        return NULL;

    dp = (fat_dir_t *)yos_malloc(sizeof(fat_dir_t) + MAX_NAME_LEN + 1);
    if (!dp) {
        yos_free(relpath);
        return NULL;
    }

    memset(dp, 0, sizeof(fat_dir_t) + MAX_NAME_LEN + 1);
    if (f_opendir(&dp->ffdir, relpath) == FR_OK) {
        yos_free(relpath);
        return (yos_dir_t *)dp;
    }

    yos_free(relpath);
    yos_free(dp);
    return NULL;
}

static yos_dirent_t* fatfs_readdir(file_t *fp, yos_dir_t *dir)
{
    fat_dir_t *dp = (fat_dir_t *)dir;
    yos_dirent_t *out_dirent;

    if (!dp)
        return NULL;

    if (f_readdir(&dp->ffdir, &dp->filinfo) != FR_OK)
        return NULL;

    if (dp->filinfo.fname[0] == 0)
        return NULL;

    dp->cur_dirent.d_ino = 0;
    if (dp->filinfo.fattrib & AM_DIR) {
        dp->cur_dirent.d_type = AM_DIR;
    }

    strncpy(dp->cur_dirent.d_name, dp->filinfo.fname, MAX_NAME_LEN);
    dp->cur_dirent.d_name[MAX_NAME_LEN] = '\0';

    out_dirent = &dp->cur_dirent;
    return out_dirent;
}

static int fatfs_closedir(file_t *fp, yos_dir_t *dir)
{
    int ret = -1;
    fat_dir_t *dp = (fat_dir_t *)dir;

    if (!dp)
        return -1;

    ret = f_closedir(&dp->ffdir);
    if (ret == FR_OK)
        yos_free(dp);

    return ret;
}

static int fatfs_mkdir(file_t *fp, const char *path)
{
    int ret = -1;
    char *relpath = NULL;

    relpath = translate_relative_path(path);
    if (!relpath)
        return -1;

    ret = f_mkdir(relpath);

    yos_free(relpath);
    return ret;
}

static const fs_ops_t fatfs_ops = {
    .open       = &fatfs_open,
    .close      = &fatfs_close,
    .read       = &fatfs_read,
    .write      = &fatfs_write,
    .lseek      = NULL,
    .sync       = &fatfs_sync,
    .stat       = &fatfs_stat,
    .unlink     = &fatfs_unlink,
    .rename     = &fatfs_rename,
    .opendir    = &fatfs_opendir,
    .readdir    = &fatfs_readdir,
    .closedir   = &fatfs_closedir,
    .mkdir      = &fatfs_mkdir
};

int fatfs_register(unsigned char pdrv)
{
    int err;
    FATFS *fatfs = NULL;

    if (g_fatfs[pdrv] != NULL)
        return FR_OK;

    fatfs = (FATFS *)yos_malloc(sizeof(FATFS));
    if (!fatfs)
        return -1;

    err = f_mount(fatfs, g_fsid[pdrv].id, 1);

    if (err == FR_OK) {
        g_fatfs[pdrv] = fatfs;
        return yos_register_fs(g_fsid[pdrv].root, &fatfs_ops, NULL);
    }

    if (err == FR_NO_FILESYSTEM) {
        char *work = (char *)yos_malloc(FF_MAX_SS);
        if (!work) {
            err = -1;
            goto error;
        }
        
        err = f_mkfs(g_fsid[pdrv].id, FM_ANY | FM_SFD, 0, work, FF_MAX_SS);
        yos_free(work);

        if (err != FR_OK)
            goto error;

        f_mount(NULL, g_fsid[pdrv].id, 1);
        err = f_mount(fatfs, g_fsid[pdrv].id, 1);

        if (err == FR_OK) {
            g_fatfs[pdrv] = fatfs;
            return yos_register_fs(g_fsid[pdrv].root, &fatfs_ops, NULL);
        }
    }

error:
    yos_free(fatfs);
    return err;
}


int fatfs_unregister(unsigned char pdrv)
{
    int err = FR_OK;

    err = yos_unregister_fs(g_fsid[pdrv].root);
    if (err == FR_OK) {
        f_mount(NULL, g_fsid[pdrv].id, 1);
        yos_free(g_fatfs[pdrv]);
    }

    return err;
}

