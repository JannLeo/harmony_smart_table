/****************************************************************************
 * fs/vfs/fs_mkdir.c
 *
 *   Copyright (C) 2007, 2008, 2014, 2017 Gregory Nutt. All rights reserved.
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
#include "sys/types.h"
#include "sys/stat.h"
#include "stdlib.h"
#include "vnode.h"
#include "string.h"
#include "capability_api.h"
#include "path_cache.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/
int do_mkdir(int dirfd, const char *pathname, mode_t mode)
{
  struct Vnode *parentVnode = NULL;
  struct Vnode *vnode = NULL;
  int ret;
  char *fullpath = NULL;
  char *relativepath = NULL;
  char *dirname = NULL;

  mode &= ~GetUmask();
  mode &= (S_IRWXU|S_IRWXG|S_IRWXO);

  /* Get absolute path by dirfd*/
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
  if (!strncmp(fullpath, "/dev", 4) || !strncmp(fullpath, "/proc", 5))
    {
      // virtual root create virtual dir
      VnodeHold();
      ret = VnodeLookup(fullpath, &vnode, V_DUMMY|V_CREATE);
      if (ret != OK)
        {
          goto errout_with_lock;
        }
      vnode->mode = mode | S_IFDIR;
      vnode->type = VNODE_TYPE_DIR;
      VnodeDrop();
      goto out;
    }

  dirname = strrchr(fullpath, '/') + 1;

  VnodeHold();
  ret = VnodeLookup(fullpath, &parentVnode, 0);
  if (ret == OK)
    {
      ret = -EEXIST;
      goto errout_with_lock;
    }

  if (parentVnode == NULL)
    {
      ret = -ENOENT;
      goto errout_with_lock;
    }
  parentVnode->useCount++;

  if (VfsVnodePermissionCheck(parentVnode, (WRITE_OP | EXEC_OP)))
    {
      ret = -EACCES;
      goto errout_with_count;
    }

  if ((parentVnode->vop != NULL) && (parentVnode->vop->Mkdir != NULL))
    {
      ret = parentVnode->vop->Mkdir(parentVnode, dirname, mode, &vnode);
    }
  else
    {
      ret = -ENOSYS;
    }

  if (ret < 0)
    {
      goto errout_with_count;
    }

  struct PathCache *dt = PathCacheAlloc(parentVnode, vnode, dirname, strlen(dirname));
  if (dt == NULL) {
      // alloc name cache failed is not a critical problem, let it go.
      PRINT_ERR("alloc path cache %s failed\n", dirname);
  }
  parentVnode->useCount--;
  VnodeDrop();
out:
  /* Directory successfully created */
  free(fullpath);

  return OK;
errout_with_count:
  parentVnode->useCount--;
errout_with_lock:
  VnodeDrop();
errout:
  set_errno(-ret);
  if (fullpath)
    {
      free(fullpath);
    }
  return VFS_ERROR;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: mkdir
 *
 * Description:  Create a directory
 *
 ****************************************************************************/

int mkdir(const char *pathname, mode_t mode)
{
  return do_mkdir(AT_FDCWD, pathname, mode);
}

/****************************************************************************
 * Name: mkdirat
 *
 * Description:  Create a directory by dirfd
 *
 ****************************************************************************/

int mkdirat(int dirfd, const char *pathname, mode_t mode)
{
  return do_mkdir(dirfd, pathname, mode);
}
