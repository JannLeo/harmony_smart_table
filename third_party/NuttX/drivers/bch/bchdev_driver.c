/****************************************************************************
 * drivers/bch/bchdev_driver.c
 *
 *   Copyright (C) 2008-2009, 2014-2017 Gregory Nutt. All rights reserved.
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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <poll.h>
#include <assert.h>
#include "bch.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int     bch_open(struct file *filep);
static int     bch_close(struct file *filep);
static off_t   bch_seek(struct file *filep, off_t offset, int whence);
static ssize_t bch_read(struct file *filep, char *buffer,
                 size_t buflen);
static ssize_t bch_write(struct file *filep, const char *buffer,
                 size_t buflen);
static int     bch_ioctl(struct file *filep, int cmd,
                 unsigned long arg);
#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
static int     bch_unlink(struct Vnode *vnode);
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/

const struct file_operations_vfs bch_fops =
{
  .open = bch_open,    /* open */
  .close = bch_close,   /* close */
  .read = bch_read,    /* read */
  .write = bch_write,   /* write */
  .seek = bch_seek,    /* seek */
  .ioctl = bch_ioctl,   /* ioctl */
  .unlink = bch_unlink,  /* unlink */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bch_open
 *
 * Description: Open the block device
 *
 ****************************************************************************/

static int bch_open(struct file *filep)
{
  struct Vnode *vnode = filep->f_vnode;
  struct bchlib_s *bch;
  int ret = OK;

  bch = (struct bchlib_s *)((struct drv_data *)vnode->data)->priv;

  /* Increment the reference count */

  bchlib_semtake(bch);
  if (bch->refs == MAX_OPENCNT)
    {
      ret = -EMFILE;
    }
  else
    {
      bch->refs++;
    }

  bchlib_semgive(bch);
  return ret;
}

/****************************************************************************
 * Name: bch_close
 *
 * Description: close the block device
 *
 ****************************************************************************/

static int bch_close(struct file *filep)
{
  struct Vnode *vnode = filep->f_vnode;
  struct bchlib_s *bch;
  int ret = OK;

  bch = (struct bchlib_s *)((struct drv_data *)vnode->data)->priv;

  /* Flush any dirty pages remaining in the cache */

  bchlib_semtake(bch);
  (void)bchlib_flushsector(bch);

  /* Decrement the reference count (I don't use bchlib_decref() because I
   * want the entire close operation to be atomic wrt other driver
   * operations.
   */

  if (bch->refs == 0)
    {
      ret = -EIO;
    }
  else
    {
      bch->refs--;

      /* If the reference count decremented to zero AND if the character
       * driver has been unlinked, then teardown the BCH device now.
       */

      if (bch->refs == 0 && bch->unlinked)
        {
           /* Tear the driver down now. */

           ret = bchlib_teardown((void *)bch);

           /* bchlib_teardown() would only fail if there are outstanding
            * references on the device.  Since we know that is not true, it
            * should not fail at all.
            */

           DEBUGASSERT(ret >= 0);
           if (ret >= 0)
             {
                /* Return without releasing the stale semaphore */

                return OK;
             }
        }
    }

  bchlib_semgive(bch);
  return ret;
}

/****************************************************************************
 * Name: bch_seek
 ****************************************************************************/

static off_t bch_seek(struct file *filep, off_t offset, int whence)
{
  struct Vnode *vnode = filep->f_vnode;
  struct bchlib_s *bch;
  loff_t newpos;
  int ret;

  bch = (struct bchlib_s *)((struct drv_data *)vnode->data)->priv;
  bchlib_semtake(bch);

  /* Determine the new, requested file position */

  switch (whence)
    {
    case SEEK_CUR:
      newpos = filep->f_pos + offset;
      break;

    case SEEK_SET:
      newpos = offset;
      break;

    case SEEK_END:
      newpos = (loff_t)bch->sectsize * bch->nsectors + offset;
      break;

    default:
      /* Return EINVAL if the whence argument is invalid */

      bchlib_semgive(bch);
      return -EINVAL;
    }

  /* Opengroup.org:
   *
   *  "The lseek() function shall allow the file offset to be set beyond the end
   *   of the existing data in the file. If data is later written at this point,
   *   subsequent reads of data in the gap shall return bytes with the value 0
   *   until data is actually written into the gap."
   *
   * We can conform to the first part, but not the second.  But return EINVAL if
   *
   *  "...the resulting file offset would be negative for a regular file, block
   *   special file, or directory."
   */

  if (newpos >= 0)
    {
      filep->f_pos = newpos;
      ret = newpos;
    }
  else
    {
      ret = -EINVAL;
    }

  bchlib_semgive(bch);
  return ret;
}

