/****************************************************************************
 * fs/dirent/fs_closedir.c
 *
 *   Copyright (C) 2007-2009, 2011, 2013-2014 Gregory Nutt. All rights reserved.
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
#include "errno.h"
#include "stdlib.h"
#include "fs/dirent_fs.h"
#include "vnode.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: closedir
 *
 * Description:
 *    The closedir() function closes the directory stream associated with
 *    'dirp'.  The directory stream descriptor 'dirp' is not available after
 *    this call.
 *
 * Input Parameters:
 *   dirp -- An instance of type DIR created by a previous call to opendir();
 *
 * Returned Value:
 *   The closedir() function returns 0 on success.  On error, -1 is
 *   returned, and errno is set appropriately.
 *
 ****************************************************************************/

int closedir(DIR *dirp)
{
  struct fs_dirent_s *idir = (struct fs_dirent_s *)dirp;
  struct Vnode *vnode = NULL;
  int ret;

  /* Verify that we were provided with a valid directory structure */

  if (!idir || idir->fd_status != DIRENT_MAGIC)
    {
      ret = -EBADF;
      goto errout;
    }

  if (idir->fd_root)
    {
      /* This is the 'root' vnode of the directory.  This means different
       * things wih different filesystems.
       */
      vnode = idir->fd_root;
      /* Perform the closedir() operation */
      if (vnode->vop && vnode->vop->Closedir)
        {
          ret = vnode->vop->Closedir(vnode, idir);
          if (ret < 0)
            {
              goto errout_with_vnode;
            }
        }
      else
        {
          ret = -ENOSYS;
          goto errout_with_vnode;
        }
      VnodeHold();
      vnode->useCount--;
      VnodeDrop();
    }

  /* Then release the container */

  idir->fd_status = 0;
  free(idir);

  return OK;

errout_with_vnode:
  free(idir);

errout:
  set_errno(-ret);
  return VFS_ERROR;
}
