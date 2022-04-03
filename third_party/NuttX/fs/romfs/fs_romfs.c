/****************************************************************************
 * rm/romfs/fs_romfs.h
 *
 *   Copyright (C) 2008-2009, 2011, 2017-2018 Gregory Nutt. All rights
 *     reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * References: Linux/Documentation/filesystems/romfs.txt
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
#include <sys/statfs.h>
#include <sys/stat.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include "fs_romfs.h"

#include "los_tables.h"
#include "user_copy.h"
#include "los_vm_filemap.h"

#ifdef LOSCFG_FS_ROMFS

/* forward define */
struct VnodeOps g_romfsVops;
struct file_operations_vfs g_romfsFops;

/****************************************************************************
 * Name: romfs_lookup
 ****************************************************************************/

int romfs_lookup(struct Vnode *parentVnode, const char *path, int len, struct Vnode **ppVnode)
{
  int ret;
  struct Vnode *newVnode = NULL;
  struct romfs_dirinfo_s *dirinfo = NULL;
  struct romfs_dirinfo_s *parent_dirinfo = NULL;
  struct romfs_mountpt_s *rm = NULL;
  struct romfs_file_s    *rf = NULL;

  /* Get mountpoint private data from the inode reference from the file
   * structure
   */

  rm = (struct romfs_mountpt_s *)parentVnode->originMount->data;

  /* Check if the mount is still healthy */

  romfs_semtake(rm);
  ret = romfs_checkmount(rm);
  if (ret != OK)
    {
      PRINTK("ERROR: romfs_checkmount failed: %d\n", ret);
      goto errout_with_semaphore;
    }

  parent_dirinfo = (struct romfs_dirinfo_s *)parentVnode->data;
  /* Initialize the directory info structure */

  dirinfo = (struct romfs_dirinfo_s *)zalloc(sizeof(struct romfs_dirinfo_s));
  if (!dirinfo)
    {
      ret = -ENOMEM;
      PRINTK("ERROR: Failed to allocate dirinfo structure, error: %d\n", -ret);
      goto errout_with_semaphore;
    }

  /* Locate the directory entry for this path */

  ret = romfs_searchdir(rm, path, len, parent_dirinfo->rd_dir.fr_firstoffset, dirinfo);
  if (ret < 0)
    {
      goto errout_with_semaphore;
    }

  /* The full path exists -- but is the final component a file
   * or a directory?  Or some other Unix file type that is not
   * appropriate in this contex.
   *
   * REVISIT: This logic should follow hard/soft link file
   * types.  At present, it returns the ENXIO.
   */

  if (!IS_DIRECTORY(dirinfo->rd_next) && !IS_FILE(dirinfo->rd_next))
    {
      /* ENXIO indicates "The named file is a character special or
       * block special file, and the device associated with this
       * special file does not exist."
       *
       * Here we also return ENXIO if the file is not a directory
       * or a regular file.
       */

      ret = -ENXIO;
      PRINTK("ERROR: '%s' is a special file\n", path);
      goto errout_with_semaphore;
    }

  if (IS_DIRECTORY(dirinfo->rd_next))
    {
      ret = VnodeAlloc(&g_romfsVops, &newVnode);
      if (ret != 0)
        {
          PRINTK("%s-%d: can't alloc vnode, error: %d\n", __FUNCTION__, __LINE__, ret);
          goto errout_with_semaphore;
        }
      newVnode->type   = VNODE_TYPE_DIR;
      newVnode->parent = parentVnode;
      newVnode->fop    = &g_romfsFops;
      newVnode->data   = dirinfo;
      newVnode->originMount = parentVnode->originMount;
      newVnode->uid    = parentVnode->uid;
      newVnode->gid    = parentVnode->gid;
      newVnode->mode   = parentVnode->mode | (S_IFDIR | S_IROTH | S_IXOTH | S_IRGRP | S_IXGRP |
                     S_IRUSR | S_IXUSR);
    }
  else
    {
      rf = (struct romfs_file_s *)zalloc(sizeof(struct romfs_file_s));
      if (!rf)
        {
          ret = -ENOMEM;
          PRINTK("ERROR: Failed to allocate private data: %d\n", ret);
          goto errout_with_semaphore;
        }

      /* Initialize the file private data (only need to initialize
      * non-zero elements)
      */

      rf->rf_size = dirinfo->rd_size;
      rf->rf_type = (uint8_t)(dirinfo->rd_next & RFNEXT_ALLMODEMASK);

      /* Get the start of the file data */

      ret = romfs_datastart(rm, dirinfo->rd_dir.fr_curroffset,
                            &rf->rf_startoffset);
      if (ret < 0)
        {
          PRINTK("ERROR: Failed to locate start of file data: %d\n", ret);
          goto errout_with_mem;
        }

      /* Then insert the new instance into the mountpoint structure.
      * It needs to be there (1) to handle error conditions that effect
      * all files, and (2) to inform the umount logic that we are busy
      * (but a simple reference count could have done that).
      */

      rf->rf_next = rm->rm_head;
      rm->rm_head = rf->rf_next;

      ret = VnodeAlloc(&g_romfsVops, &newVnode);
      if (ret != 0)
        {
          PRINTK("%s-%d: can't alloc vnode, error: %d\n", __FUNCTION__, __LINE__, ret);
          goto errout_with_mem;
        }

      newVnode->originMount = parentVnode->originMount;
      newVnode->type   = VNODE_TYPE_REG;
      newVnode->parent = parentVnode;
      newVnode->fop    = &g_romfsFops;
      newVnode->data   = rf;
      newVnode->uid    = parentVnode->uid;
      newVnode->gid    = parentVnode->gid;
      newVnode->mode   = S_IFREG | S_IROTH | S_IXOTH | S_IRGRP | S_IXGRP | S_IRUSR | S_IXUSR;

      free(dirinfo);
    }

  *ppVnode = newVnode;

  romfs_semgive(rm);
  return OK;

  /* Error exits */

errout_with_mem:
  free(rf);

errout_with_semaphore:
  romfs_semgive(rm);
  return ret;
}

