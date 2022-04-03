/****************************************************************************
 * fs/tmpfs/fs_tmpfs.c
 *
 *   Copyright (C) 2015, 2017-2018 Gregory Nutt. All rights reserved.
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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/spinlock.h>
#include <sys/statfs.h>
#include "fs/dirent_fs.h"
#include "fs/mount.h"
#include "fs/file.h"
#include "fs/fs.h"
#include "los_tables.h"
#include "fs_tmpfs.h"
#include "los_vm_filemap.h"
#include "user_copy.h"

#ifdef LOSCFG_FS_RAMFS

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*
#if CONFIG_FS_TMPFS_DIRECTORY_FREEGUARD <= CONFIG_FS_TMPFS_DIRECTORY_ALLOCGUARD
#  warning CONFIG_FS_TMPFS_DIRECTORY_FREEGUARD needs to be > ALLOCGUARD
#endif

#if CONFIG_FS_TMPFS_FILE_FREEGUARD <= CONFIG_FS_TMPFS_FILE_ALLOCGUARD
#  warning CONFIG_FS_TMPFS_FILE_FREEGUARD needs to be > ALLOCGUARD
#endif
*/
#define tmpfs_lock_file(tfo) \
           (tmpfs_lock_object((struct tmpfs_object_s *)tfo))
#define tmpfs_lock_directory(tdo) \
           (tmpfs_lock_object((struct tmpfs_object_s *)tdo))
#define tmpfs_unlock_file(tfo) \
           (tmpfs_unlock_object((struct tmpfs_object_s *)tfo))
#define tmpfs_unlock_directory(tdo) \
           (tmpfs_unlock_object((struct tmpfs_object_s *)tdo))

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* TMPFS helpers */

static void tmpfs_lock_reentrant(struct tmpfs_sem_s *sem);
static void tmpfs_lock(struct tmpfs_s *fs);
static void tmpfs_unlock_reentrant(struct tmpfs_sem_s *sem);
static void tmpfs_unlock(struct tmpfs_s *fs);
static void tmpfs_lock_object(struct tmpfs_object_s *to);
static void tmpfs_unlock_object(struct tmpfs_object_s *to);
static void tmpfs_release_lockedobject(struct tmpfs_object_s *to);
static void tmpfs_release_lockedfile(struct tmpfs_file_s *tfo);
static struct tmpfs_dirent_s *tmpfs_find_dirent(struct tmpfs_directory_s *tdo,
              const char *name);
static int  tmpfs_remove_dirent(struct tmpfs_directory_s *tdo,
              struct tmpfs_object_s *to);
static int  tmpfs_add_dirent(struct tmpfs_directory_s **tdo,
              struct tmpfs_object_s *to, const char *name);
static struct tmpfs_file_s *tmpfs_alloc_file(void);
static int tmpfs_create_file(struct tmpfs_s *fs,
                             const char *relpath,
                             struct tmpfs_directory_s *parent_input,
                             struct tmpfs_file_s **tfo);

static struct tmpfs_directory_s *tmpfs_alloc_directory(void);
static int tmpfs_create_directory(struct tmpfs_s *fs,
                                  const char *relpath,
                                  struct tmpfs_directory_s *parent,
                                  struct tmpfs_directory_s **tdo);

static int  tmpfs_find_object(struct tmpfs_s *fs,
              const char *relpath, struct tmpfs_object_s **object,
              struct tmpfs_directory_s **parent);
static int  tmpfs_find_file(struct tmpfs_s *fs,
              const char *relpath,
              struct tmpfs_file_s **tfo,
              struct tmpfs_directory_s **parent);
static int  tmpfs_find_directory(struct tmpfs_s *fs,
              const char *relpath,
              struct tmpfs_directory_s **tdo,
              struct tmpfs_directory_s **parent);

/* File system operations */

int tmpfs_close(struct file *filep);
off_t tmpfs_seek(struct file *filep, off_t offset, int whence);
int tmpfs_ioctl(struct file *filep, int cmd, unsigned long arg);
int tmpfs_closedir(struct Vnode *node, struct fs_dirent_s *dir);
int tmpfs_rewinddir(struct Vnode *vp, struct fs_dirent_s *dir);
int tmpfs_truncate(struct Vnode *vp, off_t len);

int  tmpfs_mount(struct Mount *mnt, struct Vnode *device, const void *data);
int  tmpfs_unmount(struct Mount *mnt, struct Vnode **blkdriver);

int tmpfs_lookup(struct Vnode *parent, const char *name, int len, struct Vnode **vpp);
ssize_t tmpfs_write(struct file *filep, const char *buffer, size_t buflen);
ssize_t tmpfs_read(struct file *filep, char *buffer, size_t buflen);
int tmpfs_stat(struct Vnode *vp, struct stat *st);
int tmpfs_opendir(struct Vnode *vp, struct fs_dirent_s *dir);
int tmpfs_readdir(struct Vnode *vp, struct fs_dirent_s *dir);
int tmpfs_rename(struct Vnode *oldVnode, struct Vnode *newParent, const char *oldname, const char *newname);
int tmpfs_mkdir(struct Vnode *parent, const char *relpath, mode_t mode, struct Vnode **vpp);
int tmpfs_create(struct Vnode *dvp, const char *path, int mode, struct Vnode **vpp);
int tmpfs_unlink(struct Vnode *parent, struct Vnode *node, const char *relpath);
int tmpfs_rmdir(struct Vnode *parent, struct Vnode *target, const char *dirname);
int tmpfs_reclaim(struct Vnode *vp);

int tmpfs_statfs(struct Mount *mp, struct statfs *sbp);

static void tmpfs_stat_common(struct tmpfs_object_s *to,
                              struct stat *buf);

/****************************************************************************
 * Public Data
 ****************************************************************************/
const struct MountOps tmpfs_operations = {
    .Mount = tmpfs_mount,
    .Unmount = tmpfs_unmount,
    .Statfs = tmpfs_statfs,
};

struct VnodeOps tmpfs_vops = {
    .Lookup = tmpfs_lookup,
    .Getattr = tmpfs_stat,
    .Opendir = tmpfs_opendir,
    .Readdir = tmpfs_readdir,
    .Rename = tmpfs_rename,
    .Mkdir = tmpfs_mkdir,
    .Create = tmpfs_create,
    .Unlink =  tmpfs_unlink,
    .Rmdir = tmpfs_rmdir,
    .Reclaim = tmpfs_reclaim,
    .Closedir = tmpfs_closedir,
    .Close = NULL,
    .Rewinddir = tmpfs_rewinddir,
    .Truncate = tmpfs_truncate,
};

struct file_operations_vfs tmpfs_fops = {
    .seek = tmpfs_seek,
    .write = tmpfs_write,
    .read = tmpfs_read,
    .ioctl = tmpfs_ioctl,
    .mmap = OsVfsFileMmap,
    .close = tmpfs_close,
    .fsync = NULL,
};

static struct tmpfs_s tmpfs_superblock = {0};

/****************************************************************************
 * Name: tmpfs_timestamp
 ****************************************************************************/

static time_t tmpfs_timestamp(void)
{
  struct timeval tv;

  (void)gettimeofday(&tv, (struct timezone *)NULL);

  return (time_t)(tv.tv_sec);
}

/****************************************************************************
 * Name: tmpfs_lock_reentrant
 ****************************************************************************/

static void tmpfs_lock_reentrant(struct tmpfs_sem_s *sem)
{
  pid_t me;

  /* Do we already hold the semaphore? */

  me = getpid();
  if (me == sem->ts_holder)
    {
      /* Yes... just increment the count */

      sem->ts_count++;
      DEBUGASSERT(sem->ts_count > 0);
    }

  /* Take the semaphore (perhaps waiting) */

  else
    {
      while (sem_wait(&sem->ts_sem) != 0)
        {
          /* The only case that an error should occur here is if the wait
           * was awakened by a signal.
           */

          DEBUGASSERT(get_errno() == EINTR);
        }

      /* No we hold the semaphore */

      sem->ts_holder = me;
      sem->ts_count  = 1;
    }
}

/****************************************************************************
 * Name: tmpfs_lock
 ****************************************************************************/

static void tmpfs_lock(struct tmpfs_s *fs)
{
  tmpfs_lock_reentrant(&fs->tfs_exclsem);
}

