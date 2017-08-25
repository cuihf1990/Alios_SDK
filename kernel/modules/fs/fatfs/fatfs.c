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

#include <string.h>
#include <sys/fcntl.h>
#include "diskio.h"
#include "ff.h"
#include "vfs_err.h"
#include "vfs_inode.h"
#include "vfs_register.h"

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

#if FF_USE_LFN == 3 /* Dynamic memory allocation */

/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/

void* ff_memalloc ( /* Returns pointer to the allocated memory block (null on not enough core) */
    UINT msize      /* Number of bytes to allocate */
)
{
    return (void *)yos_malloc(msize);   /* Allocate a new memory block with POSIX API */
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
    void* mblock    /* Pointer to the memory block to free */
)
{
    yos_free(mblock);   /* Free the memory block with POSIX API */
}

#endif



#if FF_FS_REENTRANT /* Mutal exclusion */

/*------------------------------------------------------------------------*/
/* Create a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object for the volume, such as semaphore and mutex.
/  When a 0 is returned, the f_mount() function fails with FR_INT_ERR.
*/

//const osMutexDef_t Mutex[FF_VOLUMES]; /* CMSIS-RTOS */


int ff_cre_syncobj (    /* 1:Function succeeded, 0:Could not create the sync object */
    BYTE vol,           /* Corresponding volume (logical drive number) */
    FF_SYNC_t *sobj     /* Pointer to return the created sync object */
)
{
    int ret = yos_mutex_new(sobj);
    return (ret == FR_OK) ? 1 : 0;
}


/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj (    /* 1:Function succeeded, 0:Could not delete due to an error */
    FF_SYNC_t sobj      /* Sync object tied to the logical drive to be deleted */
)
{
    yos_mutex_free(&sobj);
    return 1;
}


/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (  /* 1:Got a grant to access the volume, 0:Could not get a grant */
    FF_SYNC_t sobj  /* Sync object to wait */
)
{
    int ret = yos_mutex_lock(&sobj, FF_FS_TIMEOUT);
    return (ret == FR_OK) ? 1 : 0;
}


/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
    FF_SYNC_t sobj  /* Sync object to be signaled */
)
{
    yos_mutex_unlock(&sobj);
}

#endif



static char* translate_relative_path(const char *path)
{
    int len, prefix_len;
    char *relpath, *p;
    BYTE pdrv;

    if (!path)
        return NULL;

    len = strlen(path);
    for (pdrv = 0; pdrv < FF_VOLUMES; pdrv++) {
        prefix_len = strlen(g_fsid[pdrv].root);
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

static off_t fatfs_lseek(file_t *fp, off_t off, int whence)
{
    off_t new_pos = 0;
    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        if (whence == SEEK_SET) {
            new_pos = off;
        } else if (whence == SEEK_CUR) {
            off_t cur_pos = f_tell(f);
            new_pos = cur_pos + off;
        } else if (whence == SEEK_END) {
            off_t size = f_size(f);
            new_pos = size + off;
        } else {
            return -1;
        }

        if (f_lseek(f, new_pos) != FR_OK) {
            return -1;
        }
    }

    return new_pos;
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
    .lseek      = &fatfs_lseek,
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

#if FF_USE_MKFS && !FF_FS_READONLY
    if (err == FR_NO_FILESYSTEM) {
        char *work = (char *)yos_malloc(FF_MAX_SS);
        if (!work) {
            err = -1;
            goto error;
        }

        BYTE opt = FM_ANY;
        disk_ioctl(pdrv, GET_FORMAT_OPTION, &opt);

        err = f_mkfs(g_fsid[pdrv].id, opt, 0, work, FF_MAX_SS);
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
#endif
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