/****************************************************************************
 * Name: romfs_close
 ****************************************************************************/

int romfs_close(struct file *filep)
{
  return 0;
}

int romfs_reclaim(struct Vnode *vp)
{
  struct romfs_file_s *rf = vp->data;
  if (rf->rf_buffer)
    {
      free(rf->rf_buffer);
    }

  /* Then free the file structure itself. */

  free(rf);
  vp->data = NULL;

  return 0;
}

/****************************************************************************
 * Name: romfs_read
 ****************************************************************************/

static ssize_t romfs_read(struct file *filep, char *buffer,
                          size_t buflen)
{
  struct romfs_mountpt_s *rm = NULL;
  struct romfs_file_s    *rf = NULL;
  uint32_t                    offset;
  size_t                      bytesleft;
  int                         ret;

  /* Recover our private data from the struct file instance */

  rf = (struct romfs_file_s *)filep->f_vnode->data;
  rm = (struct romfs_mountpt_s *)filep->f_vnode->originMount->data;

  /* Make sure that the mount is still healthy */

  romfs_semtake(rm);
  ret = romfs_checkmount(rm);
  if (ret != OK)
    {
      PRINTK("ERROR: romfs_checkmount failed: %d\n", ret);
      goto errout_with_semaphore;
    }

  /* Get the number of bytes left in the file */

  bytesleft = rf->rf_size - filep->f_pos;

  /* Truncate read count so that it does not exceed the number
   * of bytes left in the file.
   */

  if (buflen > bytesleft)
    {
      buflen = bytesleft;
    }

  offset = rf->rf_startoffset + filep->f_pos;
  LOS_CopyFromKernel(buffer, buflen, &rm->rm_buffer[offset], buflen);
  filep->f_pos += buflen;

  romfs_semgive(rm);
  return buflen;

errout_with_semaphore:
  romfs_semgive(rm);
  return ret;
}