/****************************************************************************
 * Name: tmpfs_lock_object
 ****************************************************************************/

static void tmpfs_lock_object(struct tmpfs_object_s *to)
{
  tmpfs_lock_reentrant(&to->to_exclsem);
}

/****************************************************************************
 * Name: tmpfs_unlock_reentrant
 ****************************************************************************/

static void tmpfs_unlock_reentrant(struct tmpfs_sem_s *sem)
{
  DEBUGASSERT(sem->ts_holder == getpid());

  /* Is this our last count on the semaphore? */

  if (sem->ts_count > 1)
    {
      /* No.. just decrement the count */

      sem->ts_count--;
    }

  /* Yes.. then we can really release the semaphore */

  else
    {
      sem->ts_holder = TMPFS_NO_HOLDER;
      sem->ts_count  = 0;
      sem_post(&sem->ts_sem);
    }
}

/****************************************************************************
 * Name: tmpfs_unlock
 ****************************************************************************/

static void tmpfs_unlock(struct tmpfs_s *fs)
{
  tmpfs_unlock_reentrant(&fs->tfs_exclsem);
}

/****************************************************************************
 * Name: tmpfs_unlock_object
 ****************************************************************************/

static void tmpfs_unlock_object(struct tmpfs_object_s *to)
{
  tmpfs_unlock_reentrant(&to->to_exclsem);
}

/****************************************************************************
 * Name: tmpfs_release_lockedobject
 ****************************************************************************/

static void tmpfs_release_lockedobject(struct tmpfs_object_s *to)
{
  DEBUGASSERT(to && to->to_refs > 0);

  /* Is this a file object? */

  if (to->to_type == TMPFS_REGULAR)
    {
      tmpfs_release_lockedfile((struct tmpfs_file_s *)to);
    }
  else
    {
      if(to->to_refs > 0)
        {
          to->to_refs--;
        }
      tmpfs_unlock_object(to);
    }
}

/****************************************************************************
 * Name: tmpfs_release_lockedfile
 ****************************************************************************/

static void tmpfs_release_lockedfile(struct tmpfs_file_s *tfo)
{
  DEBUGASSERT(tfo && tfo->tfo_refs > 0);

  /* If there are no longer any references to the file and the file has been
   * unlinked from its parent directory, then free the file object now.
   */

  if (tfo->tfo_refs == 1 && (tfo->tfo_flags & TFO_FLAG_UNLINKED) != 0)
    {
      sem_destroy(&tfo->tfo_exclsem.ts_sem);
      kmm_free(tfo->tfo_data);
      kmm_free(tfo);
    }

  /* Otherwise, just decrement the reference count on the file object */

  else
    {
      if(tfo->tfo_refs > 0)
        {
          tfo->tfo_refs--;
        }
      tmpfs_unlock_file(tfo);
    }
}

/****************************************************************************
 * Name: tmpfs_find_dirent
 ****************************************************************************/

static struct tmpfs_dirent_s *tmpfs_find_dirent(struct tmpfs_directory_s *tdo,
                                                const char *name)
{
  LOS_DL_LIST *node;
  struct tmpfs_dirent_s *tde;

  /* Search the list of directory entries for a match */

  for (node = tdo->tdo_entry.pstNext; node != &tdo->tdo_entry; node = node->pstNext)
    {
      tde = (struct tmpfs_dirent_s *)node;
      if (tde->tde_inuse == true && strcmp(tde->tde_name, name) == 0)
        {
          return tde;
        }
    }

  /* Return NULL if not found */

  return NULL;
}

/****************************************************************************
 * Name: tmpfs_remove_dirent
 ****************************************************************************/

static int tmpfs_remove_dirent(struct tmpfs_directory_s *tdo,
                               struct tmpfs_object_s *to)
{
  struct tmpfs_dirent_s *tde;

  /* Search the list of directory entries for a match */

  tde = to->to_dirent;
  if (tde == NULL)
    {
      return -ENONET;
    }

  /* Free the object name */

  if (tde->tde_name != NULL)
    {
      kmm_free(tde->tde_name);
      tde->tde_name = NULL;
    }

  if (tdo->tdo_count == 0)
    {
      LOS_ListDelete(&tde->tde_node);
      kmm_free(tde);
    }
  else
    {
      tde->tde_inuse = false;
      tde->tde_object = NULL;
    }
  if(tdo->tdo_nentries > 0)
    {
      tdo->tdo_nentries--;
    }
  return OK;
}

/****************************************************************************
 * Name: tmpfs_add_dirent
 ****************************************************************************/

static int tmpfs_add_dirent(struct tmpfs_directory_s **tdo,
                            struct tmpfs_object_s *to,
                            const char *name)
{
  struct tmpfs_directory_s *parent;
  struct tmpfs_dirent_s *tde;
  char *newname;

  /* Copy the name string so that it will persist as long as the
   * directory entry.
   */

  newname = strdup(name);
  if (newname == NULL)
    {
      return -ENOSPC;
    }

  tde = (struct tmpfs_dirent_s *)malloc(sizeof(struct tmpfs_dirent_s));
  if (tde == NULL)
    {
      free(newname);
      return -ENOSPC;
    }

  tde->tde_object = to;
  tde->tde_name   = newname;
  tde->tde_inuse  = true;
  to->to_dirent   = tde;

  /* Save the new object info in the new directory entry */

  parent = *tdo;
  LOS_ListTailInsert(&parent->tdo_entry, &tde->tde_node);
  parent->tdo_nentries++;

  /* Update directory times */

  parent->tdo_ctime = parent->tdo_mtime = tmpfs_timestamp();

  return OK;
}

/****************************************************************************
 * Name: tmpfs_alloc_file
 ****************************************************************************/

static struct tmpfs_file_s *tmpfs_alloc_file(void)
{
  struct tmpfs_file_s *tfo;
  size_t allocsize;

  /* Create a new zero length file object */

  allocsize = sizeof(struct tmpfs_file_s);
  tfo = (struct tmpfs_file_s *)kmm_malloc(allocsize);
  if (tfo == NULL)
    {
      return NULL;
    }

  /* Initialize the new file object.  NOTE that the initial state is
   * locked with one reference count.
   */

  tfo->tfo_atime = tmpfs_timestamp();
  tfo->tfo_mtime = tfo->tfo_atime;
  tfo->tfo_ctime = tfo->tfo_atime;
  tfo->tfo_type  = TMPFS_REGULAR;
  tfo->tfo_refs  = 1;
  tfo->tfo_flags = 0;
  tfo->tfo_size  = 0;
  tfo->tfo_data  = NULL;

  tfo->tfo_exclsem.ts_holder = getpid();
  tfo->tfo_exclsem.ts_count  = 1;
  if (sem_init(&tfo->tfo_exclsem.ts_sem, 0, 0) != 0)
    {
      PRINT_ERR("%s %d, sem_init failed!\n", __FUNCTION__, __LINE__);
      kmm_free(tfo);
      return NULL;
    }

  return tfo;
}

/****************************************************************************
 * Name: tmpfs_create_file
 ****************************************************************************/

static int tmpfs_create_file(struct tmpfs_s *fs,
                             const char *relpath,
                             struct tmpfs_directory_s *parent_input,
                             struct tmpfs_file_s **tfo)
{
  struct tmpfs_directory_s *parent;
  struct tmpfs_file_s *newtfo;
  struct tmpfs_dirent_s *tde;
  char *copy;
  int ret;

  /* Duplicate the path variable so that we can modify it */

  copy = strdup(relpath);
  if (copy == NULL)
    {
      return -ENOSPC;
    }

  /* Separate the path into the file name and the path to the parent
   * directory.
   */
  if (parent_input == NULL)
    {
      /* No subdirectories... use the root directory */
      parent = (struct tmpfs_directory_s *)fs->tfs_root.tde_object;

      /* Lock the root directory to emulate the behavior of tmpfs_find_directory() */

      tmpfs_lock_directory(parent);
      parent->tdo_refs++;
    }
  else
    {
      parent = parent_input;
    }
  if (parent == NULL)
    {
      ret = -EEXIST;
      goto errout_with_copy;
    }

