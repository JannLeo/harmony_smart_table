/****************************************************************************
 * fs/vfs/fs_unlink.c
 *
 *   Copyright (C) 2007-2009, 2017 Gregory Nutt. All rights reserved.
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
#include "errno.h"
#include "fcntl.h"

#include "vnode.h"
#include "stdlib.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static int check_target(struct Vnode *vnode)
{
  if (vnode->type == VNODE_TYPE_DIR)
    {
      return -EISDIR;
    }

  if (vnode->useCount > 0)
    {
      return -EBUSY;
    }

  if (VfsVnodePermissionCheck(vnode->parent, (WRITE_OP | EXEC_OP)))
    {
      return -EACCES;
    }
  return OK;
}
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: do_unlink
 *
 * Description:  Remove a file managed a mountpoint
 *
 ****************************************************************************/

int do_unlink(int dirfd, const char *pathname)
{
  struct Vnode *vnode = NULL;
  int ret;
  char *name = NULL;
  char *fullpath = NULL;
  char *relativepath = NULL;

  /* Get relative path by dirfd*/
  ret = get_path_from_fd(dirfd, &relativepath);
  if (ret < 0)
    {
      goto errout;
    }

  ret = vfs_normalize_path((const char *)relativepath, pathname, &fullpath);
  if (relativepath)
    {
      free(relativepath);
    }

  if (ret < 0)
    {
      goto errout;
    }

  VnodeHold();
  ret = VnodeLookup(fullpath, &vnode, 0);
  if (ret < 0)
    {
      goto errout_with_lock;
    }

  ret = check_target(vnode);
  if (ret < 0)
    {
      goto errout_with_lock;
    }
  name = strrchr(fullpath, '/') + 1;

  if (vnode && vnode->vop && vnode->vop->Unlink)
    {
      ret = vnode->vop->Unlink(vnode->parent, vnode, name);
    }
  else if (vnode && vnode->fop && vnode->fop->unlink)
    {
      ret = vnode->fop->unlink(vnode);
      if (ret == OK) {
        goto done;
      }
    }
  else
    {
      ret = -ENOSYS;
    }

  if (ret != OK)
    {
      goto errout_with_lock;
    }

  VnodeFree(vnode);

done:
  VnodeDrop();
#ifdef LOSCFG_KERNEL_VM
  (void)remove_mapping(fullpath);
#endif
  /* Successfully unlinked */

  free(fullpath);
  return OK;

errout_with_lock:
  VnodeDrop();

errout:
  if (fullpath)
    {
      free(fullpath);
    }
  set_errno(-ret);

  return VFS_ERROR;
}

/****************************************************************************
 * Name: unlink
 *
 * Description:  Remove a file managed a mountpoint
 *
 ****************************************************************************/

int unlink(const char *pathname)
{
  return do_unlink(AT_FDCWD, pathname);
}

/****************************************************************************
 * Name: unlinkat
 *
 * Description:  Remove a file managed a mountpoint by dirfd
 *
 ****************************************************************************/
extern int do_rmdir(int dirfd, const char *pathname);

int unlinkat(int dirfd, const char *pathname, int flag)
{
  /* Now flag only support 0 && AT_REMOVEDIR */
  if ((flag & ~AT_REMOVEDIR) != 0)
    return VFS_ERROR;

  if (flag & AT_REMOVEDIR)
    return do_rmdir(dirfd, pathname);

  return do_unlink(dirfd, pathname);
}
