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

ssize_t do_readlink(int dirfd, const char *path, char *buf, size_t bufsize)
{
  struct Vnode *vnode = NULL;
  char *fullpath = NULL;
  ssize_t ret;

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

  ret = vfs_normalize_pathat(dirfd, path, &fullpath);
  if (ret < 0)
    {
      goto errout;
    }

  VnodeHold();
  ret = VnodeLookup(fullpath, &vnode, 0);
  if (ret < 0)
    {
      goto errout_with_vnode;
    }

  if (vnode->type != VNODE_TYPE_LNK)
    {
      ret = -EINVAL;
      goto errout_with_vnode;
    }

  if (!vnode->vop || !vnode->vop->Readlink)
    {
      ret = -ENOSYS;
      goto errout_with_vnode;
    }

  ret = vnode->vop->Readlink(vnode, buf, bufsize);
  if (ret < 0)
    {
      goto errout_with_vnode;
    }

  VnodeDrop();

  free(fullpath);

  return ret;

errout_with_vnode:
  VnodeDrop();
  free(fullpath);
errout:
  set_errno(-ret);
  return VFS_ERROR;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsize)
{
  return do_readlink(AT_FDCWD, pathname, buf, bufsize);
}

ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsize)
{
  return do_readlink(dirfd, pathname, buf, bufsize);
}
