/****************************************************************************
 * fs/vfs/fs_ioctl.c
 *
 *   Copyright (C) 2007-2010, 2012-2014, 2016-2017 Gregory Nutt. All rights
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
#include "sched.h"
#include "assert.h"
#include "errno.h"
#include "unistd.h"
#include "console.h"

#if defined(LOSCFG_NET_LWIP_SACK)
#include "lwip/sockets.h"
#endif

#include "vnode.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ioctl/fs_ioctl
 *
 * Description:
 *   Perform device specific operations.
 *
 * Input Parameters:
 *   fd       File/socket descriptor of device
 *   req      The ioctl command
 *   arg      The argument of the ioctl cmd
 *
 * Returned Value:
 *   >=0 on success (positive non-zero values are cmd-specific)
 *   -1 on failure with errno set properly:
 *
 *   EBADF
 *     'fd' is not a valid descriptor.
 *   EFAULT
 *     'arg' references an inaccessible memory area.
 *   EINVAL
 *     'cmd' or 'arg' is not valid.
 *   ENOTTY
 *     'fd' is not associated with a character special device.
 *   ENOTTY
 *      The specified request does not apply to the kind of object that the
 *      descriptor 'fd' references.
 *
 ****************************************************************************/

#ifdef CONFIG_LIBC_IOCTL_VARIADIC
int fs_ioctl(int fd, int req, ...)
#else
int ioctl(int fd, int req, ...)
#endif
{
  int err;
  UINTPTR arg = 0;
  va_list ap;
#if CONFIG_NFILE_DESCRIPTORS > 0
  struct file     *filep;
  int                  ret = OK;
#endif

  va_start(ap, req);
  arg = va_arg(ap, UINTPTR);
  va_end(ap);
#if CONFIG_NFILE_DESCRIPTORS > 0

  /* Did we get a valid file descriptor? */

  if ((unsigned int)fd >= CONFIG_NFILE_DESCRIPTORS)
#endif
    {
      /* Perform the socket ioctl */

#if defined(LOSCFG_NET_LWIP_SACK)
      if ((unsigned int)fd < (unsigned int)(CONFIG_NFILE_DESCRIPTORS+CONFIG_NSOCKET_DESCRIPTORS))
        {
          return socks_ioctl(fd, (long)req, (void *)arg);
        }
      else
#endif
        {
          err = EBADF;
          ret = VFS_ERROR;
          goto errout;
        }
    }

#if CONFIG_NFILE_DESCRIPTORS > 0

  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) /* fd : [0,2] */
    {
      fd = ConsoleUpdateFd();
      if (fd < 0)
        {
          err = EBADF;
          ret = VFS_ERROR;
          goto errout;
        }
    }

  /* Get the file structure corresponding to the file descriptor. */

  ret = fs_getfilep(fd, &filep);
  if (ret < 0)
    {
      /* The errno value has already been set */
      return VFS_ERROR;
    }

  /* Is a driver registered? Does it support the ioctl method? */

  if (filep->ops && filep->ops->ioctl)
    {
      /* Yes, then let it perform the ioctl */

      ret = (int) filep->ops->ioctl(filep, req, arg);
      if (ret < 0)
        {
          err = -ret;
          goto errout;
        }
    }
  else
    {
      err = ENOSYS;
      ret = VFS_ERROR;
      goto errout;
    }

  return ret;
#endif

errout:
  set_errno(err);
  return VFS_ERROR;
}
