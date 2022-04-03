/****************************************************************************
 * fs/vfs/fs_statfs.c
 *
 *   Copyright (C) 2007-2009, 2012, 2017 Gregory Nutt. All rights reserved.
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

#include "sys/statfs.h"
#include "string.h"
#include "sched.h"
#include "vnode.h"
#include "fs/mount.h"
#include "errno.h"
#include "stdlib.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: statfs
 *
 * Returned Value:
 *   Zero on success; -1 on failure with errno set:
 *
 *   EACCES  Search permission is denied for one of the directories in the
 *           path prefix of path.
 *   EFAULT  Bad address.
 *   ENOENT  A component of the path path does not exist, or the path is an
 *           empty string.
 *   ENOMEM  Out of memory
 *   ENOTDIR A component of the path is not a directory.
 *   ENOSYS  The file system does not support this call.
 *
 ****************************************************************************/

int statfs(const char *path, struct statfs *buf)
{
  struct Vnode *vnode = NULL;
  struct Mount *mnt = NULL;
  int ret = OK;

  /* Sanity checks */
  if (!path || !buf)
    {
      ret = -EFAULT;
      goto errout;
    }
  if (!path[0])
    {
      ret = -ENOENT;
      goto errout;
    }
  /* Get an vnode for this file */
  VnodeHold();
  ret = VnodeLookup(path, &vnode, 0);
  if (ret < 0)
    {
      VnodeDrop();
      goto errout;
    }
  vnode->useCount++;
  VnodeDrop();

  mnt = vnode->originMount;
  if (mnt == NULL || mnt->ops == NULL || mnt->ops->Statfs == NULL)
    {
      ret = -ENOSYS;
      goto errout_with_useCount;
    }
  else
    {
      ret = mnt->ops->Statfs(mnt, buf);
      if (ret < 0)
        {
          goto errout_with_useCount;
        }
    }

  VnodeHold();
  vnode->useCount--;
  VnodeDrop();

  return OK;

  /* Failure conditions always set the errno appropriately */

errout_with_useCount:
  VnodeHold();
  vnode->useCount--;
  VnodeDrop();
errout:
  set_errno(-ret);
  return VFS_ERROR;
}