  /* Verify that no object of this name already exists in the directory */
  tde = tmpfs_find_dirent(parent, copy);
  if (tde != NULL)
    {
      /* Something with this name already exists in the directory.
       * OR perhaps some fatal error occurred.
       */

      ret = -EEXIST;
      goto errout_with_parent;
    }

  /* Allocate an empty file.  The initial state of the file is locked with one
   * reference count.
   */

  newtfo = tmpfs_alloc_file();
  if (newtfo == NULL)
    {
      ret = -ENOSPC;
      goto errout_with_parent;
    }

  /* Then add the new, empty file to the directory */

  ret = tmpfs_add_dirent(&parent, (struct tmpfs_object_s *)newtfo, copy);
  if (ret < 0)
    {
      goto errout_with_file;
    }

  /* Release the reference and lock on the parent directory */
  if (parent->tdo_refs > 0)
    {
      parent->tdo_refs--;
    }
  tmpfs_unlock_directory(parent);

  /* Free the copy of the relpath and return success */

  kmm_free(copy);
  *tfo = newtfo;
  return OK;

  /* Error exits */

errout_with_file:
  sem_destroy(&newtfo->tfo_exclsem.ts_sem);
  kmm_free(newtfo);

errout_with_parent:
  if (parent->tdo_refs > 0)
  {
    parent->tdo_refs--;
  }
  tmpfs_unlock_directory(parent);

errout_with_copy:
  kmm_free(copy);
  return ret;
}

/****************************************************************************
 * Name: tmpfs_alloc_directory
 ****************************************************************************/

static struct tmpfs_directory_s *tmpfs_alloc_directory(void)
{
  struct tmpfs_directory_s *tdo;
  size_t allocsize;

  allocsize = sizeof(struct tmpfs_directory_s);
  tdo = (struct tmpfs_directory_s *)kmm_malloc(allocsize);
  if (tdo == NULL)
    {
      return NULL;
    }

  /* Initialize the new directory object */

  tdo->tdo_atime    = tmpfs_timestamp();
  tdo->tdo_mtime    = tdo->tdo_mtime;
  tdo->tdo_ctime    =  tdo->tdo_mtime;
  tdo->tdo_type     = TMPFS_DIRECTORY;
  tdo->tdo_refs     = 0;
  tdo->tdo_nentries = 0;
  tdo->tdo_count    = 0;
  LOS_ListInit(&tdo->tdo_entry);

  tdo->tdo_exclsem.ts_holder = TMPFS_NO_HOLDER;
  tdo->tdo_exclsem.ts_count  = 0;
  if (sem_init(&tdo->tdo_exclsem.ts_sem, 0, 1) != 0)
    {
      PRINT_ERR("%s %d, sem_init failed!\n", __FUNCTION__, __LINE__);
      kmm_free(tdo);
      return NULL;
    }
  return tdo;
}

/****************************************************************************
 * Name: tmpfs_create_directory
 ****************************************************************************/

static int tmpfs_create_directory(struct tmpfs_s *fs,
                                  const char *relpath,
                                  struct tmpfs_directory_s *parent_input,
                                  struct tmpfs_directory_s **tdo)
{
  struct tmpfs_directory_s *parent;
  struct tmpfs_directory_s *newtdo;
  struct tmpfs_dirent_s *tde;
  char *copy;
  int ret;

  /* Duplicate the path variable so that we can modify it */

  copy = strdup(relpath);
  if (copy == NULL)
    {
      return -ENOSPC;
    }

  /* Separate the path into the file name and the path to the parent
   * directory.
   */
  if (parent_input == NULL)
    {
      /* No subdirectories... use the root directory */

      parent = (struct tmpfs_directory_s *)fs->tfs_root.tde_object;

      tmpfs_lock_directory(parent);
      parent->tdo_refs++;
    }
  else
    {
      parent = parent_input;
    }

  /* Verify that no object of this name already exists in the directory */
  if (parent == NULL)
    {
      ret = -EEXIST;
      goto errout_with_copy;
    }
  tde = tmpfs_find_dirent(parent, copy);
  if (tde != NULL)
    {
      /* Something with this name already exists in the directory.
       * OR perhaps some fatal error occurred.
       */

      ret = -EEXIST;

      goto errout_with_parent;
    }

  /* Allocate an empty directory object.  NOTE that there is no reference on
   * the new directory and the object is not locked.
   */

  newtdo = tmpfs_alloc_directory();
  if (newtdo == NULL)
    {
      ret = -ENOSPC;
      goto errout_with_parent;
    }

  /* Then add the new, empty file to the directory */

  ret = tmpfs_add_dirent(&parent, (struct tmpfs_object_s *)newtdo, copy);
  if (ret < 0)
    {
      goto errout_with_directory;
    }

  /* Free the copy of the relpath, release our reference to the parent directory,
   * and return success
   */
  if (parent->tdo_refs > 0)
    {
      parent->tdo_refs--;
    }
  tmpfs_unlock_directory(parent);
  kmm_free(copy);

  /* Return the (unlocked, unreferenced) directory object to the caller */

  if (tdo != NULL)
    {
      *tdo = newtdo;
    }

  return OK;

  /* Error exits */

errout_with_directory:
  sem_destroy(&newtdo->tdo_exclsem.ts_sem);
  kmm_free(newtdo);

errout_with_parent:
  if (parent->tdo_refs > 0)
    {
      parent->tdo_refs--;
    }
  tmpfs_unlock_directory(parent);

errout_with_copy:
  kmm_free(copy);
  return ret;
}

/****************************************************************************
 * Name: tmpfs_find_object
 ****************************************************************************/

static int tmpfs_find_object(struct tmpfs_s *fs,
                             const char *relpath,
                             struct tmpfs_object_s **object,
                             struct tmpfs_directory_s **parent)
{
  struct tmpfs_object_s *to = NULL;
  struct tmpfs_dirent_s *tde;
  struct tmpfs_directory_s *tdo = NULL;
  struct tmpfs_directory_s *next_tdo;
  char *segment;
  char *next_segment;
  char *tkptr;
  char *copy;

  /* Make a copy of the path (so that we can modify it via strtok) */

  copy = strdup(relpath);
  if (copy == NULL)
    {
      return -ENOSPC;
    }

  /* Traverse the file system for any object with the matching name */

  to       = fs->tfs_root.tde_object;
  next_tdo = (struct tmpfs_directory_s *)fs->tfs_root.tde_object;
  tdo      = next_tdo;
  for (segment =  strtok_r(copy, "/", &tkptr);
       segment != NULL;
       segment = next_segment)
    {
      /* Get the next segment after the one we are currently working on.
       * This will be NULL is we are working on the final segment of the
       * relpath.
       */

      next_segment = strtok_r(NULL, "/", &tkptr);

      /* Search the next directory. */

      tdo = next_tdo;

      /* Find the TMPFS object with the next segment name in the current
       * directory.
       */

      tde = tmpfs_find_dirent(tdo, segment);
      if (tde == NULL)
        {
          /* No object with this name exists in the directory. */

          kmm_free(copy);
          return -ENOENT;
        }

      to = tde->tde_object;

      /* Is this object another directory? */

      if (to->to_type != TMPFS_DIRECTORY)
        {
          /* No.  Was this the final segment in the path? */

          if (next_segment == NULL)
            {
              /* Then we can break out of the loop now */

               break;
            }

          /* No, this was not the final segement of the relpath.
           * We cannot continue the search if any of the intermediate
           * segments do no correspond to directories.
           */

          kmm_free(copy);
          return -ENOTDIR;
        }

      /* Search this directory for the next segement.  If we
       * exit the loop, tdo will still refer to the parent
       * directory of to.
       */

      next_tdo = (struct tmpfs_directory_s *)to;
    }

  /* When we exit this loop (successfully), to will point to the TMPFS
   * object associated with the terminal segment of the relpath.
   * Increment the reference count on the located object.
   */

  /* Free the dup'ed string */

  kmm_free(copy);

  /* Return what we found */

  if (parent)
    {
      if (tdo != NULL)
        {
          /* Get exclusive access to the parent and increment the reference
           * count on the object.
           */

          tmpfs_lock_directory(tdo);
          tdo->tdo_refs++;
        }

      *parent = tdo;
    }

  if (object)
    {
      if (to != NULL)
        {
          /* Get exclusive access to the object and increment the reference
           * count on the object.
           */

          tmpfs_lock_object(to);
          to->to_refs++;
        }

      *object = to;
    }