/****************************************************************************
 * Name: romfs_seek
 ****************************************************************************/

off_t romfs_seek(struct file *filep, off_t offset, int whence)
{
  struct romfs_file_s    *rf = NULL;
  loff_t                 position;

  /* Recover our private data from the struct file instance */

  rf = (struct romfs_file_s *)filep->f_vnode->data;
  position = filep->f_pos;

  /* Map the offset according to the whence option */

  switch (whence)
    {
    case SEEK_SET: /* The offset is set to offset bytes. */
        position = offset;
        break;

    case SEEK_CUR: /* The offset is set to its current location plus
                    * offset bytes. */

        position += offset;
        break;

    case SEEK_END: /* The offset is set to the size of the file plus
                    * offset bytes. */

        position = offset + rf->rf_size;
        break;

    default:
        PRINTK("ERROR: Whence is invalid: %d\n", whence);
        return -EINVAL;
    }

  /* Limit positions to the end of the file. */

  if (position > rf->rf_size)
    {
      /* Otherwise, the position is limited to the file size */

      position = rf->rf_size;
    }

  if (position < 0)
    {
      return -EINVAL;
    }
  return position;
}

/****************************************************************************
 * Name: romfs_seek64
 ****************************************************************************/

loff_t romfs_seek64(struct file *filep, loff_t offset, int whence)
{
  return (loff_t)romfs_seek(filep, (off_t)offset, whence);
}

/****************************************************************************
 * Name: romfs_ioctl
 ****************************************************************************/

static int romfs_ioctl(struct file *filep, int cmd, unsigned long arg)
{
  return -ENOSYS;
}

/****************************************************************************
 * Name: romfs_opendir
 *
 * Description:
 *   Open a directory for read access
 *
 ****************************************************************************/

int romfs_opendir(struct Vnode *vp, struct fs_dirent_s *dir)
{
  int ret;
  struct romfs_mountpt_s *rm = NULL;
  struct romfs_dirinfo_s *dirinfo = NULL;
  struct fs_romfsdir_s   *ptr = NULL;

  /* Recover our private data from the vnode instance */

  rm = (struct romfs_mountpt_s *)vp->originMount->data;

  /* Make sure that the mount is still healthy */

  romfs_semtake(rm);
  ret = romfs_checkmount(rm);
  if (ret != OK)
    {
      PRINTK("ERROR: romfs_checkmount failed: %d\n", ret);
      goto errout_with_semaphore;
    }

  ptr = (struct fs_romfsdir_s *)zalloc(sizeof(struct fs_romfsdir_s));
  if (ptr == NULL)
    {
      ret = -ENOMEM;
      PRINTK("ERROR: zalloc error for romfsdir. error: %d\n", ret);
      goto errout_with_semaphore;
    }

  dirinfo = (struct romfs_dirinfo_s *)vp->data;
  memcpy(ptr, &dirinfo->rd_dir, sizeof(struct fs_romfsdir_s));
  dir->u.fs_dir = (fs_dir_s *)ptr;

  romfs_semgive(rm);
  return OK;

errout_with_semaphore:
  romfs_semgive(rm);
  return ret;
}

/****************************************************************************
 * Name: romfs_closedir
 *
 * Description:
 *   Close a directory for read access
 *
 ****************************************************************************/

int romfs_closedir(struct Vnode *vp, struct fs_dirent_s *dir)
{
  struct fs_romfsdir_s *ptr = NULL;

  ptr = (struct fs_romfsdir_s *)dir->u.fs_dir;
  if (ptr == NULL)
    {
      PRINTK("ERROR: romfs_closedir failed, fs_romfsdir_s is NULL.\n");
      return -EINVAL;
    }
  free(ptr);
  dir->u.fs_dir = NULL;
  return 0;
}

/****************************************************************************
 * Name: romfs_readdir
 *
 * Description: Read the next directory entry
 *
 ****************************************************************************/