/****************************************************************************
 * Name: bch_read
 ****************************************************************************/

static ssize_t bch_read(struct file *filep, char *buffer, size_t len)
{
  struct Vnode *vnode = filep->f_vnode;
  struct bchlib_s *bch;
  int ret;

  bch = (struct bchlib_s *)((struct drv_data *)vnode->data)->priv;

  bchlib_semtake(bch);
  ret = bchlib_read(bch, buffer, filep->f_pos, len);
  if (ret > 0)
    {
      filep->f_pos += len;
    }

  bchlib_semgive(bch);
  return ret;
}

/****************************************************************************
 * Name: bch_write
 ****************************************************************************/

static ssize_t bch_write(struct file *filep, const char *buffer, size_t len)
{
  struct Vnode *vnode = filep->f_vnode;
  struct bchlib_s *bch;
  int ret = -EACCES;

  bch = (struct bchlib_s *)((struct drv_data *)vnode->data)->priv;

  if (!bch->readonly)
    {
      bchlib_semtake(bch);
      ret = bchlib_write(bch, buffer, filep->f_pos, len);
      if (ret > 0)
        {
          filep->f_pos += len;
        }

      bchlib_semgive(bch);
    }

  return ret;
}

/****************************************************************************
 * Name: bch_ioctl
 *
 * Description:
 *   Handle IOCTL commands
 *
 ****************************************************************************/

static int bch_ioctl(struct file *filep, int cmd, unsigned long arg)
{
  struct Vnode *vnode = filep->f_vnode;
  struct bchlib_s *bch;
  struct block_operations *bop = NULL;
  int ret = -ENOTTY;

  bch = (struct bchlib_s *)((struct drv_data *)vnode->data)->priv;

  /* Process the call according to the command */

  switch (cmd)
    {
      /* This isa request to get the private data structure */

      case DIOC_GETPRIV:
        {
          struct bchlib_s **bchr =
            (struct bchlib_s **)((uintptr_t)arg);

          bchlib_semtake(bch);
          if (!bchr || bch->refs == MAX_OPENCNT)
            {
              ret   = -EINVAL;
            }
          else
            {
              bch->refs++;
              ret = LOS_CopyFromKernel(bchr, sizeof(char *), &bch, sizeof(char *));
              if (ret)
                {
                  ret = -EFAULT;
                }
              else
                {
                  ret   = OK;
                }
            }

          bchlib_semgive(bch);
        }
        break;

    /* Otherwise, pass the IOCTL command on to the contained block driver. */

    default:
      {
        /* Does the block driver support the ioctl method? */

        los_disk *disk = bch->disk;
        if (disk == NULL)
          {
            ret = -1;
            break;
          }

        if (pthread_mutex_lock(&disk->disk_mutex) != ENOERR)
          {
            PRINT_ERR("%s %d, mutex lock fail!\n", __FUNCTION__, __LINE__);
            return -1;
          }
        if (disk->disk_status == STAT_INUSED)
          {
            bop = (struct block_operations *)(((struct drv_data *)vnode->data)->ops);
            if (bop != NULL && bop->ioctl != NULL)
              {
                ret = bop->ioctl(bch->vnode, cmd, arg);
              }
          }

        if (pthread_mutex_unlock(&disk->disk_mutex) != ENOERR)
          {
            PRINT_ERR("%s %d, mutex unlock fail!\n", __FUNCTION__, __LINE__);
            return -1;
          }
        }
      break;
    }

  return ret;
}

/****************************************************************************
 * Name: bch_unlink
 *
 * Handle unlinking of the BCH device
 *
 ****************************************************************************/

int bch_unlink(struct Vnode *vnode)
{
  struct bchlib_s *bch;
  int ret = OK;

  bch = (struct bchlib_s *)((struct drv_data *)vnode->data)->priv;

  /* Get exclusive access to the BCH device */

  bchlib_semtake(bch);

  /* Indicate that the driver has been unlinked */

  bch->unlinked = true;

  /* If there are no open references to the drvier then teardown the BCH
   * device now.
   */

  if (bch->refs == 0)
    {
      /* Tear the driver down now. */

      ret = bchlib_teardown((void *)bch);

      /* bchlib_teardown() would only fail if there are outstanding
       * references on the device.  Since we know that is not true, it
       * should not fail at all.
       */

      DEBUGASSERT(ret >= 0);
      if (ret >= 0)
        {
          /* Return without releasing the stale semaphore */

          return OK;
        }
    }

  bchlib_semgive(bch);
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