  return OK;
}

/****************************************************************************
 * Name: tmpfs_find_file
 ****************************************************************************/

static int tmpfs_find_file(struct tmpfs_s *fs,
                           const char *relpath,
                           struct tmpfs_file_s **tfo,
                           struct tmpfs_directory_s **parent)
{
  struct tmpfs_object_s *to;
  int ret;

  /* Find the object at this path.  If successful, tmpfs_find_object() will
   * lock both the object and the parent directory and will increment the
   * reference count on both.
   */

  ret = tmpfs_find_object(fs, relpath, &to, parent);
  if (ret >= 0)
    {
      /* We found it... but is it a regular file? */

      if (to->to_type != TMPFS_REGULAR)
        {
          /* No... unlock the object and its parent and return an error */

          tmpfs_release_lockedobject(to);

          if (parent)
            {
              struct tmpfs_directory_s *tdo = *parent;

              tdo->tdo_refs--;
              tmpfs_unlock_directory(tdo);
            }

          ret = -EISDIR;
        }

      /* Return the verified file object */

      *tfo = (struct tmpfs_file_s *)to;
    }

  return ret;
}

/****************************************************************************
 * Name: tmpfs_find_directory
 ****************************************************************************/

static int tmpfs_find_directory(struct tmpfs_s *fs,
                           const char *relpath,
                           struct tmpfs_directory_s **tdo,
                           struct tmpfs_directory_s **parent)
{
  struct tmpfs_object_s *to;
  int ret;

  /* Find the object at this path */

  ret = tmpfs_find_object(fs, relpath, &to, parent);
  if (ret >= 0)
    {
      /* We found it... but is it a regular file? */

      if (to->to_type != TMPFS_DIRECTORY)
        {
          /* No... unlock the object and its parent and return an error */

          tmpfs_release_lockedobject(to);

          if (parent)
            {
              struct tmpfs_directory_s *tmptdo = *parent;

              tmptdo->tdo_refs--;
              tmpfs_unlock_directory(tmptdo);
            }

          ret = -ENOTDIR;
        }

      /* Return the verified file object */

      *tdo = (struct tmpfs_directory_s *)to;
    }

  return ret;
}

/****************************************************************************
 * Name: tmpfs_close
 ****************************************************************************/

int tmpfs_close(struct file *filep)
{
  struct tmpfs_file_s *tfo;

  DEBUGASSERT(filep != NULL);

  /* Recover our private data from the struct file instance */
  tfo = (struct tmpfs_file_s *)(filep->f_vnode->data);
  if (tfo == NULL)
    {
      return -EINVAL;
    }
  /* Get exclusive access to the file */

  tmpfs_lock_file(tfo);

  /* Decrement the reference count on the file */

  if (tfo->tfo_refs > 0)
    {
      tfo->tfo_refs--;
    }

  /* If the reference count decremented to zero and the file has been
   * unlinked, then free the file allocation now.
   */

  if (tfo->tfo_refs == 0 && (tfo->tfo_flags & TFO_FLAG_UNLINKED) != 0)
    {
      /* Free the file object while we hold the lock?  Weird but this
       * should be safe because the object is unlinked and could not
       * have any other references.
       */

      (void)sem_destroy(&tfo->tfo_exclsem.ts_sem);
      kmm_free(tfo->tfo_data);
      kmm_free(tfo);
      return OK;
    }

  /* Release the lock on the file */

  tmpfs_unlock_file(tfo);
  return OK;
}

/****************************************************************************
 * Name: tmpfs_read
 ****************************************************************************/

ssize_t tmpfs_read(struct file *filep, char *buffer, size_t buflen)
{
  struct tmpfs_file_s *tfo;
  ssize_t nread;
  loff_t startpos;
  loff_t endpos;

  DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

  /* Recover our private data from the struct file instance */

  tfo = (struct tmpfs_file_s *)(filep->f_vnode->data);
  if (tfo == NULL)
    {
      return -EINVAL;
    }
  if (filep->f_pos >= tfo->tfo_size || buflen == 0)
    {
      return 0;
    }

  /* Get exclusive access to the file */

  tmpfs_lock_file(tfo);

  /* Handle attempts to read beyond the end of the file. */

  startpos = filep->f_pos;
  nread    = buflen;
  endpos   = startpos + buflen;

  if (endpos > tfo->tfo_size)
    {
      endpos = tfo->tfo_size;
      nread  = endpos - startpos;
    }

  /* Copy data from the memory object to the user buffer */

  if (LOS_CopyFromKernel(buffer, buflen, &tfo->tfo_data[startpos], nread) != 0)
    {
      tmpfs_unlock_file(tfo);
      return -EINVAL;
    }
  filep->f_pos += nread;

  /* Update the node's access time */

  tfo->tfo_atime = tmpfs_timestamp();

  /* Release the lock on the file */

  tmpfs_unlock_file(tfo);
  return nread;
}

/****************************************************************************
 * Name: tmpfs_create
 ****************************************************************************/

int tmpfs_create(struct Vnode *dvp, const char *path, int mode, struct Vnode **vpp)
{
    struct Vnode *vp = NULL;
    struct tmpfs_file_s *tfo;
    struct tmpfs_s *fs;
    int ret = 0;
    struct tmpfs_directory_s *parent_tdo = NULL;

    if (dvp == NULL)
      {
        return -ENOENT;
      }

    fs = dvp->originMount->data;
    if (fs == NULL)
      {
        return -ENOENT;
      }

    tmpfs_lock(fs);

    if (dvp->data != NULL)
      {
        parent_tdo = (struct tmpfs_directory_s *)(dvp->data);
      }

    ret = tmpfs_create_file(fs, path, parent_tdo, &tfo);
    if (ret < 0)
      {
        goto errout_with_fslock;
      }

    ret = VnodeAlloc(&tmpfs_vops, &vp);
    if (ret != 0)
      {
        tmpfs_unlock_file(tfo);
        goto errout_with_fslock;
      }
    vp->parent = dvp;
    vp->vop = dvp->vop;
    vp->fop = dvp->fop;
    vp->data = tfo;
    vp->originMount = dvp->originMount;
    vp->type = VNODE_TYPE_REG;
    tfo->mode = mode;
    vp->mode = tfo->mode;
    vp->gid = tfo->gid;
    vp->uid = tfo->uid;

    ret = VfsHashInsert(vp, (uint32_t)tfo);

    *vpp = vp;
    tmpfs_unlock_file(tfo);
errout_with_fslock:
    tmpfs_unlock(fs);
    return 0;
}


/****************************************************************************
 * Name: los_set_ramfs_unit
 ****************************************************************************/

static spinlock_t tmpfs_alloc_unit_lock;
bool is_tmpfs_lock_init = false;
unsigned int g_tmpfs_alloc_unit = 0;

void los_set_ramfs_unit(off_t size)
{
  if (is_tmpfs_lock_init && size >= 0)
    {
      spin_lock(&tmpfs_alloc_unit_lock);
      g_tmpfs_alloc_unit = size;
      spin_unlock(&tmpfs_alloc_unit_lock);
    }
}

/****************************************************************************
 * Name: tmpfs_write
 ****************************************************************************/