static int romfs_readdir(struct Vnode *vp, struct fs_dirent_s *dir)
{
  struct romfs_mountpt_s *rm = NULL;
  struct fs_romfsdir_s   *romfsdir = NULL;
  uint32_t                    linkoffset;
  uint32_t                    next;
  uint32_t                    info;
  uint32_t                    size;
  int                         ret;
  int                         i = 0;

  /* Recover our private data from the vnode instance */

  rm = (struct romfs_mountpt_s *)vp->originMount->data;

  /* Make sure that the mount is still healthy */

  romfs_semtake(rm);
  ret = romfs_checkmount(rm);
  if (ret != OK)
    {
      PRINTK("ERROR: omfs_checkmount failed: %d\n", ret);
      goto errout_with_semaphore;
    }

  romfsdir = (struct fs_romfsdir_s *)dir->u.fs_dir;
  /* Loop, skipping over unsupported items in the file system */

  while (i < dir->read_cnt)
    {
      /* Have we reached the end of the directory */

      if (!romfsdir->fr_curroffset)
        {
          /* We signal the end of the directory by returning the
           * special error -ENOENT
           */

          ret = -ENOENT;
          break;
        }

      /* Parse the directory entry */

      ret = romfs_parsedirentry(rm, romfsdir->fr_curroffset, &linkoffset,
                                &next, &info, &size);
      if (ret < 0)
        {
          PRINTK("ERROR: romfs_parsedirentry failed: %d\n", ret);
          goto errout_with_semaphore;
        }

      /* Save the filename */

      ret = romfs_parsefilename(rm, (uint32_t)romfsdir->fr_curroffset, dir->fd_dir[i].d_name);
      if (ret < 0)
        {
          PRINTK("ERROR: romfs_parsefilename failed: %d\n", ret);
          goto errout_with_semaphore;
        }

      /* Set up the next directory entry offset */

      romfsdir->fr_curroffset = (off_t)next & RFNEXT_OFFSETMASK;

      if (!strcmp(dir->fd_dir[i].d_name, ".") || !strcmp(dir->fd_dir[i].d_name, ".."))
        {
          continue;
        }

      /* Check the file type */

      if (IS_DIRECTORY(next))
        {
          dir->fd_dir[i].d_type = DT_DIR;
        }
      else if (IS_FILE(next))
        {
          dir->fd_dir[i].d_type = DT_REG;
        }

      dir->fd_position++;
      dir->fd_dir[i].d_off = dir->fd_position;
      dir->fd_dir[i].d_reclen = (uint16_t)sizeof(struct dirent);

      i++;
    }
  romfs_semgive(rm);
  return i;

errout_with_semaphore:
  romfs_semgive(rm);
  return ret;
}

/****************************************************************************
 * Name: romfs_rewindir
 *
 * Description: Reset directory read to the first entry
 *
 ****************************************************************************/

static int romfs_rewinddir(struct Vnode *vp, struct fs_dirent_s *dir)
{
  int ret;
  struct romfs_mountpt_s *rm = NULL;
  struct fs_romfsdir_s   *romfsdir = NULL;

  /* Recover our private data from the vnode instance */

  rm = (struct romfs_mountpt_s *)vp->originMount->data;

  /* Make sure that the mount is still healthy */

  romfs_semtake(rm);
  ret = romfs_checkmount(rm);
  if (ret == OK)
    {
      romfsdir = (struct fs_romfsdir_s *)dir->u.fs_dir;
      romfsdir->fr_curroffset = romfsdir->fr_firstoffset;
    }

  romfs_semgive(rm);
  return ret;
}

/****************************************************************************
 * Name: romfs_bind
 *
 * Description: This implements a portion of the mount operation. This
 *  function allocates and initializes the mountpoint private data and
 *  binds the blockdriver inode to the filesystem private data.  The final
 *  binding of the private data (containing the blockdriver) to the
 *  mountpoint is performed by mount().
 *
 ****************************************************************************/

