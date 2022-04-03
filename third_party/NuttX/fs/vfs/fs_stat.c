/****************************************************************************
 * fs/vfs/fs_stat.c
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

#include "errno.h"
#include "sys/stat.h"
#include "string.h"
#include "stdlib.h"
#include "vnode.h"
/****************************************************************************
 * Global Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stat
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
 *
 ****************************************************************************/

int stat(const char *path, struct stat *buf)
{
  struct Vnode *vp = NULL;
  int ret;

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
  ret = VnodeLookup(path, &vp, 0);
  if (ret < 0)
    {
      VnodeDrop();
      goto errout;
    }

  /* The way we handle the stat depends on the type of vnode that we
   * are dealing with.
   */

  if (vp->vop != NULL && vp->vop->Getattr != NULL)
    {
      vp->useCount++;
      VnodeDrop();
      ret = vp->vop->Getattr(vp, buf);
      VnodeHold();
      vp->useCount--;
      VnodeDrop();
    }
  else
    {
      VnodeDrop();
      ret = -ENOSYS;
      goto errout;
    }

  if (ret < 0)
    {
      goto errout;
    }

  return OK;

 /* Failure conditions always set the errno appropriately */

errout:
  set_errno(-ret);
  return VFS_ERROR;
}
