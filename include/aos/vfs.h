/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_VFS_H
#define AOS_VFS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int     d_ino;                      /* file number */
    uint8_t d_type;                     /* type of file */
    char    d_name[];                   /* file name */
} aos_dirent_t;

typedef struct {
    int         dd_vfs_fd;              /* This keeps track of the current directory position for telldir */
    int         dd_rsv;
} aos_dir_t;

/**
 * @brief open the file or device by its path @path.
 *
 * @param[in] @path   the path of the file or device you want to open.
 * @param[in] @flags  the mode of open operation
 *
 * @retval  >=0 on success.
 * @retval <0 failure.
 */
int aos_open(const char *path, int flags);

/**
 * @brief close the file or device by its fd @fd.
 *
 * @param[in] @fd  the handle of the file or device.
 *
 * @retval  0 on success.
 * @retval <0 failure.
 */
int aos_close(int fd);

/**
 * @brief read the contents of a file or device into a buffer.
 *
 * @param[in]   @fd      the handle of the file or device.
 * @param[in]   @nbytes  the number of bytes to read.
 * @param[out]  @buf     the buffer to read in to.
 *
 * @retval  The number of bytes read, 0 at end of file, negative error on failure
 */
ssize_t aos_read(int fd, void *buf, size_t nbytes);

/**
 * @brief write the contents of a buffer to file or device
 *
 * @param[in]   @fd      the handle of the file or device.
 * @param[in]   @nbytes  the number of bytes to write.
 * @param[in]   @buf     the buffer to write from.
 *
 * @retval  The number of bytes written, negative error on failure.
 */
ssize_t aos_write(int fd, const void *buf, size_t nbytes);

/**
 * @brief This is a wildcard API for sending controller specific commands.
 *
 * @param[in]  @fd   the handle of the file or device
 * @param[in]  @cmd  A controller specific command.
 * @param[in]  @arg  Argument to the command; interpreted according to the command.
 *
 * @retval  any return from the command.
 */
int aos_ioctl(int fd, int cmd, unsigned long arg);

/**
 * @brief A mechanism to multiplex input/output over a set of file handles(file descriptors).
 * For every file handle provided, poll() examines it for any events registered for that particular
 * file handle.
 *
 * @param[in]  @fds      a point to the array of PollFh struct carrying a FileHandle and bitmasks of events
 * @param[in]  @nfhs     number of file handles
 * @param[in]  @timeout  timer value to timeout or -1 for loop forever
 *
 * @retval number of file handles selected (for which revents is non-zero). 0 if timed out with nothing selected. -1 for error.
 */
int aos_poll(struct pollfd *fds, int nfds, int timeout);

/**
 * @brief  performs one of the operations described below on the open file descriptor @fd.
 *         The operation is determined by @cmd.
 *
 * @param[in]  @fd   the handle of the file or device
 * @param[in]  @cmd  the operation of the file or device
 * @param[in]  @val  it depends on whether @cmd need params.
 *
 * @retval  0 on success, otherwise -1 will be returned
 */
int aos_fcntl(int fd, int cmd, int val);

/**
  * @brief move the file position to a given offset from from a given location
  *
  * @param[in] @fd     File handle
  * @param[in] @offset   The offset from whence to move to
  * @param[in] @whence   The start of where to seek
  *      SEEK_SET to start from beginning of file,
  *      SEEK_CUR to start from current position in file,
  *      SEEK_END to start from end of file
  *
  * @retval  The new offset of the file
  */
off_t aos_lseek(int fd, off_t offset, int whence);

/**
  * @brief flush any buffers associated with the file
  *
  * @param[in] @fd     File handle
  *
  * @retval 0 on success, negative error code on failure
  */
int aos_sync(int fd);

/**
  * @brief store information about the file in a stat structure
  *
  * @param[in] @path   The name of the file to find information about
  * @param[out] @st     The stat buffer to write to
  *
  * @retval 0 on success, negative error code on failure
  */
int aos_stat(const char *path, struct stat *st);

/**
  * @brief remove a file from the filesystem.
  *
  * @param[in] @path     The name of the file to remove.
  *
  * @retval 0 on success, negative error code on failure
  */
int aos_unlink(const char *path);

/**
  * @brief rename a file in the filesystem.
  *
  * @param[in] @oldpath     The name of the file to rename.
  * @param[in] @newpath   The name to rename it to
  *
  * @retval 0 on success, negative error code on failure
  */
int aos_rename(const char *oldpath, const char *newpath);

/**
  * @brief open a directory on the filesystem
  *
  * @param[in] @path     Name of the directory to open
  *
  * @retval a point of directory stream on success, NULL on failure
  */
aos_dir_t *aos_opendir(const char *path);

/**
  * @brief close a directory
  *
  * @param[in] @dir      Dir handle
  *
  * @retval 0 on success, negative error code on failure
  */
int aos_closedir(aos_dir_t *dir);

/**
  * @brief read the next directory entry
  *
  * @param[in] @dir      Dir handle
  *
  * @retval a pointer to a dirent structure.
  */
aos_dirent_t *aos_readdir(aos_dir_t *dir);

/**
  * @brief create the directory, if they do not already exist.
  *
  * @param[in] @path the name of the directory
  *
  * @retval 0 on success, negative error code on failure
  */
int aos_mkdir(const char *path);

#ifdef __cplusplus
}
#endif

#endif

