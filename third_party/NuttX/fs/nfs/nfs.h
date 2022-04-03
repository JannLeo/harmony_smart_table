/****************************************************************************
 * fs/nfs/nfs.h
 *
 *   Copyright (C) 2012 Gregory Nutt. All rights reserved.
 *   Copyright (C) 2012 Jose Pablo Rojas Vargas. All rights reserved.
 *   Author: Jose Pablo Rojas Vargas <jrojas@nx-engineering.com>
 *           Gregory Nutt <gnutt@nuttx.org>
 *
 * Leveraged from OpenBSD:
 *
 *   Copyright (c) 1989, 1993, 1995
 *   The Regents of the University of California.  All rights reserved.
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
 * 3. Neither the name of the University nor the names of its contributors
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

#ifndef __FS_NFS_NFS_H
#define __FS_NFS_NFS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "nfs_mount.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define NFS_TICKS          1              /* Number of system ticks */
#define NFS_HZ             CLOCKS_PER_SEC /* Ticks/sec */
#define NFS_TIMEO          (1 * NFS_HZ)   /* Default timeout = 1 second */
#define NFS_MINTIMEO       (1 * NFS_HZ)   /* Min timeout to use */
#define NFS_MAXTIMEO       (60 * NFS_HZ)  /* Max timeout to backoff to */
#define NFS_TIMEOUTMUL     2              /* Timeout/Delay multiplier */
#define NFS_MAXREXMIT      10             /* Stop counting after this many */
#define NFS_RETRANS        5              /* Num of retrans for soft mounts */
#define NFS_WSIZE          (8192 * 4)     /* Def. write data size <= 8192 */
#define NFS_RSIZE          (8192 * 4)     /* Def. read data size <= 8192 */
#define NFS_READDIRSIZE    1024           /* Def. readdir size */
#define NFS_NPROCS         23

/* Ideally, NFS_DIRBLKSIZ should be bigger, but I've seen servers with
 * broken NFS/ethernet drivers that won't work with anything bigger (Linux..)
 */

#define NFS_DIRBLKSIZ      1024           /* Must be a multiple of DIRBLKSIZ */

/* Increment NFS statistics */

#ifdef CONFIG_NFS_STATISTICS
#  define nfs_statistics(n) do { nfsstats.rpccnt[n]++; } while (0)
#else
#  define nfs_statistics(n)
#endif

/****************************************************************************
 *  Public Data
 ****************************************************************************/

typedef void (*NFSMOUNT_HOOK)(struct nfs_args*);
extern uint32_t nfs_true;
extern uint32_t nfs_false;
extern NFSMOUNT_HOOK g_NFSMOUNT_HOOK;
extern uint32_t nfs_xdrneg1;
#ifdef CONFIG_NFS_STATISTICS
extern struct nfsstats nfsstats;
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* NFS statistics structure */

struct nfsstats
{
  uint64_t rpccnt[NFS_NPROCS];
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
extern void nfs_mux_take(struct nfsmount *nmp);
extern void nfs_mux_release(struct nfsmount *nmp);
extern int  nfs_checkmount(struct nfsmount *nmp);
extern int  nfs_fsinfo(struct nfsmount *nmp);
extern int nfs_request(struct nfsmount *nmp, int procnum,
                void *request, size_t reqlen,
                void *response, size_t resplen);
extern int  nfs_lookup(struct nfsmount *nmp, const char *filename,
              struct file_handle *fhandle,
              struct nfs_fattr *obj_attributes,
              struct nfs_fattr *dir_attributes);
extern int  nfs_findnode(struct nfsmount *nmp, const char *relpath,
              struct file_handle *fhandle,
              struct nfs_fattr *obj_attributes,
              struct nfs_fattr *dir_attributes);
extern int nfs_finddir(struct nfsmount *nmp, const char *relpath,
              struct file_handle *fhandle,
              struct nfs_fattr *attributes, char *filename);
extern void nfs_attrupdate(struct nfsnode *np,
              struct nfs_fattr *attributes);
extern int nfs_mount(const char *server_ip_and_path, const char *mount_path,
              unsigned int uid, unsigned int gid);


#ifdef __cplusplus
#if __cplusplus
}
#endif /*__cplusplus */
#endif /*__cplusplus */

#endif /* _NFS_NFS_H */
