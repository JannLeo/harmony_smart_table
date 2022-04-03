/****************************************************************************
 * fs/driver/fs_findblockdriver.c
 *
 *   Copyright (C) 2008, 2017 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in pathname and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of pathname code must retain the above copyright
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

#include "sys/types.h"
#include "sys/mount.h"
#include "errno.h"
#include "fs/driver.h"
#include "string.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: find_blockdriver
 *
 * Description:
 *   Return the inode of the block driver specified by 'pathname'
 *
 * Input Parameters:
 *   pathname   - The full path to the block driver to be located
 *   mountflags - If MS_RDONLY is not set, then driver must support write
 *                operations (see include/sys/mount.h)
 *   ppinode    - Address of the location to return the inode reference
 *
 * Returned Value:
 *   Returns zero on success or a negated errno on failure:
 *
 *   ENOENT  - No block driver of this name is registered
 *   ENOTBLK - The inode associated with the pathname is not a block driver
 *   EACCESS - The MS_RDONLY option was not set but this driver does not
 *             support write access
 *
 ****************************************************************************/
int find_blockdriver(const char *pathname, int mountflags, struct Vnode **vpp)
{
  int ret;
  struct Vnode *vp = NULL;

  /* Sanity checks */

  /* Find the vnode registered with this pathname */
  VnodeHold();
  ret = VnodeLookup(pathname, &vp, V_DUMMY);
  if (ret < 0)
    {
      ret = -EACCES;
      goto errout;
    }

  /* Verify that the vnode is a block driver. */
  if (vp->type != VNODE_TYPE_BLK)
    {
      PRINT_DEBUG("%s is not a block driver\n", pathname);
      ret = -ENOTBLK;
      goto errout;
    }

  /* Make sure that the vnode supports the requested access */

  struct block_operations *i_bops = (struct block_operations *)((struct drv_data *)vp->data)->ops;

  if (i_bops == NULL || i_bops->read == NULL || (i_bops->write == NULL && (mountflags & MS_RDONLY) == 0))
    {
      PRINT_DEBUG("%s does not support requested access\n", pathname);
      ret = -EACCES;
      goto errout;
    }

  *vpp = vp;
  VnodeDrop();
  return OK;

errout:
  VnodeDrop();
  return ret;
}
