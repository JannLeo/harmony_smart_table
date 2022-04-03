/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "unistd.h"
#include "errno.h"
#include "vnode.h"
#include "path_cache.h"

int follow_symlink(int dirfd, const char *path, struct Vnode **vnode, char **fullpath)
{
  int ret;
  struct Vnode *newvnode = NULL;
  char pathname[PATH_MAX] = {0};

  (void)strcpy_s(pathname, PATH_MAX, path);

  for (int i = 0; i < CONFIG_FS_MAX_LNK_CNT; i++)
    {
      if (*fullpath)
        {
          free(*fullpath);
          *fullpath = NULL;
        }

      ret = vfs_normalize_pathat(dirfd, pathname, fullpath);
      if (ret < 0)
        {
          return ret;
        }

      ret = VnodeLookup(*fullpath, &newvnode, 0);
      if (ret != OK)
        {
          /* The object of fullpath is not exist. Return its parent's vnode. */
          *vnode = newvnode;
          return ret;
        }
      if (newvnode->type != VNODE_TYPE_LNK)
        {
          /* The object of fullpath is exist, and is not a symbol link. Return its vnode. */
          *vnode = newvnode;
          return ret;
        }
      if (newvnode->vop->Readlink == NULL)
        {
          ret = -ENOSYS;
          return ret;
        }

      /* The object of fullpath is a symbol link. Read its target and find the source file successively. */
      (void)memset_s(pathname, PATH_MAX, 0, PATH_MAX);
      ret = newvnode->vop->Readlink(newvnode, pathname, PATH_MAX);
      if (ret < 0)
        {
          return ret;
        }
    }

  /* Failed to find the source file in CONFIG_FS_MAX_LNK_CNT times. */
  return -ELOOP;
}

int do_symlink(const char *target, int newfd, const char *path)
{
  struct Vnode *parent_vnode = NULL;
  struct Vnode *new_vnode = NULL;
  char *fullpath = NULL;
  char *newname = NULL;
  int ret;

  if (!path)
    {
      ret = -EFAULT;
      goto errout;
    }

  if (*path == '\0')
    {
      ret = -EINVAL;
      goto errout;
    }

  if (strlen(target) >= PATH_MAX)
    {
      ret = -ENAMETOOLONG;
      goto errout;
    }

  ret = vfs_normalize_pathat(newfd, path, &fullpath);
  if (ret < 0)
    {
      goto errout;
    }

  newname = strrchr(fullpath, '/') + 1;

  VnodeHold();
  ret = VnodeLookup(fullpath, &parent_vnode, 0);
  if (ret == 0)
    {
      ret = -EEXIST;
      goto errout_with_vnode;
    }

  if (!parent_vnode->vop || !parent_vnode->vop->Symlink)
    {
      ret = -ENOSYS;
      goto errout_with_vnode;
    }

  parent_vnode->useCount++;
  ret = parent_vnode->vop->Symlink(parent_vnode, &new_vnode, (const char *)newname, (const char *)target);
  parent_vnode->useCount--;
  if (ret < 0)
    {
      goto errout_with_vnode;
    }

  PathCacheAlloc(parent_vnode, new_vnode, newname, strlen(newname));
  VnodeDrop();

  free(fullpath);

  return OK;

errout_with_vnode:
  VnodeDrop();
  free(fullpath);
errout:
  set_errno(-ret);
  return VFS_ERROR;
}

int symlink(const char *target, const char *path)
{
  return do_symlink(target, AT_FDCWD, path);
}

int symlinkat(const char *target, int newdirfd, const char *path)
{
  return do_symlink(target, newdirfd, path);
}
