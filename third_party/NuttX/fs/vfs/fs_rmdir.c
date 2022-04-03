/****************************************************************************
 * fs/vfs/fs_rmdir.c
 *
 *   Copyright (C) 2007-2009, 2014, 2017 Gregory Nutt. All rights reserved.
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
#include "stdlib.h"
#include "vnode.h"
#include "sys/stat.h"
#include "string.h"
#include "limits.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static int check_target(struct Vnode *vnode, char *name) {
  if (vnode == NULL)
    {
      return -ENOENT;
    }

  if (vnode->type != VNODE_TYPE_DIR)
    {
      return -ENOTDIR;
    }

  if (vnode->useCount > 0)
    {
      return -EBUSY;
    }

  if ((vnode->flag & VNODE_FLAG_MOUNT_ORIGIN)
      || (vnode->flag & VNODE_FLAG_MOUNT_NEW))
    {
      return -EBUSY;
    }

  char cwd[PATH_MAX];
  char *pret = getcwd(cwd, PATH_MAX);
  if (pret != NULL)
    {
      struct Vnode *cwdnode = NULL;
      int ret = VnodeLookup(cwd, &cwdnode, 0);
      if (ret == OK && (cwdnode == vnode))
        {
          return -EBUSY;
        }
    }
  return OK;
}
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: do_rmdir
 *
 * Description:  Remove a file managed a mountpoint
 *
 ****************************************************************************/

int do_rmdir(int dirfd, const char *pathname)
{
  struct Vnode *vnode = NULL;
  char             *fullpath     = NULL;
  char             *relativepath = NULL;
  char             *name         = NULL;
  int               ret;

  /* Get relative path by dirfd*/
  ret = get_path_from_fd(dirfd, &relativepath);
  if (ret < 0)
    {
      goto errout;
    }

  if (relativepath)
    {
      ret = vfs_normalize_path((const char *)relativepath, pathname, &fullpath);
      free(relativepath);
      if (ret < 0)
        {
          goto errout;
        }

      name = strrchr(fullpath, '/');
      VnodeHold();
      ret = VnodeLookup(fullpath, &vnode, 0);
    }
  else
    {
      name = strrchr(pathname, '/');
      VnodeHold();
      if (name == NULL)
        {
          name = (char *)pathname;
        }
      else
        {
          name++;
        }
      ret = VnodeLookup(pathname, &vnode, 0);
    }

  if (ret != OK)
    {
      goto errout_with_lock;
    }

  ret = check_target(vnode, name);
  if (ret != OK) {
      PRINT_ERR("rmdir failed err = %d\n", ret);
      goto errout_with_lock;
  }

  if (VfsVnodePermissionCheck(vnode->parent, (WRITE_OP | EXEC_OP))) {
      ret = -EACCES;
      goto errout_with_lock;
  }

  if (vnode && vnode->vop && vnode->vop->Rmdir) {
      ret = vnode->vop->Rmdir(vnode->parent, vnode, name);
  } else {
      ret = -ENOSYS;
  }
  if (ret < 0) {
      goto errout_with_lock;
  }
  VnodeFree(vnode);
  VnodeDrop();

  /* Successfully unlinked */
  if (fullpath)
    {
      free(fullpath);
    }

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
 * Name: rmdir
 *
 * Description:  Remove a file managed a mountpoint
 *
 ****************************************************************************/

int rmdir(const char *pathname)
{
  return do_rmdir(AT_FDCWD, pathname);
}