ssize_t tmpfs_write(struct file *filep, const char *buffer, size_t buflen)
{
  struct tmpfs_file_s *tfo;
  ssize_t nwritten;
  loff_t startpos;
  loff_t endpos;
  int ret;
  int alloc;
  char *data;

  DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

  if (buflen == 0)
    {
      return 0;
    }

  /* Recover our private data from the struct file instance */
  tfo = (struct tmpfs_file_s *)(filep->f_vnode->data);
  if (tfo == NULL)
    {
      return -EINVAL;
    }
  /* Get exclusive access to the file */

  tmpfs_lock_file(tfo);

  /* Handle attempts to write beyond the end of the file */

  startpos = filep->f_pos;
  nwritten = buflen;
  endpos   = startpos + buflen;

  if (startpos < 0)
    {
      ret = -EPERM;
      goto errout_with_lock;
    }

  if (endpos > tfo->tfo_size)
    {
      spin_lock(&tmpfs_alloc_unit_lock);
      alloc = (g_tmpfs_alloc_unit > buflen) ? g_tmpfs_alloc_unit : buflen;
      spin_unlock(&tmpfs_alloc_unit_lock);
      data  = (char *)malloc(startpos + alloc);
      if (!data)
        {
          ret = -ENOSPC;
          goto errout_with_lock;
        }
      if (tfo->tfo_size)
        {
          ret = memcpy_s(data, startpos + alloc, tfo->tfo_data, tfo->tfo_size);
          if (ret != EOK)
            {
              ret = -1;
              free(data);
              goto errout_with_lock;
            }
          free(tfo->tfo_data);
        }
      if (startpos > tfo->tfo_size)
        {
          (void)memset_s(data + tfo->tfo_size, startpos + alloc - tfo->tfo_size, 0, startpos - tfo->tfo_size);
        }

      tfo->tfo_data = data;
      tfo->tfo_size = startpos + alloc;
    }

  /* Copy data from the memory object to the user buffer */
  if (LOS_CopyToKernel(&tfo->tfo_data[startpos], nwritten, buffer, nwritten) != 0)
    {
      ret = -EINVAL;
      goto errout_with_lock;
    }
  filep->f_pos += nwritten;

  /* Update the modified and access times of the node */

  tfo->tfo_ctime = tfo->tfo_mtime = tmpfs_timestamp();

  /* Release the lock on the file */

  tmpfs_unlock_file(tfo);
  return nwritten;

errout_with_lock:
  tmpfs_unlock_file(tfo);
  return (ssize_t)ret;
}

/****************************************************************************
 * Name: tmpfs_seek
 ****************************************************************************/

off_t tmpfs_seek(struct file *filep, off_t offset, int whence)
{
  struct tmpfs_file_s *tfo;
  off_t position;

  DEBUGASSERT(filep->f_priv != NULL && filep->f_vnode != NULL);

  /* Recover our private data from the struct file instance */

  tfo = (struct tmpfs_file_s *)(filep->f_vnode->data);
  if (tfo == NULL)
    {
      return -EINVAL;
    }
  /* Map the offset according to the whence option */

  switch (whence)
    {
      case SEEK_SET: /* The offset is set to offset bytes. */
          position = offset;
          break;

      case SEEK_CUR: /* The offset is set to its current location plus
                      * offset bytes. */
          position = offset + filep->f_pos;
          break;

      case SEEK_END: /* The offset is set to the size of the file plus
                      * offset bytes. */
          position = offset + tfo->tfo_size;
          break;

      default:
          return -EINVAL;
    }

  /* Attempts to set the position beyond the end of file will
   * work if the file is open for write access.
   *
   * REVISIT: This simple implementation has no per-open storage that
   * would be needed to retain the open flags.
   */
    if (position < 0)
    {
      return -EINVAL;
    }

  /* Save the new file position */

  filep->f_pos = position;
  return position;
}

/****************************************************************************
 * Name: tmpfs_ioctl
 ****************************************************************************/

int tmpfs_ioctl(struct file *filep, int cmd, unsigned long arg)
{
  return -EINVAL;
}

/****************************************************************************
 * Name: tmpfs_opendir
 ****************************************************************************/

int tmpfs_opendir(struct Vnode *vp, struct fs_dirent_s *dir)
{
  struct tmpfs_s *fs;
  struct tmpfs_directory_s *tdo;
  int ret = 0;
  struct fs_tmpfsdir_s *tmp;

  DEBUGASSERT(vp != NULL && dir != NULL);

  /* Get the mountpoint private data from the inode structure */

  fs = vp->originMount->data;
  DEBUGASSERT(fs != NULL);

  tmp = (struct fs_tmpfsdir_s *)malloc(sizeof(struct fs_tmpfsdir_s));
  if (!tmp)
    {
      return -ENOSPC;
    }

  /* Get exclusive access to the file system */

  tmpfs_lock(fs);

  /* Find the directory object associated with this relative path.
   * If successful, this action will lock both the parent directory and
   * the file object, adding one to the reference count of both.
   * In the event that -ENOENT, there will still be a reference and
   * lock on the returned directory.
   */

  if (vp->data != NULL)
    {
      tdo = (struct tmpfs_directory_s *)vp->data;
    }
  else
    {
      tdo = (struct tmpfs_directory_s *)fs->tfs_root.tde_object;
    }

  if (tdo == NULL)
    {
      free(tmp);
      tmpfs_unlock(fs);
      return -EINTR;
    }
  tmp->tf_tdo   = tdo;
  tmp->tf_index = 0;
  dir->u.fs_dir = (fs_dir_s)tmp;
  tdo->tdo_count++;
  tdo->tdo_refs++;
  tmpfs_unlock_directory(tdo);

  /* Release the lock on the file system and return the result */

  tmpfs_unlock(fs);
  return ret;
}

/****************************************************************************
 * Name: tmpfs_closedir
 ****************************************************************************/

int tmpfs_closedir(struct Vnode *vp, struct fs_dirent_s *dir)
{
  struct tmpfs_directory_s *tdo;
  struct fs_tmpfsdir_s *tmp;

  DEBUGASSERT(vp != NULL && dir != NULL);

  /* Get the directory structure from the dir argument */

  tmp = (struct fs_tmpfsdir_s *)dir->u.fs_dir;
  if (tmp == NULL)
    {
      return -ENOENT;
    }
  tdo = tmp->tf_tdo;
  DEBUGASSERT(tdo != NULL);

  /* Decrement the reference count on the directory object */

  tmpfs_lock_directory(tdo);
  if (tdo->tdo_count == 1)
    {
      LOS_DL_LIST *node = tdo->tdo_entry.pstNext;
      struct tmpfs_dirent_s *tde;
      while (node != &tdo->tdo_entry)
        {
          tde = (struct tmpfs_dirent_s *)node;
          node = node->pstNext;
          if (tde->tde_inuse == false)
            {
              LOS_ListDelete(&tde->tde_node);
              kmm_free(tde);
            }
        }
    }
  if (tdo->tdo_refs > 0)
    {
      tdo->tdo_refs--;
    }
  if (tdo->tdo_count > 0)
    {
      tdo->tdo_count--;
    }
  tmpfs_unlock_directory(tdo);

  free(tmp);

  return OK;
}

/****************************************************************************
 * Name: tmpfs_readdir
 ****************************************************************************/

int tmpfs_readdir(struct Vnode *vp, struct fs_dirent_s *dir)
{
  struct tmpfs_directory_s *tdo;
  unsigned int index;
  int ret;
  struct fs_tmpfsdir_s *tmp;
  LOS_DL_LIST *node;
  struct tmpfs_dirent_s *tde;

  DEBUGASSERT(vp != NULL && dir != NULL);

  /* Get the directory structure from the dir argument and lock it */

  tmp = (struct fs_tmpfsdir_s *)dir->u.fs_dir;
  if (tmp == NULL)
    {
      return -ENOENT;
    }

  tdo = tmp->tf_tdo;
  if (tdo == NULL)
    {
      return -ENOENT;
    }

  tmpfs_lock_directory(tdo);

  /* Have we reached the end of the directory? */

  index = tmp->tf_index;
  node = tdo->tdo_entry.pstNext;
  while (node != &tdo->tdo_entry && index != 0)
    {
       node = node->pstNext;
       index--;
    }

  while (node != &tdo->tdo_entry)
    {
      tde = (struct tmpfs_dirent_s *)node;
      tmp->tf_index++;
      if (tde->tde_inuse == true)
       {
         break;
       }
       node = node->pstNext;
    }

  if (node == &tdo->tdo_entry)
    {
      /* We signal the end of the directory by returning the special error:
       * -ENOENT
       */

      PRINT_INFO("End of directory\n");
      ret = -ENOENT;
    }
  else
    {
      struct tmpfs_object_s *to;

      /* Does this entry refer to a file or a directory object? */

      to  = tde->tde_object;
      DEBUGASSERT(to != NULL);

      if (to->to_type == TMPFS_DIRECTORY)
        {
          /* A directory */

           dir->fd_dir[0].d_type = DT_DIR;
        }
      else /* to->to_type == TMPFS_REGULAR) */
        {
          /* A regular file */

           dir->fd_dir[0].d_type = DT_REG;
        }

      /* Copy the entry name */

      (void)strncpy_s(dir->fd_dir[0].d_name, NAME_MAX + 1, tde->tde_name, NAME_MAX);

      dir->fd_position++;
      dir->fd_dir[0].d_off = dir->fd_position;
      dir->fd_dir[0].d_reclen = (uint16_t)sizeof(struct dirent);

      ret = 1; // 1 means current file num is 1
    }

  tmpfs_unlock_directory(tdo);
  return ret;
}