int romfs_bind(struct Mount *mnt, struct Vnode *blkDriver, const void *data)
{
  struct romfs_mountpt_s *rm = NULL;
  struct romfs_dirinfo_s *dirinfo = NULL;
  struct Vnode *pv = NULL;
  int ret;

  rm = (struct romfs_mountpt_s *)zalloc(sizeof(struct romfs_mountpt_s));
  if (!rm)
    {
      PRINTK("ERROR: Failed to allocate mountpoint structure, error: %d\n", -ENOMEM);
      return -ENOMEM;
    }

  /* Initialize the allocated mountpt state structure.  The filesystem is
   * responsible for one reference ont the blkdriver inode and does not
   * have to addref() here (but does have to release in ubind().
   */

  (void)sem_init(&rm->rm_sem, 0, 0);   /* Initialize the semaphore that controls access */

  /* Get the hardware configuration and setup buffering appropriately */

  ret = romfs_hwconfigure(rm);
  if (ret)
    {
      PRINTK("ERROR: romfs_hwconfigure failed: %d\n", ret);
      goto errout_with_sem;
    }

  /* Then complete the mount by getting the ROMFS configuratrion from
   * the ROMF header
   */

  ret = romfs_fsconfigure(rm);
  if (ret < 0)
    {
      PRINTK("ERROR: romfs_fsconfigure failed: %d\n", ret);
      goto errout_with_buffer;
    }

  dirinfo = (struct romfs_dirinfo_s *)zalloc(sizeof(struct romfs_dirinfo_s));
  if (!dirinfo)
    {
      PRINTK("ERROR: Failed to allocate dirinfo structure, error: %d\n", -ENOMEM);
      goto errout_with_buffer;
    }

  dirinfo->rd_dir.fr_firstoffset = rm->rm_rootoffset;
  dirinfo->rd_dir.fr_curroffset  = rm->rm_rootoffset;
  dirinfo->rd_next               = RFNEXT_DIRECTORY;
  dirinfo->rd_size               = 0;

  /* Mounted! */

  ret = VnodeAlloc(&g_romfsVops, &pv);
  if (ret)
    {
      goto errout_with_dirinfo;
    }

  pv->type = VNODE_TYPE_DIR;
  pv->data = dirinfo;
  pv->originMount = mnt;
  pv->fop = &g_romfsFops;
  pv->uid = mnt->vnodeBeCovered->uid;
  pv->gid = mnt->vnodeBeCovered->gid;
  pv->mode = S_IFDIR | S_IROTH | S_IXOTH | S_IRGRP | S_IXGRP | S_IRUSR | S_IXUSR;
  mnt->data = rm;
  mnt->vnodeCovered = pv;

  romfs_semgive(rm);
  return OK;

errout_with_dirinfo:
  free(dirinfo);

errout_with_buffer:
  free(rm->rm_buffer);

errout_with_sem:
  (void)sem_destroy(&rm->rm_sem);
  free(rm);
  return ret;
}

/****************************************************************************
 * Name: romfs_unbind
 *
 * Description: This implements the filesystem portion of the umount
 *   operation.
 *
 ****************************************************************************/

static int romfs_unbind(struct Mount *mnt, struct Vnode **blkDriver)
{
  struct romfs_mountpt_s *rm = (struct romfs_mountpt_s *)mnt->data;

  if (!rm)
    {
      return -EINVAL;
    }

  /* VFS can assure the mountpoint can be umounted. */

  romfs_semtake(rm);
  /* Release the mountpoint private data */

  if (rm->rm_buffer)
    {
      free(rm->rm_buffer);
    }

  (void)sem_destroy(&rm->rm_sem);
  free(rm);
  return OK;
}

/****************************************************************************
 * Name: romfs_statfs
 *
 * Description: Return filesystem statistics
 *
 ****************************************************************************/

