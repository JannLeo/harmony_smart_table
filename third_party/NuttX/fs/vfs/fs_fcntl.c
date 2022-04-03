/****************************************************************************
 * fs/vfs/fs_fcntl.c
 *
 *   Copyright (C) 2009, 2012-2014, 2016-2017 Gregory Nutt. All rights
 *     reserved.
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

#include "stdarg.h"
#include "fcntl.h"
#include "errno.h"
#include "assert.h"
#include "vnode.h"

#if defined(LOSCFG_NET_LWIP_SACK)
#include "lwip/sockets.h"
#endif

#define FAPPEND     O_APPEND
#define FFSYNC      O_SYNC
#define FNONBLOCK   O_NONBLOCK
#define FNDELAY     O_NDELAY
#define FFCNTL      (FNONBLOCK | FNDELAY | FAPPEND | FFSYNC | FASYNC)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_vfcntl
 *
 * Description:
 *   Similar to the standard vfcntl function except that is accepts a struct
 *   struct file instance instead of a file descriptor.
 *
 * Input Parameters:
 *   filep - Instance for struct file for the opened file.
 *   cmd   - Indentifies the operation to be performed.
 *   ap    - Variable argument following the command.
 *
 * Returned Value:
 *   The nature of the return value depends on the command.  Non-negative
 *   values indicate success.  Failures are reported as negated errno
 *   values.
 *
 ****************************************************************************/

#if CONFIG_NFILE_DESCRIPTORS > 0
int file_vfcntl(struct file *filep, int cmd, va_list ap)
{
  int err = 0;
  int ret = OK;

  /* Was this file opened ? */

  if (!filep || !filep->f_vnode)
    {
      err = EBADF;
      goto errout;
    }

  switch (cmd)
    {
      case F_DUPFD:
        /* Return a new file descriptor which shall be the lowest numbered
         * available (that is, not already open) file descriptor greater than
         * or equal to the third argument, arg, taken as an integer of type
         * int. The new file descriptor shall refer to the same open file
         * description as the original file descriptor, and shall share any
         * locks.  The FD_CLOEXEC flag associated  with the new file descriptor
         * shall be cleared to keep the file open across calls to one of the
         * exec functions.
         */

        {
          /* Does not set the errno variable in the event of a failure */

          ret = file_dup(filep, va_arg(ap, int));
        }
        break;

      case F_GETFD:
        /* Get the file descriptor flags defined in <fcntl.h> that are associated
         * with the file descriptor fd.  File descriptor flags are associated
         * with a single file descriptor and do not affect other file descriptors
         * that refer to the same file.
         */

        {
          ret = (filep->f_oflags & O_CLOEXEC) ? FD_CLOEXEC : 0;
        }
        break;

      case F_SETFD:
        /* Set the file descriptor flags defined in <fcntl.h>, that are associated
         * with fd, to the third argument, arg, taken as type int. If the
         * FD_CLOEXEC flag in the third argument is 0, the file shall remain open
         * across the exec functions; otherwise, the file shall be closed upon
         * successful execution of one  of  the  exec  functions.
         */

        {
          int oflags = va_arg(ap, int);

          if (oflags & FD_CLOEXEC)
            {
              filep->f_oflags |= O_CLOEXEC;
            }
          else
            {
              err = EPERM; /* Not support */
            }
        }
        break;

      case F_GETFL:
        /* Get the file status flags and file access modes, defined in
         * <fcntl.h>, for the file description associated with fd. The file
         * access modes can be extracted from the return value using the
         * mask O_ACCMODE, which is defined  in <fcntl.h>. File status flags
         * and file access modes are associated with the file description
         * and do not affect other file descriptors that refer to the same
         * file with different open file descriptions.
         */

        {
          ret = filep->f_oflags;
        }
        break;

      case F_SETFL:
        /* Set the file status flags, defined in <fcntl.h>, for the file
         * description associated with fd from the corresponding  bits in
         * the third argument, arg, taken as type int. Bits corresponding
         * to the file access mode and the file creation flags, as defined
         * in <fcntl.h>, that are set in arg shall be ignored. If any bits
         * in arg other than those mentioned here are changed by the
         * application, the result is unspecified.
         */

        {
          int oflags = va_arg(ap, int);

          oflags          &=  FFCNTL;
          filep->f_oflags &= ~FFCNTL;
          filep->f_oflags |= oflags;
        }
        break;

      case F_GETOWN:
        /* If fd refers to a socket, get the process or process group ID
         * specified to receive SIGURG signals when out-of-band data is
         * available. Positive values indicate a process ID; negative
         * values, other than -1, indicate a process group ID. If fd does
         * not refer to a socket, the results are unspecified.
         */

      case F_SETOWN:
        /* If fd refers to a socket, set the process or process group ID
         * specified to receive SIGURG signals when out-of-band data is
         * available, using the value of the third argument, arg, taken as
         * type int. Positive values indicate a process ID; negative values,
         * other than -1, indicate a process group ID.  If fd does not refer
         * to a socket, the results are unspecified.
         */

        err = EBADF; /* Only valid on socket descriptors */
        break;

      case F_GETLK:
        /* Get the first lock which blocks the lock description pointed to
         * by the third argument, arg, taken as a pointer to type struct
         * flock, defined in <fcntl.h>.  The information retrieved shall
         * overwrite the information passed to fcntl() in the structure
         * flock. If no lock is found that would prevent this lock from
         * being created, then the structure shall be left unchanged except
         * for the lock type which shall be set to F_UNLCK.
         */

      case F_SETLK:
        /* Set or clear a file segment lock according to the lock
         * description pointed to by the third argument, arg, taken as a
         * pointer to type struct flock, defined in <fcntl.h>. F_SETLK can
         * establish shared (or read) locks (F_RDLCK) or exclusive (or
         * write) locks (F_WRLCK), as well  as to remove either type of lock
         * (F_UNLCK).  F_RDLCK, F_WRLCK, and F_UNLCK are defined in
         * <fcntl.h>. If a shared or exclusive lock cannot be set, fcntl()
         * shall return immediately with a return value of -1.
         */

      case F_SETLKW:
        /* This command shall be equivalent to F_SETLK except that if a
         * shared or exclusive lock is blocked by other locks, the thread
         * shall wait until the request can be satisfied. If a signal that
         * is to be caught is received while fcntl() is waiting for a
         * region, fcntl() shall be interrupted. Upon return from the signal
         * handler, fcntl() shall return -1 with errno set to [EINTR], and
         * the lock operation shall not be done.
         */

        err = ENOSYS; /* Not implemented */
        break;

      default:
        err = EINVAL;
        break;
    }

errout:
  if (err != 0)
    {
      set_errno(err);
      return VFS_ERROR;
    }

  return ret;
}
#endif /* CONFIG_NFILE_DESCRIPTORS > 0 */

