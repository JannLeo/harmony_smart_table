/****************************************************************************
 * fs/driver/fs_registerdriver.c
 *
 *   Copyright (C) 2007-2009, 2012 Gregory Nutt. All rights reserved.
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
#include "sys/types.h"
#include "errno.h"
#include "fs/driver.h"
#include "vnode.h"
#include "string.h"
#include "path_cache.h"
#include "limits.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: register_driver
 *
 * Description:
 *   Register a character driver vnode the pseudo file system.
 *
 * Input Parameters:
 *   path - The path to the vnode to create
 *   fops - The file operations structure
 *   mode - inmode priviledges (not used)
 *   priv - Private, user data that will be associated with the vnode.
 *
 * Returned Value:
 *   Zero on success (with the vnode point in 'vnode'); A negated errno
 *   value is returned on a failure (all error values returned by
 *   vnode_reserve):
 *
 *   EINVAL - 'path' is invalid for this operation
 *   EEXIST - An vnode already exists at 'path'
 *   ENOMEM - Failed to allocate in-memory resources for the operation
 *
 ****************************************************************************/

int register_driver(const char *path, const struct file_operations_vfs *fops,
                    mode_t mode, void *priv)
{
  struct Vnode *vnode = NULL;
  int ret;

  if (path == NULL || strlen(path) >= PATH_MAX || strncmp("/dev/", path, DEV_PATH_LEN) != 0)
    {
      return -EINVAL;
    }

  VnodeHold();
  ret = VnodeLookup(path, &vnode, 0);
  if (ret == 0)
    {
      VnodeDrop();
      return -EEXIST;
    }

  /* Insert a dummy node -- we need to hold the vnode semaphore because we
   * will have a momentarily bad structure.
   */

  struct drv_data *data = (struct drv_data *)zalloc(sizeof(struct drv_data));

  data->ops = (void *)fops;
  data->mode = mode;
  data->priv = priv;

  ret = VnodeLookup(path, &vnode, V_CREATE | V_DUMMY);
  if (ret == OK)
    {
      /* We have it, now populate it with driver specific information.
       * NOTE that the initial reference count on the new vnode is zero.
       */
      vnode->type = VNODE_TYPE_CHR;
      vnode->data = data;
      vnode->mode = mode;
      vnode->fop = (struct file_operations_vfs *)fops;
    }

  VnodeDrop();

  return ret;
}
