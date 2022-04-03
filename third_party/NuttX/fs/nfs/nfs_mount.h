/****************************************************************************
 * fs/nfs/nfs_mount.h
 *
 *   Copyright (C) 2012 Gregory Nutt. All rights reserved.
 *   Copyright (C) 2012 Jose Pablo Rojas Vargas. All rights reserved.
 *   Author: Jose Pablo Rojas Vargas <jrojas@nx-engineering.com>
 *           Gregory Nutt <gnutt@nuttx.org>
 *
 * Leveraged from OpenBSD:
 *
 *  Copyright (c) 1989, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __FS_NFS_NFS_MOUNT_H
#define __FS_NFS_NFS_MOUNT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/socket.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <pthread.h>
#include "rpc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* NFS mount option flags */

#define NFSMNT_SOFT              (1 << 0)      /* Soft mount (hard is default) */
#define NFSMNT_WSIZE             (1 << 1)      /* Set write size */
#define NFSMNT_RSIZE             (1 << 2)      /* Set read size */
#define NFSMNT_TIMEO             (1 << 3)      /* Set initial timeout */
#define NFSMNT_RETRANS           (1 << 4)      /* Set number of request retries */
#define NFSMNT_READDIRSIZE       (1 << 5)      /* Set readdir size */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Mount structure. One mount structure is allocated for each NFS mount. This
 * structure holds NFS specific information for mount.
 */

struct nfsmount
{
  struct nfsnode  *nm_head;                   /* A list of all files opened on this mountpoint */
  struct nfsdir_s *nm_dir;                    /* A list of all directories opened on this mountpoint */
  pthread_mutex_t  nm_mux;                    /* Used to assure thread-safe access */
  nfsfh_t          nm_fh;                     /* File handle of root dir */
  char             nm_path[NFS_MOUNT_PATH_MAX_SIZE];  /* server's path of the directory being mounted */
  struct nfs_fattr nm_fattr;                  /* nfs file attribute cache */
  struct rpcclnt  *nm_rpcclnt;                /* RPC state */
  int32_t          nm_so;                     /* RPC socket */
  struct sockaddr  nm_nam;                    /* Addr of server */
  bool             nm_mounted;                /* true: The file system is ready */
  uint8_t          nm_fhsize;                 /* Size of root file handle (host order) */
  uint8_t          nm_sotype;                 /* Type of socket */
  uint8_t          nm_retry;                  /* Max retries */
  uint32_t         nm_timeo;                  /* Timeout value (in system clock ticks) */
  uint16_t         nm_rsize;                  /* Max size of read RPC */
  uint16_t         nm_wsize;                  /* Max size of write RPC */
  uint16_t         nm_readdirsize;            /* Size of a readdir RPC */
  uint16_t         nm_buflen;                 /* Size of I/O buffer */
  mode_t           nm_permission;
  uint             nm_gid;
  uint             nm_uid;

  /* Set aside memory on the stack to hold the largest call message.  NOTE
   * that for the case of the write call message, it is the reply message that
   * is in this union.
   */

  union
  {
    struct rpc_call_pmap    pmap;
    struct rpc_call_mount   mountd;
    struct rpc_call_create  create;
    struct rpc_call_lookup  lookup;
    struct rpc_call_read    read;
    struct rpc_call_remove  removef;
    struct rpc_call_rename  renamef;
    struct rpc_call_mkdir   mkdir;
    struct rpc_call_rmdir   rmdir;
    struct rpc_call_readdir readdir;
    struct rpc_call_fs      fsstat;
    struct rpc_call_setattr setattr;
    struct rpc_call_fs      fs;
    struct rpc_reply_write  write;
  } nm_msgbuffer;

  /* I/O buffer (must be a aligned to 32-bit boundaries).  This buffer used for all
   * reply messages EXCEPT for the WRITE RPC. In that case it is used for the WRITE
   * call message that contains the data to be written.  This buffer must be
   * dynamically sized based on the characteristics of the server and upon the
   * configuration of the NuttX network.  It must be sized to hold the largest
   * possible WRITE call message or READ response message.
   */

  uint32_t         nm_iobuffer[1];            /* Actual size is given by nm_buflen */
};

/* The size of the nfsmount structure will debug on the size of the allocated I/O
 * buffer.
 */

#define SIZEOF_nfsmount(n) (sizeof(struct nfsmount) + ((n + 3) & ~3) - sizeof(uint32_t))

/* Mount parameters structure. This structure is use in nfs_decode_args funtion before one
 * mount structure is allocated in each NFS mount.
 */

struct nfs_mount_parameters
{
  uint32_t         timeo;                  /* Timeout value (in deciseconds) */
  uint8_t          retry;                  /* Max retries */
  uint16_t         rsize;                  /* Max size of read RPC */
  uint16_t         wsize;                  /* Max size of write RPC */
  uint16_t         readdirsize;            /* Size of a readdir RPC */
};

struct nfs_args
{
  uint8_t         addrlen;               /* Length of address */
  uint8_t         sotype;                /* Socket type */
  uint8_t         flags;                 /* Flags, determines if following are valid: */
  uint8_t         timeo;                 /* Time value in deciseconds (with NFSMNT_TIMEO) */
  uint8_t         retrans;               /* Times to retry send (with NFSMNT_RETRANS) */
  uint16_t        wsize;                 /* Write size in bytes (with NFSMNT_WSIZE) */
  uint16_t        rsize;                 /* Read size in bytes (with NFSMNT_RSIZE) */
  uint16_t        readdirsize;           /* readdir size in bytes (with NFSMNT_READDIRSIZE) */
  char            *path;                 /* Server's path of the directory being mount */
  struct sockaddr addr;                  /* File server address (requires 32-bit alignment) */
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