/****************************************************************************
 * Name: tmpfs_rewinddir
 ****************************************************************************/

int tmpfs_rewinddir(struct Vnode *vp, struct fs_dirent_s *dir)
{
  struct fs_tmpfsdir_s *tmp;
  PRINT_DEBUG("vp: %p dir: %p\n",  vp, dir);
  DEBUGASSERT(vp != NULL && dir != NULL);
  tmp = (struct fs_tmpfsdir_s *)dir->u.fs_dir;

  /* Set the readdir index to zero */

  tmp->tf_index = 0;
  return OK;
}

int tmpfs_truncate(struct Vnode *vp, off_t len)
{
  struct tmpfs_file_s *tfo = NULL;

  tfo = vp->data;
  tfo->tfo_size = 0;

  if (tfo->tfo_data)
    {
      free(tfo->tfo_data);
    }

  return OK;
}


/****************************************************************************
 * Name: tmpfs_mount
 ****************************************************************************/

int tmpfs_mount(struct Mount *mnt, struct Vnode *device, const void *data)
{
  struct tmpfs_directory_s *tdo;
  struct tmpfs_s *fs = &tmpfs_superblock;
  struct Vnode *vp = NULL;
  int ret;

  DEBUGASSERT(device == NULL && data != NULL);

  if (fs->tfs_root.tde_object != NULL)
    {
      return -EPERM;
    }

  /* Create a root file system.  This is like a single directory entry in
   * the file system structure.
   */

  tdo = tmpfs_alloc_directory();
  if (tdo == NULL)
    {
      return -ENOSPC;
    }

  LOS_ListInit(&fs->tfs_root.tde_node);
  fs->tfs_root.tde_object = (struct tmpfs_object_s *)tdo;
  fs->tfs_root.tde_name   = NULL;

  /* Set up the backward link (to support reallocation) */

  tdo->tdo_dirent         = &fs->tfs_root;

  /* Initialize the file system state */

  fs->tfs_exclsem.ts_holder = TMPFS_NO_HOLDER;
  fs->tfs_exclsem.ts_count  = 0;
  sem_init(&fs->tfs_exclsem.ts_sem, 0, 1);

  /* Return the new file system handle */

  spin_lock_init(&tmpfs_alloc_unit_lock);
  is_tmpfs_lock_init = true;

  ret = VnodeAlloc(&tmpfs_vops, &vp);
  if (ret != 0)
    {
      ret = ENOMEM;
      goto ERROR_WITH_FSWIN;
    }

  tdo->mode = mnt->vnodeBeCovered->mode;
  tdo->gid = mnt->vnodeBeCovered->gid;
  tdo->uid = mnt->vnodeBeCovered->uid;
  vp->originMount = mnt;
  vp->fop = &tmpfs_fops;
  vp->type = VNODE_TYPE_DIR;
  vp->data = NULL;
  vp->mode = tdo->mode;
  vp->gid = tdo->gid;
  vp->uid = tdo->uid;
  mnt->data = fs;
  mnt->vnodeCovered = vp;

  return OK;

ERROR_WITH_FSWIN:
  return ret;
}

/****************************************************************************
 * Name: tmpfs_unmount
 ****************************************************************************/

int tmpfs_unmount(struct Mount *mnt, struct Vnode **blkdriver)
{
  struct tmpfs_s *fs = (struct tmpfs_s *)mnt->data;
  struct tmpfs_directory_s *tdo;
  int ret = 0;

  DEBUGASSERT(fs != NULL && fs->tfs_root.tde_object != NULL);

  /* Lock the file system */

  tmpfs_lock(fs);

  tdo = (struct tmpfs_directory_s *)fs->tfs_root.tde_object;
  if (tdo == NULL)
    {
      ret = -EINVAL;
      goto errout_with_objects;
    }

  if (tdo->tdo_nentries > 0 || tdo->tdo_refs > 1)
    {
      ret = -EBUSY;
      goto errout_with_objects;
    }

  /* Now we can destroy the root file system and the file system itself. */

  sem_destroy(&tdo->tdo_exclsem.ts_sem);
  kmm_free(tdo);

  sem_destroy(&fs->tfs_exclsem.ts_sem);
  fs->tfs_root.tde_object = NULL;

  tmpfs_unlock(fs);
  is_tmpfs_lock_init = false;
  return ret;

errout_with_objects:
  tmpfs_unlock(fs);
  return ret;
}


int tmpfs_lookup(struct Vnode *parent, const char *relPath, int len, struct Vnode **vpp)
{
  // 1. when first time create file, lookup fail, then call tmpfs_create.
  // 2. when  ls, lookup will success to show.
  // 3. when cd, lookup success.
  struct tmpfs_object_s *to;
  struct tmpfs_s *fs;
  struct tmpfs_dirent_s *tde = NULL;
  struct tmpfs_directory_s *parent_tdo;

  struct Vnode *vp = NULL;
  int ret = 0;
  char filename[len + 1];
  ret = memcpy_s(filename, (len + 1), relPath, len);
  if (ret != 0)
    {
        ret = -ENOMEM;
        goto errout;
    }
  filename[len] = '\0';

  fs = parent->originMount->data;
  DEBUGASSERT(fs != NULL && fs->tfs_root.tde_object != NULL);

  tmpfs_lock(fs);

  parent_tdo = (struct tmpfs_directory_s *)(parent->data);

  if (parent_tdo == NULL)
    {
      // if parent_tdo don't exist, find file in root.
      ret = tmpfs_find_object(fs, filename, &to, NULL);
      if (ret < 0)
        {
          goto errout_with_lock;
        }
    }
  else
    {
      if (parent_tdo->tdo_type != TMPFS_DIRECTORY)
        {
          ret = -ENOENT;
          goto errout_with_lock;
        }
      // if parent_tdo exist£¬search for the relationship between parent_tdo and the current dirname.
      tde = tmpfs_find_dirent(parent_tdo, filename);
      if (tde == NULL)
        {
          ret = -ENOENT;
          goto errout_with_lock;
        }
      to = tde->tde_object;
    }

  if (to == NULL)
    {
      ret = -ENOENT;
      goto errout_with_lock;
    }

  (void)VfsHashGet(parent->originMount, (uint32_t)to, &vp, NULL, NULL);
  if (vp == NULL)
    {
      ret = VnodeAlloc(&tmpfs_vops, &vp);
      if (ret != 0)
        {
          PRINTK("%s-%d \n", __FUNCTION__, __LINE__);
          goto errout_with_objects;
        }

      vp->vop = parent->vop;
      vp->fop = parent->fop;
      vp->data = to;
      vp->originMount = parent->originMount;
      vp->type = to->to_type == TMPFS_REGULAR ? VNODE_TYPE_REG : VNODE_TYPE_DIR;
      vp->mode = to->mode;
      vp->gid = to->gid;
      vp->uid = to->uid;

      ret = VfsHashInsert(vp, (uint32_t)to);
    }
  vp->parent = parent;

  *vpp = vp;

  tmpfs_release_lockedobject(to);
  tmpfs_unlock(fs);

  return 0;

errout_with_objects:
  tmpfs_release_lockedobject(to);
errout_with_lock:
  tmpfs_unlock(fs);
errout:
  return ret;
}

int tmpfs_reclaim(struct Vnode *vp)
{
    vp->data = NULL;
    return 0;
}

/****************************************************************************
 * Name: tmpfs_statfs
 ****************************************************************************/

int tmpfs_statfs(struct Mount *mp, struct statfs *sbp)
{
  (void)memset_s(sbp, sizeof(struct statfs), 0, sizeof(struct statfs));

  sbp->f_type = TMPFS_MAGIC;
  sbp->f_flags = mp->mountFlags;

  return OK;
}

/****************************************************************************
 * Name: tmpfs_unlink
 ****************************************************************************/

