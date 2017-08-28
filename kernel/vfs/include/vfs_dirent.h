#ifndef VFS_DIRENT_H
#define VFS_DIRENT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int     d_ino;                      /* file number */
    uint8_t d_type;                     /* type of file */
    char    d_name[];                   /* file name */
} yos_dirent_t;

typedef struct {
    int         dd_vfs_fd;              /* This keeps track of the current directory position for telldir */
    int         dd_rsv;
} yos_dir_t;

#ifdef __cplusplus
}
#endif

#endif /* VFS_DIRENT_H */


