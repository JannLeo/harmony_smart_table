/****************************************************************************
 * include/fs/file.h
 *
 *   Copyright (C) 2007-2009, 2011-2013, 2015-2018 Gregory Nutt. All rights
 *     reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_FS_FILE_H
#define __INCLUDE_FS_FILE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "vfs_config.h"

#include "sys/types.h"
#include "stdarg.h"
#include "stdint.h"

#include "semaphore.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/****************************************************************************
 * Global Function Prototypes
 ****************************************************************************/

/* Callback used by foreach_mountpoints to traverse all mountpoints in the
 * pseudo-file system.
 */

struct statfs;                    /* Forward reference */
typedef int (*foreach_mountpoint_t)(const char *mountpoint,
                                    struct statfs *statbuf,
                                    void *arg);

struct filelist *sched_getfiles(void);

/* fs/fs_sendfile.c *************************************************/
/****************************************************************************
 * Name: sendfile
 *
 * Description:
 *   Copy data between one file descriptor and another.
 *
 ****************************************************************************/
ssize_t sendfile(int outfd, int infd, off_t *offset, size_t count);

/**
 * @ingroup  fs
 * @brief get the path by a given file fd.
 *
 * @par Description:
 * The function is used for getting the path by a given file fd.
 *
 * @attention
 * <ul>
 * <li>Only support file fd, not any dir fd.</li>
 * </ul>
 *
 * @param  fd               [IN] Type #int     file fd.
 * @param  path             [IN] Type #char ** address of the location to return the path reference.
 *
 * @retval #0      get path success
 * @retval #~0     get path failed
 *
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since 2020-1-8
 */

extern int get_path_from_fd(int fd, char **path);
bool get_bit(int i);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* __INCLUDE_FS_FILE_H */