int tmpfs_unlink(struct Vnode *parent, struct Vnode *node, const char *relpath)
{
  struct tmpfs_s *fs;
  struct tmpfs_directory_s *parent_dir;
  struct tmpfs_file_s *tfo = NULL;
  int ret;

  PRINT_INFO("mountpt: %p node: %p relpath: %s\n", parent, node, relpath);
  DEBUGASSERT(parent != NULL && node != NULL && relpath != NULL);

  if (strlen(relpath) == 0)
    {
      return -EISDIR;
    }

  /* Get the file system structure from the inode reference. */
  if (node->originMount == NULL)
    {
      return -EISDIR;
    }

  fs = node->originMount->data;
  if (fs == NULL)
    {
      return -EISDIR;
    }

  DEBUGASSERT(fs != NULL && fs->tfs_root.tde_object != NULL);

  /* Get exclusive access to the file system */

  tmpfs_lock(fs);

  /* Find the file object and parent directory associated with this relative
   * path.  If successful, tmpfs_find_file will lock both the file object
   * and the parent directory and take one reference count on each.
   */

  parent_dir = (struct tmpfs_directory_s *)(parent->data);
  if (parent_dir == NULL)
    {
      ret = tmpfs_find_file(fs, relpath, &tfo, &parent_dir);
      if (ret < 0)
        {
          goto errout_with_lock;
        }
    }
  else
    {
      tfo = (struct tmpfs_file_s *)node->data;
    }

  if (tfo == NULL || parent_dir == NULL)
    {
      ret = -EISDIR;
      goto errout_with_lock;
    }
  DEBUGASSERT(tfo != NULL);

  /* Remove the file from parent directory */
  ret = tmpfs_remove_dirent(parent_dir, (struct tmpfs_object_s *)tfo);
  if (ret < 0)
    {
      goto errout_with_objects;
    }

  /* If the reference count is not one, then just mark the file as
   * unlinked
   */

  if (tfo->tfo_refs > 1)
    {
      /* Make the file object as unlinked */

      tfo->tfo_flags |= TFO_FLAG_UNLINKED;

      /* Release the reference count on the file object */

      tfo->tfo_refs--;
      tmpfs_unlock_file(tfo);
    }

  /* Otherwise we can free the object now */

  else
    {
      sem_destroy(&tfo->tfo_exclsem.ts_sem);
      kmm_free(tfo->tfo_data);
      kmm_free(tfo);
      node->data = NULL;
    }

  /* Release the reference and lock on the parent directory */

  if (parent_dir->tdo_refs > 0)
    {
      parent_dir->tdo_refs--;
    }
  tmpfs_unlock_directory(parent_dir);
  tmpfs_unlock(fs);

  return OK;

errout_with_objects:
  tmpfs_release_lockedfile(tfo);

  if (parent_dir->tdo_refs > 0)
    {
      parent_dir->tdo_refs--;
    }

  tmpfs_unlock_directory(parent_dir);

errout_with_lock:
  tmpfs_unlock(fs);
  return ret;
}

/****************************************************************************
 * Name: tmpfs_mkdir
 ****************************************************************************/

int tmpfs_mkdir(struct Vnode *parent, const char *relpath, mode_t mode, struct Vnode **vpp)
{
  struct tmpfs_s *fs;
  struct Vnode *vp = NULL;
  struct tmpfs_directory_s *tdo;
  struct tmpfs_directory_s *parent_tdo = NULL;
  int ret;

  DEBUGASSERT(parent != NULL && relpath != NULL);

  if (strlen(relpath) == 0)
    {
      return -EEXIST;
    }

  /* Get the file system structure from the inode reference. */

  fs = parent->originMount->data;
  DEBUGASSERT(fs != NULL && fs->tfs_root.tde_object != NULL);

  /* Get exclusive access to the file system */

  tmpfs_lock(fs);

  if (parent->data != NULL)
    {
      parent_tdo = (struct tmpfs_directory_s *)(parent->data);
    }
  /* Create the directory. */
  ret = tmpfs_create_directory(fs, relpath, parent_tdo, &tdo);
  if (ret != OK)
    {
      goto errout_with_lock;
    }

  ret = VnodeAlloc(&tmpfs_vops, &vp);
  if (ret != 0)
    {
      goto errout_with_lock;
    }

  tdo->mode = mode;
  vp->parent = parent;
  vp->vop = parent->vop;
  vp->fop = parent->fop;
  vp->data = tdo;
  vp->originMount = parent->originMount;
  vp->type = VNODE_TYPE_DIR;
  vp->mode = tdo->mode;
  vp->gid = tdo->gid;
  vp->uid = tdo->uid;

  ret = VfsHashInsert(vp, (uint32_t)tdo);
  *vpp = vp;

errout_with_lock:
  tmpfs_unlock(fs);
  return ret;
}

/****************************************************************************
 * Name: tmpfs_rmdir
 ****************************************************************************/

int tmpfs_rmdir(struct Vnode *parent, struct Vnode *target, const char *dirname)
{
  struct tmpfs_s *fs;
  struct tmpfs_directory_s *parent_dir;
  struct tmpfs_directory_s *tdo;
  int ret = 0;

  DEBUGASSERT(parent != NULL && target != NULL && relpath != NULL);

  /* Get the file system structure from the inode reference. */
  fs = parent->originMount->data;
  if (fs == NULL)
    {
      return -EISDIR;
    }
  DEBUGASSERT(fs != NULL && fs->tfs_root.tde_object != NULL);

  /* Get exclusive access to the file system */

  tmpfs_lock(fs);

  /* Find the directory object and parent directory associated with this
   * relative path.  If successful, tmpfs_find_file will lock both the
   * directory object and the parent directory and take one reference count
   * on each.
   */
  parent_dir = (struct tmpfs_directory_s *)(parent->data);
  if (parent_dir == NULL)
    {
      ret = tmpfs_find_directory(fs, dirname, &tdo, &parent_dir);
      if (ret < 0)
        {
          goto errout_with_lock;
        }
    }
  else
    {
      tdo = (struct tmpfs_directory_s *)target->data;
    }

  if (tdo == NULL || tdo->tdo_type != TMPFS_DIRECTORY)
    {
      ret = -EISDIR;
      goto errout_with_lock;
    }

  /* Is the directory empty?  We cannot remove directories that still
   * contain references to file system objects.  No can we remove the
   * directory if there are outstanding references on it (other than
   * our reference).
   */

  if (tdo->tdo_nentries > 0 || tdo->tdo_refs > 1)
    {
      ret = -EBUSY;
      goto errout_with_objects;
    }
  /* Remove the directory from parent directory */
  ret = tmpfs_remove_dirent(parent_dir, (struct tmpfs_object_s *)tdo);
  if (ret < 0)
    {
      goto errout_with_objects;
    }

  /* Free the directory object */

  sem_destroy(&tdo->tdo_exclsem.ts_sem);
  kmm_free(tdo);
  target->data = NULL;

  /* Release the reference and lock on the parent directory */

  if (parent_dir->tdo_refs > 0)
    {
      parent_dir->tdo_refs--;
    }

  tmpfs_unlock_directory(parent_dir);
  tmpfs_unlock(fs);

  return OK;

errout_with_objects:
  if (tdo->tdo_refs > 0)
    {
      tdo->tdo_refs--;
    }
  tmpfs_unlock_directory(tdo);

  if (parent_dir->tdo_refs > 0)
    {
      parent_dir->tdo_refs--;
    }
  tmpfs_unlock_directory(parent_dir);

errout_with_lock:
  tmpfs_unlock(fs);
  return ret;
}

/****************************************************************************
 * Name: tmpfs_rename
 ****************************************************************************/