/****************************************************************************
 * Name: fcntl
 *
 * Description:
 *   fcntl() will perform the operation specified by 'cmd' on an open file.
 *
 * Input Parameters:
 *   fd  - File descriptor of the open file
 *   cmd - Identifies the operation to be performed.  Command specific
 *         arguments may follow.
 *
 * Returned Value:
 *   The returned value depends on the nature of the command but for all
 *   commands the return value of -1 (ERROR) indicates that an error has
 *   occurred and, in this case, the errno variable will be set
 *   appropriately
 *
 ****************************************************************************/

int fcntl(int fd, int cmd, ...)
{
  struct file *filep = NULL;
  va_list ap;
  int ret;
  int val = 0;

  /* Setup to access the variable argument list */

  va_start(ap, cmd);

  /* Did we get a valid file descriptor? */

#if CONFIG_NFILE_DESCRIPTORS > 0
  if ((unsigned int)fd < CONFIG_NFILE_DESCRIPTORS)
    {
      /* Get the file structure corresponding to the file descriptor. */

      ret = fs_getfilep(fd, &filep);
      if (ret < 0)
        {
          /* The errno value has already been set */
          va_end(ap);
          return VFS_ERROR;
        }

          /* Let file_vfcntl() do the real work.  The errno is not set on
           * failures.
           */

      ret = file_vfcntl(filep, cmd, ap);
    }
  else
#endif
    {
      /* No... check for operations on a socket descriptor */

#if defined(LOSCFG_NET_LWIP_SACK)
      if ((unsigned int)fd < (unsigned int)(CONFIG_NFILE_DESCRIPTORS+CONFIG_NSOCKET_DESCRIPTORS))
        {
          /* Yes.. defer socket descriptor operations to net_vfcntl() */

          val = va_arg(ap, int);
          ret = lwip_fcntl(fd, cmd, val);
        }
      else
#endif
        {
          /* No.. this descriptor number is out of range */

          (void)val;
          ret = EBADF;
          set_errno(ret);
          va_end(ap);
          return VFS_ERROR;
        }
    }

  va_end(ap);
  return ret;
}

int fcntl64(int fd, int cmd, ...)
{
  struct file *filep = NULL;
  va_list va_ap;
  int reval;
  int va_val = 0;

  /* Setup to access the variable argument list */

  va_start(va_ap, cmd);

  /* Did we get a valid file descriptor? */

#if CONFIG_NFILE_DESCRIPTORS > 0
  if ((unsigned int)fd < CONFIG_NFILE_DESCRIPTORS)
    {
      /* Get the file structure corresponding to the file descriptor. */

      int ret = fs_getfilep(fd, &filep);
      if (ret < 0)
        {
          /* The errno value has already been set */
          va_end(va_ap);
          return VFS_ERROR;
        }

      /* Let file_vfcntl() do the real work */

      reval = file_vfcntl(filep, cmd, va_ap);
    }
  else
#endif
    {
      /* No... check for operations on a socket descriptor */

#if defined(LOSCFG_NET_LWIP_SACK)
      if ((unsigned int)fd < (unsigned int)(CONFIG_NFILE_DESCRIPTORS+CONFIG_NSOCKET_DESCRIPTORS))
        {
          /* Yes.. defer socket descriptor operations to net_vfcntl() */

          va_val = va_arg(va_ap, int);
          reval = lwip_fcntl(fd, cmd, va_val);
        }
      else
#endif
        {
          /* No.. this descriptor number is out of range */

          (void)va_val;
          reval = EBADF;
          set_errno(reval);
          va_end(va_ap);
          return VFS_ERROR;
        }
    }

  va_end(va_ap);
  return reval;
}
