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

int do_link(int oldfd, const char *oldpath, int newfd, const char *newpath, int flag)
{
  struct Vnode *new_parent_vnode = NULL;
  struct Vnode *new_vnode = NULL;
  struct Vnode *old_vnode = NULL;
  char *fulloldpath = NULL;
  char *fullnewpath = NULL;
  char *newname = NULL;
  int ret;

  if (!oldpath || !newpath)
    {
      ret = -EFAULT;
      goto errout;
    }

  if (*oldpath == '\0' || *newpath == '\0' || flag & ~AT_SYMLINK_FOLLOW)
    {
      ret = -EINVAL;
      goto errout;
    }

  ret = vfs_normalize_pathat(newfd, newpath, &fullnewpath);
  if (ret < 0)
    {
      goto errout;
    }

  if (!(flag & AT_SYMLINK_FOLLOW))
    {
      ret = vfs_normalize_pathat(oldfd, oldpath, &fulloldpath);
      if (ret < 0)
        {
          goto errout_with_newpath;
        }
    }

  newname = strrchr(fullnewpath, '/') + 1;

  VnodeHold();

  if (flag & AT_SYMLINK_FOLLOW)
    {
      ret = follow_symlink(oldfd, oldpath, &old_vnode, &fulloldpath);
      if (ret < 0)
        {
          goto errout_with_vnode;
        }
    }
  else
    {
      ret = VnodeLookup(fulloldpath, &old_vnode, 0);
      if (ret < 0)
        {
          goto errout_with_vnode;
        }
    }

  if (old_vnode->type != VNODE_TYPE_REG && old_vnode->type != VNODE_TYPE_LNK)
    {
      ret = -EPERM;
      goto errout_with_vnode;
    }

  ret = VnodeLookup(fullnewpath, &new_parent_vnode, 0);
  if (ret == OK)
    {
      ret = -EEXIST;
      goto errout_with_vnode;
    }

  if (old_vnode->originMount != new_parent_vnode->originMount)
    {
      ret = -EXDEV;
      goto errout_with_vnode;
    }

  if (!old_vnode->vop || !old_vnode->vop->Link)
    {
      ret = -ENOSYS;
      goto errout_with_vnode;
    }
  new_parent_vnode->useCount++;
  ret = old_vnode->vop->Link(old_vnode, new_parent_vnode, &new_vnode, newname);
  new_parent_vnode->useCount--;
  if (ret < 0)
    {
      goto errout_with_vnode;
    }
  PathCacheAlloc(new_parent_vnode, new_vnode, newname, strlen(newname));
  VnodeDrop();

  free(fulloldpath);
  free(fullnewpath);

  return OK;

errout_with_vnode:
  VnodeDrop();
  free(fulloldpath);
errout_with_newpath:
  free(fullnewpath);
errout:
  set_errno(-ret);
  return VFS_ERROR;
}

int link(const char *oldpath, const char *newpath)
{
  return do_link(AT_FDCWD, oldpath, AT_FDCWD, newpath, 0);
}

int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags)
{
  return do_link(olddirfd, oldpath, newdirfd, newpath, flags);
}