int tmpfs_rename(struct Vnode *oldVnode, struct Vnode *newParent, const char *oldname, const char *newname)
{
  struct tmpfs_directory_s *oldparent;
  struct tmpfs_directory_s *newparent_tdo;
  struct tmpfs_object_s *old_to;
  struct tmpfs_dirent_s *tde;
  struct tmpfs_directory_s *tdo;
  struct tmpfs_file_s *tfo;
  struct tmpfs_s *fs;
  struct tmpfs_s *new_fs;
  struct Vnode *old_parent_vnode;

  char *copy;
  int ret = 0;
  unsigned int oldrelpath_len, newrelpath_len, cmp_namelen;

  oldrelpath_len = strlen(oldname);
  newrelpath_len = strlen(newname);

  cmp_namelen = (oldrelpath_len <= newrelpath_len) ? oldrelpath_len : newrelpath_len;
  if (!cmp_namelen || ((!strncmp(oldname, newname, cmp_namelen)) &&
           (oldname[cmp_namelen] == '/' ||
            newname[cmp_namelen] == '/')))
    {
      return -EPERM;
    }

  /* Get the file system structure from the inode reference. */
  if (oldVnode->parent == NULL)
    {
      return -EPERM;
    }

  fs = oldVnode->parent->originMount->data;
  if (fs == NULL)
    {
      return -EPERM;
    }

  DEBUGASSERT(fs != NULL && fs->tfs_root.tde_object != NULL);

  /* Duplicate the newpath variable so that we can modify it */

  copy = strdup(newname);
  if (copy == NULL)
    {
      return -ENOSPC;
    }

  /* Get exclusive access to the file system */

  tmpfs_lock(fs);

  /* Separate the new path into the new file name and the path to the new
   * parent directory.
   */
  newparent_tdo = (struct tmpfs_directory_s *)(newParent->data);
  if (newparent_tdo == NULL)
    {
      new_fs = newParent->originMount->data;
      newparent_tdo = (struct tmpfs_directory_s *)new_fs->tfs_root.tde_object;
      if (newparent_tdo == NULL)
        {
          ret = -ENOTEMPTY;
          goto errout_with_lock;
        }
      tmpfs_lock_directory(newparent_tdo);
      newparent_tdo->tdo_refs++;
    }

  /* Find the old object at oldpath.  If successful, tmpfs_find_object()
   * will lock both the object and the parent directory and will increment
   * the reference count on both.
   */
  old_parent_vnode = oldVnode->parent;
  oldparent = (struct tmpfs_directory_s *)(old_parent_vnode->data);
  old_to = (struct tmpfs_object_s *)oldVnode->data;

  if (oldparent == NULL)
    {
      oldparent = (struct tmpfs_directory_s *)fs->tfs_root.tde_object;
      if (oldparent == NULL || old_to == NULL)
        {
          ret = -ENOTEMPTY;
          goto errout_with_newparent;
        }
    }

  tmpfs_lock_directory(oldparent);
  tde = tmpfs_find_dirent(newparent_tdo, copy);
  if (tde != NULL)
    {
      struct tmpfs_object_s *new_to = tde->tde_object;

      /* Cannot rename a directory to a noempty directory */

      if (tde->tde_object->to_type == TMPFS_DIRECTORY)
        {
          tdo = (struct tmpfs_directory_s *)new_to;
          if (tdo->tdo_nentries != 0)
            {
              ret = -ENOTEMPTY;
              goto errout_with_oldparent;
            }
        }

      /* Null rename, just return */

      if (old_to == new_to)
        {
          ret = ENOERR;
          goto errout_with_oldparent;
        }

      /* Check that we are renaming like-for-like */

      if (old_to->to_type == TMPFS_REGULAR && new_to->to_type == TMPFS_DIRECTORY)
        {
          ret = -EISDIR;
          goto errout_with_oldparent;
        }

      if (old_to->to_type == TMPFS_DIRECTORY && new_to->to_type == TMPFS_REGULAR)
        {
          ret = -ENOTDIR;
          goto errout_with_oldparent;
        }

      /* Now delete the destination directory entry */

      ret = tmpfs_remove_dirent(newparent_tdo, new_to);
      if (ret < 0)
        {
          goto errout_with_oldparent;
        }

      if (new_to->to_type == TMPFS_DIRECTORY)
        {
          (void)sem_destroy(&new_to->to_exclsem.ts_sem);
          kmm_free(new_to);
        }
      else
        {
          tfo = (struct tmpfs_file_s *)new_to;
          if (new_to->to_refs > 0)
            {
              /* Make the file object as unlinked */

              tfo->tfo_flags |= TFO_FLAG_UNLINKED;
            }

          /* Otherwise we can free the object now */

          else
            {
              (void)sem_destroy(&tfo->tfo_exclsem.ts_sem);
              kmm_free(tfo->tfo_data);
              kmm_free(tfo);
            }
        }
    }

  /* Remove the entry from the parent directory */

  ret = tmpfs_remove_dirent(oldparent, old_to);
  if (ret < 0)
    {
      goto errout_with_oldparent;
    }

  /* Add an entry to the new parent directory. */

  ret = tmpfs_add_dirent(&newparent_tdo, old_to, copy);
  oldVnode->parent = newParent;

errout_with_oldparent:
  if (oldparent == NULL)
    {
      tmpfs_unlock(fs);
      kmm_free(copy);
      return ret;
    }
  if (oldparent->tdo_refs > 0)
    {
      oldparent->tdo_refs--;
    }
  tmpfs_unlock_directory(oldparent);
  tmpfs_release_lockedobject(old_to);

errout_with_newparent:
  if (newparent_tdo == NULL)
    {
      tmpfs_unlock(fs);
      kmm_free(copy);
      return ret;
    }
  if (newparent_tdo->tdo_refs > 0)
    {
      newparent_tdo->tdo_refs--;
    }
  tmpfs_unlock_directory(newparent_tdo);

errout_with_lock:
  tmpfs_unlock(fs);
  kmm_free(copy);
  return ret;
}

/****************************************************************************
 * Name: tmpfs_stat_common
 ****************************************************************************/

static void tmpfs_stat_common(struct tmpfs_object_s *to,
                              struct stat *buf)
{
  size_t objsize;

  /* Is the tmpfs object a regular file? */

  (VOID)memset_s(buf, sizeof(struct stat), 0, sizeof(struct stat));

  if (to->to_type == TMPFS_REGULAR)
    {
      struct tmpfs_file_s *tfo =
        (struct tmpfs_file_s *)to;

      /* -rwxrwxrwx */

      buf->st_mode = S_IRWXO | S_IRWXG | S_IRWXU | S_IFREG;
      buf->st_nlink = tfo->tfo_refs - 1;

      /* Get the size of the object */

      objsize = tfo->tfo_size;
    }
  else /* if (to->to_type == TMPFS_DIRECTORY) */
    {
      struct tmpfs_directory_s *tdo =
        (struct tmpfs_directory_s *)to;

      /* drwxrwxrwx */

      buf->st_mode = S_IRWXO | S_IRWXG | S_IRWXU | S_IFDIR;
      buf->st_nlink   = tdo->tdo_nentries + tdo->tdo_refs;

      /* Get the size of the object */

      objsize = sizeof(struct tmpfs_directory_s);
    }

  /* Fake the rest of the information */

  buf->st_size    = objsize;
  buf->st_blksize = 0;
  buf->st_blocks  = 0;
  buf->st_atime   = to->to_atime;
  buf->st_mtime   = to->to_mtime;
  buf->st_ctime   = to->to_ctime;
}

/****************************************************************************
 * Name: tmpfs_stat
 ****************************************************************************/

int tmpfs_stat(struct Vnode *vp, struct stat *st)
{
  struct tmpfs_s *fs;
  struct tmpfs_object_s *to;
  int ret;

  DEBUGASSERT(vp != NULL && st != NULL);
  /* Get the file system structure from the inode reference. */

  fs = vp->originMount->data;
  DEBUGASSERT(fs != NULL && fs->tfs_root.tde_object != NULL);

  /* Get exclusive access to the file system */

  tmpfs_lock(fs);

  /* Find the tmpfs object at the relpath.  If successful,
   * tmpfs_find_object() will lock the object and increment the
   * reference count on the object.
   */
  if (vp->data != NULL)
    {
      to = (struct tmpfs_object_s *)vp->data;
    }
  else
    {
      to = fs->tfs_root.tde_object;
    }
  to->to_refs++;

  /* We found it... Return information about the file object in the stat
   * buffer.
   */
  if(to == NULL)
    {
      ret = -ENOENT;
      goto errout_with_fslock;
    }
  DEBUGASSERT(to != NULL);
  tmpfs_stat_common(to, st);

  /* Unlock the object and return success */

  tmpfs_release_lockedobject(to);
  ret = OK;

errout_with_fslock:
  tmpfs_unlock(fs);
  return ret;
}

FSMAP_ENTRY(ramfs_fsmap, "ramfs", tmpfs_operations, FALSE, FALSE);

#endif

