/****************************************************************************
 * fs/dirent/fs_opendir.c
 *
 *   Copyright (C) 2007-2009, 2011, 2013-2014, 2017-2018 Gregory Nutt. All
 *     rights reserved.
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
#include "dirent.h"
#include "string.h"
#include "assert.h"
#include "errno.h"
#include "stdlib.h"
#include "fs/dirent_fs.h"
#include "vnode.h"
#include "path_cache.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: opendir
 *
 * Description:
 *   The  opendir() function opens a directory stream corresponding to the
 *   directory name, and returns a pointer to the directory stream. The
 *   stream is positioned at the first entry in the directory.
 *
 * Input Parameters:
 *   path -- the directory to open
 *
 * Returned Value:
 *   The opendir() function returns a pointer to the directory stream.  On
 *   error, NULL is returned, and errno is set appropriately.
 *
 *   EACCES  - Permission denied.
 *   EMFILE  - Too many file descriptors in use by process.
 *   ENFILE  - Too many files are currently open in the
 *             system.
 *   ENOENT  - Directory does not exist, or name is an empty
 *             string.
 *   ENOMEM  - Insufficient memory to complete the operation.
 *   ENOTDIR - 'path' is not a directory.
 *
 ****************************************************************************/

DIR *opendir(const char *path)
{
  struct Vnode *vp = NULL;
  struct fs_dirent_s *dir = NULL;
  int ret;

  /* Find the node matching the path. */
  VnodeHold();
  ret = VnodeLookup(path, &vp, 0);
  if (vp == NULL || ret != OK)
    {
      VnodeDrop();
      goto errout;
    }
  if (vp->type != VNODE_TYPE_DIR)
    {
      ret = -ENOTDIR;
      PRINT_ERR("opendir (%s) failed, err=%d\n", path, ret);
      VnodeDrop();
      goto errout;
    }

  vp->useCount++;
  VnodeDrop();

  /* Allocate a type DIR -- which is little more than an vp container. */
  dir = (struct fs_dirent_s *)calloc(1, sizeof(struct fs_dirent_s));
  if (!dir)
    {
      /* Insufficient memory to complete the operation. */
      ret = -ENOMEM;
      goto errout_with_count;
    }

  /* Populate the DIR structure and return it to the caller.  The way that
   * we do this depends on whenever this is a "normal" pseudo-file-system
   * vp or a file system mountpoint.
   */

  dir->fd_position = 0;      /* This is the position in the read stream */

  if (vp->vop != NULL && vp->vop->Opendir != NULL)
    {
      ret = vp->vop->Opendir(vp, dir);
    }
  else
    {
      ret = -ENOSYS;
    }
  if (ret < 0)
    {
      goto errout_with_count;
    }
  dir->fd_status = DIRENT_MAGIC;
  dir->fd_root = vp;

  return ((DIR *)dir);

  /* Nasty goto's make error handling simpler */

errout_with_count:
  VnodeHold();
  vp->useCount--;
  VnodeDrop();
errout:
  set_errno(-ret);
  return NULL;
}

int do_opendir(const char *path, int oflags)
{
  int ret;
  int fd;

  struct Vnode *vp = NULL;
  struct file *filep = NULL;
  struct fs_dirent_s *dir = NULL;
  char *fullpath = NULL;
  char *relativepath = NULL;

  if (path == NULL || *path == 0)
    {
       ret = -EINVAL;
       goto errout;
    }

  ret = get_path_from_fd(AT_FDCWD, &relativepath);
  if (ret < 0)
    {
      goto errout;
    }

  ret = vfs_normalize_path((const char *)relativepath, path, &fullpath);
  if (relativepath)
    {
      free(relativepath);
    }
  if (ret < 0)
    {
      goto errout;
    }

  VnodeHold();
  /* Get an vnode for this file */
  ret = VnodeLookup(path, &vp, 0);
  if (ret < 0)
    {
      VnodeDrop();
      goto errout;
    }
  if (vp->type != VNODE_TYPE_DIR)
    {
      ret = -ENOTDIR;
      VnodeDrop();
      goto errout;
    }
  vp->useCount++;
  VnodeDrop();

  /* Associate the vnode with a file structure */
  fd = files_allocate(vp, oflags, 0, NULL, 3); /* 3: file start fd */
  if (fd < 0)
    {
      ret = -EMFILE;
      goto errout_with_vnode;
    }

  /* Get the file structure corresponding to the file descriptor. */
  ret = fs_getfilep(fd, &filep);
  if (ret < 0)
    {
      ret = -EPERM;
      goto errout_with_fd;
    }

  filep->f_path = (char *)fullpath; /* The mem will free in close(fd); */

  /* Allocate a type DIR -- which is little more than an vnode  container. */
  dir = (struct fs_dirent_s *)zalloc(sizeof(struct fs_dirent_s));
  if (dir == NULL)
    {
      ret = -ENOMEM;
      goto errout_with_fd;
    }
  dir->fd_position = 0;      /* This is the position in the read stream */

  /* Open the directory at the relative path */
  if (vp->vop != NULL && vp->vop->Opendir != NULL)
    {
      ret = vp->vop->Opendir(vp, dir);
    }
  else
    {
      ret = -ENOSYS;
    }

  if (ret < 0)
    {
      free(dir);
      goto errout_with_fd;
    }

  dir->fd_root = vp;
  dir->fd_status = DIRENT_MAGIC;
  filep->f_dir = dir;

  return fd;

errout_with_fd:
  files_release(fd);
errout_with_vnode:
  VnodeHold();
  vp->useCount--;
  VnodeDrop();
errout:
  if (fullpath)
    {
      free(fullpath);
    }
  set_errno(-ret);
  return VFS_ERROR;
}