static int romfs_statfs(struct Mount *mnt, struct statfs *buf)
{
  struct romfs_mountpt_s *rm;
  int ret;

  rm = (struct romfs_mountpt_s *)mnt->data;

  /* Check if the mount is still healthy */

  romfs_semtake(rm);
  ret = romfs_checkmount(rm);
  if (ret < 0)
    {
      PRINTK("ERROR: romfs_checkmount failed: %d\n", ret);
      goto errout_with_semaphore;
    }

  /* Fill in the statfs info */

  memset(buf, 0, sizeof(struct statfs));
  buf->f_type    = ROMFS_MAGIC;

  /* We will claim that the optimal transfer size is the size of one sector */

  buf->f_bsize   = rm->rm_hwsectorsize;

  /* Everything else follows in units of sectors */

  buf->f_blocks  = SEC_NSECTORS(rm, rm->rm_volsize + SEC_NDXMASK(rm));
  buf->f_bfree   = 0;
  buf->f_bavail  = rm->rm_volsize;
  buf->f_namelen = NAME_MAX;

  romfs_semgive(rm);
  return OK;

errout_with_semaphore:
  romfs_semgive(rm);
  return ret;
}

/****************************************************************************
 * Name: romfs_stat
 *
 * Description: Return information about a file or directory
 *
 ****************************************************************************/

static int romfs_stat(struct Vnode *vp, struct stat *buf)
{
  struct romfs_mountpt_s *rm = NULL;
  struct romfs_dirinfo_s *dirinfo = NULL;
  struct romfs_file_s    *rf = NULL;
  int ret;

  rm = (struct romfs_mountpt_s *)vp->originMount->data;

  /* Check if the mount is still healthy */

  romfs_semtake(rm);
  ret = romfs_checkmount(rm);
  if (ret != OK)
    {
      PRINTK("ERROR: romfs_checkmount failed: %d\n", ret);
      goto errout_with_semaphore;
    }

  if (vp->type == VNODE_TYPE_DIR)
    {
      dirinfo = (struct romfs_dirinfo_s *)vp->data;
      buf->st_mode = vp->mode;
      buf->st_size = dirinfo->rd_size;
    }
  else if (vp->type == VNODE_TYPE_REG)
    {
      rf = (struct romfs_file_s *)vp->data;
      buf->st_mode = vp->mode;
      buf->st_size = rf->rf_size;
    }
  else
    {
      PRINTK("ERROR: Unsupported file type: %d\n", vp->type);
      ret = -EINVAL;
      goto errout_with_semaphore;
    }

  buf->st_blksize = rm->rm_hwsectorsize;
  buf->st_dev     = 0;
  buf->st_ino     = 0;
  buf->st_nlink   = 0;
  buf->st_uid     = vp->uid;
  buf->st_gid     = vp->gid;
  buf->st_atime   = 0;
  buf->st_mtime   = 0;
  buf->st_ctime   = 0;

errout_with_semaphore:
  romfs_semgive(rm);
  return ret;
}

const struct MountOps romfs_operations =
{
  .Mount = romfs_bind,
  .Unmount = romfs_unbind,
  .Statfs = romfs_statfs,
};

struct VnodeOps g_romfsVops =
{
  .Lookup = romfs_lookup,
  .Create = NULL,
  .Rename = NULL,
  .Mkdir = NULL,
  .Getattr = romfs_stat,
  .Opendir = romfs_opendir,
  .Readdir = romfs_readdir,
  .Closedir = romfs_closedir,
  .Rewinddir = romfs_rewinddir,
  .Unlink = NULL,
  .Rmdir = NULL,
  .Chattr = NULL,
  .Reclaim = romfs_reclaim,
  .Truncate = NULL,
  .Truncate64 = NULL,
};

struct file_operations_vfs g_romfsFops =
{
  .read = romfs_read,
  .write = NULL,
  .mmap = OsVfsFileMmap,
  .seek = romfs_seek,
  .ioctl = romfs_ioctl,
  .close = romfs_close,
  .fsync = NULL,
};

FSMAP_ENTRY(romfs_fsmap, "romfs", romfs_operations, FALSE, FALSE);

#endif
