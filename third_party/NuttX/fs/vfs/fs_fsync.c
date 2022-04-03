/****************************************************************************
 * fs/vfs/fs_fsync.c
 *
 *   Copyright (C) 2007-2009, 2013-2014, 2016-2017 Gregory Nutt. All rights
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "vfs_config.h"

#include "unistd.h"
#include "fcntl.h"
#include "errno.h"
#include "assert.h"



#include "vnode.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_fsync
 *
 * Description:
 *   Equivalent to the standard fsync() function except that is accepts a
 *   struct file instance instead of a file descriptor and it does not set
 *   the errno variable.
 *
 ****************************************************************************/

int file_fsync(struct file *filep)
{
  int ret;

  /* Was this file opened for write access? */

  if ((filep->f_oflags & O_ACCMODE) == 0)
    {
      ret = EBADF;
      goto errout;
    }

  /* Is this vnode a registered mountpoint? Does it support the
   * sync operations may be relevant to device drivers but only
   * the mountpoint operations vtable contains a sync method.
   */

  if (!filep || !filep->ops || !filep->ops->fsync)
    {
      ret = EINVAL;
      goto errout;
    }

  /* Yes, then tell the mountpoint to sync this file */

  ret = filep->ops->fsync(filep);
  if (ret >= 0)
    {
      return OK;
    }

  ret = -ret;

errout:
  set_errno(ret);
  return VFS_ERROR;
}

/****************************************************************************
 * Name: fsync
 *
 * Description:
 *   This func simply binds vnode sync methods to the sync system call.
 *
 ****************************************************************************/

int fsync(int fd)
{
  struct file *filep = NULL;

  /* Get the file structure corresponding to the file descriptor. */

  int ret = fs_getfilep(fd, &filep);
  if (ret < 0)
    {
      /* The errno value has already been set */
      return VFS_ERROR;
    }

  /* Perform the fsync operation */

  return file_fsync(filep);
}

