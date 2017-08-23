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

#ifndef YOS_VFS_H
#define YOS_VFS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief open the file or device by its path @path.
 *
 * @param[in] @path   the path of the file or device you want to open.
 * @param[in] @flags  the mode of open operation
 *
 * @retval  >=0 on success.
 * @retval <0 failure.
 */
int yos_open(const char *path, int flags);

/**
 * @brief close the file or device by its fd @fd.
 *
 * @param[in] @fd  the handle of the file or device.
 *
 * @retval  0 on success.
 * @retval <0 failure.
 */
int yos_close(int fd);

/**
 * @brief read the contents of a file or device into a buffer.
 *
 * @param[in]   @fd      the handle of the file or device.
 * @param[in]   @nbytes  the number of bytes to read.
 * @param[out]  @buf     the buffer to read in to.
 *
 * @retval  The number of bytes read, 0 at end of file, negative error on failure
 */
ssize_t yos_read(int fd, void *buf, size_t nbytes);

/**
 * @brief write the contents of a buffer to file or device
 *
 * @param[in]   @fd      the handle of the file or device.
 * @param[in]   @nbytes  the number of bytes to write.
 * @param[in]   @buf     the buffer to write from.
 *
 * @retval  The number of bytes written, negative error on failure.
 */
ssize_t yos_write(int fd, const void *buf, size_t nbytes);

/**
 * @brief This is a wildcard API for sending controller specific commands.
 *
 * @param[in]  @fd   the handle of the file or device
 * @param[in]  @cmd  A controller specific command.
 * @param[in]  @arg  Argument to the command; interpreted according to the command.
 * @retval  any return from the command.
 */
int yos_ioctl(int fd, int cmd, unsigned long arg);

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
int yos_poll(struct pollfd *fds, int nfds, int timeout);

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
int yos_fcntl(int fd, int cmd, int val);

#ifdef __cplusplus
}
#endif

#endif

