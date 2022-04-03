/****************************************************************************
 * drivers/bch/bchlib_setup.c
 *
 *   Copyright (C) 2008-2009, 2011, 2016 Gregory Nutt. All rights reserved.
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
#include <sys/types.h>
#include <sys/mount.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <securec.h>
#include "bch.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bchlib_setup
 *
 * Description:
 *   Setup so that the block driver referenced by 'blkdev' can be accessed
 *   similar to a character device.
 *
 ****************************************************************************/

int bchlib_setup(const char *blkdev, bool readonly, void **handle)
{
  struct bchlib_s *bch;
  struct geometry geo;
  los_part *part;
  int ret;

  DEBUGASSERT(blkdev);

  /* Allocate the BCH state structure */

  bch = (struct bchlib_s *)zalloc(sizeof(struct bchlib_s));
  if (!bch)
    {
      PRINTK("ERROR: Failed to allocate BCH structure\n");
      return -ENOMEM;
    }

  /* Open the block driver */

  ret = open_blockdriver(blkdev, readonly ? MS_RDONLY : 0, &bch->vnode);
  if (ret < 0)
    {
      PRINTK("ERROR: Failed to open driver %s: %d\n", blkdev, -ret);
      goto errout_with_bch;
    }

  struct drv_data *drv = (struct drv_data *)bch->vnode->data;
  struct block_operations *bops = (struct block_operations *)drv->ops;

  DEBUGASSERT(bch->vnode && bops && bops->geometry);

  ret = bops->geometry(bch->vnode, &geo);
  if (ret < 0)
    {
      PRINTK("ERROR: geometry failed: %d\n", -ret);
      goto errout_with_bch;
    }

  if (!geo.geo_available)
    {
      PRINTK("ERROR: geometry failed: %d\n", -ret);
      ret = -ENODEV;
      goto errout_with_bch;
    }

  if (!readonly && (!bops->write || !geo.geo_writeenabled))
    {
      PRINTK("ERROR: write access not supported\n");
      ret = -EACCES;
      goto errout_with_bch;
    }

  /* Save the geometry info and complete initialization of the structure */

  (void)sem_init(&bch->sem, 0, 1);
  bch->nsectors = geo.geo_nsectors;
  bch->sectsize = geo.geo_sectorsize;
  bch->sector   = (size_t)-1;
  bch->readonly = readonly;
  bch->dirty    = false;
  bch->unlinked = false;

  part = los_part_find(bch->vnode);
  if (part != NULL)
    {
      bch->sectstart = part->sector_start;
      bch->nsectors = part->sector_count;
      bch->disk = get_disk(part->disk_id);
    }
  else
    {
      PRINTK("ERROR: los_part_find failed\n");
      ret = -ENODEV;
      goto errout_with_bch;
    }

  /* Allocate the sector I/O buffer */

  bch->buffer = (uint8_t *)malloc(bch->sectsize);
  if (!bch->buffer)
    {
      PRINTK("ERROR: Failed to allocate sector buffer\n");
      ret = -ENOMEM;
      goto errout_with_bch;
    }

  *handle = bch;
  return OK;

errout_with_bch:
  (void)sem_destroy(&bch->sem);
  free(bch);
  return ret;
}
