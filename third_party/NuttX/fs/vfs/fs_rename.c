/****************************************************************************
 * fs/vfs/fs_rename.c
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

#include "stdio.h"
#include "unistd.h"
#include "errno.h"
#include "stdlib.h"
#include "vnode.h"
#include "limits.h"
#include "fs/fs_operation.h"
#include "path_cache.h"
/****************************************************************************
 * Public Functions
 ****************************************************************************/
static int check_rename_target(struct Vnode *old_vnode, struct Vnode *old_parent_vnode,
    struct Vnode *new_vnode, struct Vnode *new_parent_vnode)
{
  if (old_vnode == NULL || old_parent_vnode == NULL ||
      new_parent_vnode == NULL || new_parent_vnode->type != VNODE_TYPE_DIR)
    {
      return -ENOENT;
    }
  if (old_vnode->type != VNODE_TYPE_DIR && old_vnode->type != VNODE_TYPE_REG)
    {
      return -EACCES;
    }
  if (new_vnode != NULL && new_vnode->type != old_vnode->type)
    {
      if (new_vnode->type == VNODE_TYPE_DIR)
        {
          return -EISDIR;
        }
      return -ENOTDIR;
    }
  if (new_vnode != NULL && new_vnode->useCount != 0)
    {
      return -EBUSY;
    }

  if (VfsVnodePermissionCheck(old_parent_vnode, (WRITE_OP | EXEC_OP))
      || VfsVnodePermissionCheck(new_parent_vnode, (WRITE_OP | EXEC_OP)))
    {
      return -EACCES;
    }

  if (old_parent_vnode->originMount != new_parent_vnode->originMount)
    {
      return -EXDEV;
    }
  if ((old_vnode->flag & VNODE_FLAG_MOUNT_ORIGIN)
       || (old_vnode->flag & VNODE_FLAG_MOUNT_NEW))
    {
      return -EBUSY;
    }
  if (new_vnode != NULL && ((new_vnode->flag & VNODE_FLAG_MOUNT_ORIGIN)
      || (new_vnode->flag & VNODE_FLAG_MOUNT_NEW)))
    {
      return -EBUSY;
    }

  return OK;
}

static int check_path_invalid(const char *fulloldpath, const char *fullnewpath)
{
  char cwd[PATH_MAX];
  char *pret = getcwd(cwd, PATH_MAX);
  ssize_t len = strlen(fulloldpath);
  if (pret != NULL)
    {
      if (!strcmp(fulloldpath, cwd))
        {
          return -EBUSY;
        }
    }

  if (strncmp(fulloldpath, fullnewpath, len))
    {
      return OK;
    }

  if (fullnewpath[len] != '/')
    {
      return OK;
    }

  return -EINVAL;
}

int do_rename(int oldfd, const char *oldpath, int newfd, const char *newpath)
{
  struct Vnode *old_parent_vnode = NULL;
  struct Vnode *new_parent_vnode = NULL;
  struct Vnode *old_vnode = NULL;
  struct Vnode *new_vnode = NULL;
  char *fulloldpath = NULL;
  char *fullnewpath = NULL;
  char *oldname = NULL;
  char *newname = NULL;
  int ret;
  if (!oldpath || *oldpath == '\0' || !newpath || *newpath == '\0')
    {
      ret = -EINVAL;
      goto errout;
    }

  ret = vfs_normalize_pathat(oldfd, oldpath, &fulloldpath);
  if (ret < 0)
    {
      goto errout;
    }

  ret = vfs_normalize_pathat(newfd, newpath, &fullnewpath);
  if (ret < 0)
    {
      goto errout_with_oldpath;
    }
  oldname = strrchr(fulloldpath, '/') + 1;
  newname = strrchr(fullnewpath, '/') + 1;
  ret = check_path_invalid(fulloldpath, fullnewpath);
  if (ret != OK)
    {
      goto errout_with_newpath;
    }

  VnodeHold();
  ret = VnodeLookup(fulloldpath, &old_vnode, 0);
  if (ret < 0)
    {
      goto errout_with_vnode;
    }
  old_parent_vnode = old_vnode->parent;
  ret = VnodeLookup(fullnewpath, &new_vnode, 0);
  if (ret == OK)
    {
      new_parent_vnode = new_vnode->parent;
    }
  else
    {
      new_parent_vnode = new_vnode;
      new_vnode = NULL;
    }
  ret = check_rename_target(old_vnode, old_parent_vnode, new_vnode, new_parent_vnode);
  if (ret != OK)
    {
      goto errout_with_vnode;
    }
  if (old_vnode == new_vnode)
    {
      VnodeDrop();
      free(fulloldpath);
      free(fullnewpath);
      return OK;
    }
  if (!old_vnode->vop || !old_vnode->vop->Rename)
    {
      ret = -ENOSYS;
      goto errout_with_vnode;
    }
  new_parent_vnode->useCount++;
  ret = old_vnode->vop->Rename(old_vnode, new_parent_vnode, oldname, newname);
  new_parent_vnode->useCount--;
  if (ret < 0)
    {
      goto errout_with_vnode;
    }
  VnodeFree(new_vnode);
  VnodePathCacheFree(old_vnode);
  PathCacheAlloc(new_parent_vnode, old_vnode, newname, strlen(newname));
  VnodeDrop();
  ret = update_file_path(fulloldpath, fullnewpath);
  if (ret != OK)
    {
      PRINT_ERR("rename change file path failed, something bad might happped.\n");
    }
  /* Successfully renamed */

  rename_mapping(fulloldpath, fullnewpath);

  free(fulloldpath);
  free(fullnewpath);

  return OK;

errout_with_vnode:
  VnodeDrop();
errout_with_newpath:
  free(fullnewpath);
errout_with_oldpath:
  free(fulloldpath);
errout:
  set_errno(-ret);
  return VFS_ERROR;
}


/****************************************************************************
 * Name: rename
 *
 * Description:  Rename a file managed a mountpoint
 *
 ****************************************************************************/

int rename(const char *oldpath, const char *newpath)
{
  return do_rename(AT_FDCWD, oldpath, AT_FDCWD, newpath);
}

/****************************************************************************
 * Name: renameat
 *
 * Description:  Rename a file managed a mountpoint with relatively fds.
 *
 ****************************************************************************/

int renameat(int oldfd, const char *oldpath, int newdfd, const char *newpath)
{
  return do_rename(oldfd, oldpath, newdfd, newpath);
}
