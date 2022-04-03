/****************************************************************************
 * fs/nfs/nfs_vfsops.c
 *
 *   Copyright (C) 2012-2013, 2015, 2017-2018 Gregory Nutt. All rights reserved.
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "vfs_config.h"
#include "dirent.h"
#include "fs/fs.h"
#include "fs/dirent_fs.h"
#include "nfs.h"
#include "nfs_node.h"
#include "xdr_subs.h"
#include "los_tables.h"
#include "vnode.h"
#include "los_vm_filemap.h"
#include "user_copy.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/****************************************************************************
 * Public Data
 ****************************************************************************/

uint32_t nfs_true;
uint32_t nfs_false;
uint32_t nfs_xdrneg1;
NFSMOUNT_HOOK g_NFSMOUNT_HOOK = (NFSMOUNT_HOOK)(UINTPTR)NULL;

#ifdef CONFIG_NFS_STATISTICS
struct nfsstats nfsstats;
#endif

#define USE_GUARDED_CREATE 1

#ifdef LOSCFG_FS_NFS
/****************************************************************************
 * Private Type Definitions
 ****************************************************************************/

#define NFS_DIR_ENTRY_MALLOC(entry)                                           \
  do                                                                          \
    {                                                                         \
      entry = (struct entry3 *)malloc(sizeof(struct entry3));                 \
      if (entry == NULL)                                                      \
        {                                                                     \
          PRINT_DEBUG("malloc failed\n");                                           \
          error = ENOMEM;                                                     \
          goto errout_with_memory;                                            \
        }                                                                     \
      (void)memset_s(entry, sizeof(struct entry3), 0, sizeof(struct entry3)); \
    }                                                                         \
  while (0)

#define NFS_DIR_ENTRY_FREE(entry)  \
  do                               \
    {                              \
      free(entry->contents);       \
      entry->contents = NULL;      \
      free(entry);                 \
      entry = NULL;                \
    }                              \
  while (0)

#define FILENAME_MAX_LEN 50
struct MountOps nfs_mount_operations;
struct VnodeOps nfs_vops;
struct file_operations_vfs nfs_fops;
struct nfs_statinfo_s
{
  uint16_t ns_mode;    /* File access mode */
  uint8_t  ns_type;    /* File type */
  uint64_t ns_size;    /* File size */
  time_t   ns_atime;   /* Time of last access */
  time_t   ns_mtime;   /* Time of last modification */
  time_t   ns_ctime;   /* Time of last status change */
};
extern void nfs_stat_common(struct nfs_statinfo_s *info, struct stat *buf);

static mode_t type_to_mode(int type, mode_t permission)
{
  switch (type)
    {
    case VNODE_TYPE_DIR:
      return permission | S_IFDIR;
    case VNODE_TYPE_REG:
      return permission | S_IFREG;
    case VNODE_TYPE_BLK:
      return permission | S_IFBLK;
    case VNODE_TYPE_CHR:
      return permission | S_IFCHR;
    case VNODE_TYPE_FIFO:
      return permission | S_IFIFO;
    default:
      break;
    }
  return permission;
}

static int nfs_2_vfs(int result)
{
  int status;

  if ((result < NFS_OK) || (result > NFSERR_NOTEMPTY))
    {
      return result;
    }

  /* Nfs errno to Libc errno */
  switch (result)
    {
    case NFSERR_NAMETOL:
      status = ENAMETOOLONG;
      break;
    case NFSERR_NOTEMPTY:
      status = ENOTEMPTY;
      break;
    default:
      status = result;
      break;
    }

  return status;
}

/****************************************************************************
 * Name: nfs_fileupdate
 *
 * Description:
 *   This is to update the file attributes like size, type. This sends a LOOKUP msg of nfs
 *   to get latest file attributes.
 *
 * Returned Value:
 *   0 on success; a positive errno value on failure.
 *
 ****************************************************************************/
static int nfs_fileupdate(struct nfsmount *nmp, char *filename,
    struct file_handle *parent_fhandle, struct nfsnode *np)
{
  struct file_handle fhandle;
  int                error;
  struct nfs_fattr   fattr;

  /* Find the NFS node associate with the path */

  fhandle.length = parent_fhandle->length;
  (void)memcpy_s(&(fhandle.handle), fhandle.length, &(parent_fhandle->handle), parent_fhandle->length);
  error = nfs_lookup(nmp, filename, &fhandle, &fattr, NULL);

  if (error != OK)
    {
      PRINTK("ERROR: nfs_lookup failed returned: %d\n", error);
      return error;
    }

  /* Update the file handle */

  error = memcpy_s(&(np->n_fhandle), NFSX_V3FHMAX, &(fhandle.handle), fhandle.length);
  if (error != EOK)
    {
      return ENOBUFS;
    }

  np->n_fhsize = fhandle.length;

  /* Save the file attributes */

  nfs_attrupdate(np, &fattr);

  return OK;
}

int vfs_nfs_reclaim(struct Vnode *node)
{
  struct nfsnode  *prev = NULL;
  struct nfsnode  *curr = NULL;
  struct nfsmount *nmp = NULL;
  struct nfsnode  *np = NULL;
  if (node->data == NULL)
    {
      return OK;
    }
  nmp = (struct nfsmount *)(node->originMount->data);
  nfs_mux_take(nmp);
  np = (struct nfsnode*)(node->data);
  int ret;

  if (np->n_crefs > 1)
    {
      np->n_crefs--;
      ret = OK;
    }

  /* There are no more references to the file structure.  Now we need to
   * free up all resources associated with the open file.
   *
   * First, find our file structure in the list of file structures
   * containted in the mount structure.
   */

  else
    {
      /* Assume file structure will not be found.  This should never happen. */

      ret = -EINVAL;

      for (prev = NULL, curr = nmp->nm_head;
          curr;
          prev = curr, curr = curr->n_next)
        {
          /* Check if this node is ours */

          if (np == curr)
            {
              /* Yes.. remove it from the list of file structures */

              if (prev)
                {
                  /* Remove from mid-list */

                  prev->n_next = np->n_next;
                }
              else
                {
                  /* Remove from the head of the list */

                  nmp->nm_head = np->n_next;
                }

              /* Then deallocate the file structure and return success */

              free(np->n_name);
              free(np);
              ret = OK;
              break;
            }
        }
    }

  nfs_mux_release(nmp);
  return ret;
}

static int vfs_nfs_stat_internal(struct nfsmount *nmp, struct nfsnode *nfs_node)
{
  int ret;
  struct timespec ts;
  struct rpc_call_fs attr_call;
  struct rpc_reply_getattr attr_reply;
  attr_call.fs.fsroot.length = txdr_unsigned(nfs_node->n_fhsize);
  memcpy_s(&(attr_call.fs.fsroot.handle), sizeof(nfsfh_t), &(nfs_node->n_fhandle), sizeof(nfsfh_t));
  ret = nfs_request(nmp, NFSPROC_GETATTR, &attr_call,
      sizeof(struct file_handle), &attr_reply,
      sizeof(struct rpc_reply_getattr));
  if (ret != OK)
    {
      return ret;
    }
  /* Extract the file mode, file type, and file size. */

  nfs_node->n_mode  = fxdr_unsigned(uint16_t, attr_reply.attr.fa_mode);
  nfs_node->n_type  = fxdr_unsigned(uint8_t, attr_reply.attr.fa_type);
  nfs_node->n_size  = fxdr_hyper(&attr_reply.attr.fa_size);

  /* Extract time values as type time_t in units of seconds */

  fxdr_nfsv3time(&attr_reply.attr.fa_mtime, &ts);
  nfs_node->n_mtime = ts.tv_sec;

  fxdr_nfsv3time(&attr_reply.attr.fa_atime, &ts);
  nfs_node->n_atime = ts.tv_sec;

  fxdr_nfsv3time(&attr_reply.attr.fa_ctime, &ts);
  nfs_node->n_ctime = ts.tv_sec;

  return OK;
}

/****************************************************************************
 * Name: nfs_decode_args
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void nfs_decode_args(struct nfs_mount_parameters *nprmt,
                            struct nfs_args *argp)
{
  int maxio;

  /* Get the selected timeout value */

  if ((argp->flags & NFSMNT_TIMEO) != 0 && argp->timeo > 0)
    {
      uint32_t tmp = ((uint32_t)argp->timeo * NFS_HZ + 5) / 10;
      if (tmp < NFS_MINTIMEO)
        {
          tmp = NFS_MINTIMEO;
        }
      else if (tmp > NFS_MAXTIMEO)
        {
          tmp = NFS_MAXTIMEO;
        }

      nprmt->timeo = tmp;
    }

  /* Get the selected retransmission count */

  if ((argp->flags & NFSMNT_RETRANS) != 0 && argp->retrans > 1)
    {
      if  (argp->retrans < NFS_MAXREXMIT)
        {
          nprmt->retry = argp->retrans;
        }
      else
        {
          nprmt->retry = NFS_MAXREXMIT;
        }
    }

  if ((argp->flags & NFSMNT_SOFT) == 0)
    {
      nprmt->retry = NFS_MAXREXMIT + 1;  /* Past clip limit */
    }

  /* Get the maximum amount of data that can be transPRINT_ERRed in one packet */

  if ((argp->sotype == SOCK_DGRAM) != 0)
    {
      maxio = NFS_MAXDGRAMDATA;
    }
  else
    {
      PRINT_ERR("Only SOCK_DRAM is supported\n");
      maxio = NFS_MAXDATA;
    }

  /* Get the maximum amount of data that can be transPRINT_ERRed in one write transfer */

  if ((argp->flags & NFSMNT_WSIZE) != 0 && argp->wsize > 0)
    {
      nprmt->wsize = argp->wsize;

      /* Round down to multiple of blocksize */

      nprmt->wsize &= ~(NFS_FABLKSIZE - 1);
      if (nprmt->wsize <= 0)
        {
          nprmt->wsize = NFS_FABLKSIZE;
        }
    }

  if (nprmt->wsize > maxio)
    {
      nprmt->wsize = maxio;
    }

  if (nprmt->wsize > MAXBSIZE)
    {
      nprmt->wsize = MAXBSIZE;
    }

  /* Get the maximum amount of data that can be transPRINT_ERRed in one read transfer */

  if ((argp->flags & NFSMNT_RSIZE) != 0 && argp->rsize > 0)
    {
      nprmt->rsize = argp->rsize;

      /* Round down to multiple of blocksize */

      nprmt->rsize &= ~(NFS_FABLKSIZE - 1);
      if (nprmt->rsize <= 0)
        {
          nprmt->rsize = NFS_FABLKSIZE;
        }
    }

  if (nprmt->rsize > maxio)
    {
      nprmt->rsize = maxio;
    }

  if (nprmt->rsize > MAXBSIZE)
    {
      nprmt->rsize = MAXBSIZE;
    }

  /* Get the maximum amount of data that can be transPRINT_ERRed in directory transfer */

  if ((argp->flags & NFSMNT_READDIRSIZE) != 0 && argp->readdirsize > 0)
    {
      nprmt->readdirsize = argp->readdirsize;

      /* Round down to multiple of blocksize */

      nprmt->readdirsize &= ~(NFS_DIRBLKSIZ - 1);
      if (nprmt->readdirsize < NFS_DIRBLKSIZ)
        {
          nprmt->readdirsize = NFS_DIRBLKSIZ;
        }
    }
  else if (argp->flags & NFSMNT_RSIZE)
    {
      nprmt->readdirsize = nprmt->rsize;
    }

  if (nprmt->readdirsize > maxio)
    {
      nprmt->readdirsize = maxio;
    }
}

/****************************************************************************
 * Name: nfs_bind
 *
 * Description:
 *  This implements a portion of the mount operation. This function allocates
 *  and initializes the mountpoint private data and gets mount information
 *  from the NFS server.  The final binding of the private data (containing
 *  NFS server mount information) to the  mountpoint is performed by mount().
 *
 * Returned Value:
 *   0 on success; a negated errno value on failure.
 *
 ****************************************************************************/

int nfs_bind(struct Vnode *blkdriver, const void *data,
                    void **handle, const char *relpath)
{
  struct nfs_args        *argp = (struct nfs_args *)data;
  struct nfsmount        *nmp = NULL;
  struct rpcclnt             *rpc = NULL;
  struct rpc_call_fs          getattr;
  struct rpc_reply_getattr    resok;
  struct nfs_mount_parameters nprmt;
  uint32_t                    buflen;
  uint32_t                    pathlen;
  uint32_t                    tmp;
  int                         error = 0;
  pthread_mutexattr_t attr;

  DEBUGASSERT(data && handle);

  /* Set default values of the parameters.  These may be overridden by
   * settings in the argp->flags.
   */

  nprmt.timeo       = NFS_TIMEO;
  nprmt.retry       = NFS_RETRANS;
  nprmt.wsize       = NFS_WSIZE;
  nprmt.rsize       = NFS_RSIZE;
  nprmt.readdirsize = NFS_READDIRSIZE;

  nfs_decode_args(&nprmt, argp);

  /* Determine the size of a buffer that will hold one RPC data transfer.
   * First, get the maximum size of a read and a write transfer.
   */

  pathlen = strlen(argp->path);
  if (pathlen >= NFS_MOUNT_PATH_MAX_SIZE) {
      return -ENAMETOOLONG;
  }

  buflen = SIZEOF_rpc_call_write(nprmt.wsize);
  tmp    = SIZEOF_rpc_reply_read(nprmt.rsize);

  /* The buffer size will be the maximum of those two sizes */

  if (tmp > buflen)
    {
      buflen = tmp;
    }

  /* But don't let the buffer size exceed the MSS of the socket type.
   *
   * In the case where there are multiple network devices with different
   * link layer protocols, each network device may support a different
   * UDP MSS value.  Here we arbitrarily select the minimum MSS for
   * that case.
   */

  /* Create an instance of the mountpt state structure */

  nmp = (struct nfsmount *)malloc(SIZEOF_nfsmount(buflen));
  if (!nmp)
    {
      PRINT_ERR("Failed to allocate mountpoint structure\n");
      return -ENOMEM;
    }

  (void)memset_s(nmp, SIZEOF_nfsmount(buflen), 0, SIZEOF_nfsmount(buflen));

  /* Save the allocated I/O buffer size */

  nmp->nm_buflen = (uint16_t)buflen;

  nmp->nm_so = -1;

  /* Initialize the allocated mountpt state structure. */

  (void)pthread_mutexattr_init(&attr);
  (void)pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  error = pthread_mutex_init(&nmp->nm_mux, &attr);
  if (error)
    {
      return -error;
    }

  /* Initialize NFS */

  nfs_true = txdr_unsigned(TRUE);
  nfs_false = txdr_unsigned(FALSE);
  nfs_xdrneg1 = txdr_unsigned(-1);

  rpcclnt_init();

  /* Set initial values of other fields */

  nmp->nm_timeo       = nprmt.timeo;
  nmp->nm_retry       = nprmt.retry;
  nmp->nm_wsize       = nprmt.wsize;
  nmp->nm_rsize       = nprmt.rsize;
  nmp->nm_readdirsize = nprmt.readdirsize;
  nmp->nm_fhsize      = NFSX_V3FHMAX;

  (void)strncpy_s(nmp->nm_path, sizeof(nmp->nm_path), argp->path, pathlen);
  (void)memcpy_s(&nmp->nm_nam, sizeof(struct sockaddr), &argp->addr, argp->addrlen);

  /* Set up the sockets and per-host congestion */

  nmp->nm_sotype  = argp->sotype;

  if (nmp->nm_sotype == SOCK_DGRAM || nmp->nm_sotype == SOCK_STREAM)
    {
      /* Connection-less... connect now */

      /* Create an instance of the rpc state structure */

      rpc = (struct rpcclnt *)malloc(sizeof(struct rpcclnt));
      if (!rpc)
        {
          PRINT_ERR("Failed to allocate rpc structure\n");
          error = ENOMEM;
          goto bad;
        }

      (void)memset_s(rpc, sizeof(struct rpcclnt), 0, sizeof(struct rpcclnt));

      PRINT_INFO("Connecting\n");

      /* Translate nfsmnt flags -> rpcclnt flags */

      rpc->rc_path       = nmp->nm_path;
      rpc->rc_name       = &nmp->nm_nam;
      rpc->rc_sotype     = nmp->nm_sotype;
      rpc->rc_retry      = nmp->nm_retry;
      rpc->rc_so         = -1;

      nmp->nm_rpcclnt    = rpc;

      error = rpcclnt_connect(nmp->nm_rpcclnt);
      if (error != OK)
        {
          PRINTK("ERROR: nfs_connect failed: %d\n", error);
          goto bad;
        }
    }

  nmp->nm_mounted        = true;
  nmp->nm_so             = nmp->nm_rpcclnt->rc_so;
  nmp->nm_head           = NULL;
  nmp->nm_dir            = NULL;
  nmp->nm_fhsize         = nmp->nm_rpcclnt->rc_fhsize;
  (void)memcpy_s(&nmp->nm_fh, sizeof(nfsfh_t), &nmp->nm_rpcclnt->rc_fh, sizeof(nfsfh_t));

  /* Get the file attributes */

  getattr.fs.fsroot.length = txdr_unsigned(nmp->nm_fhsize);
  (void)memcpy_s(&getattr.fs.fsroot.handle, sizeof(nfsfh_t), &nmp->nm_fh, sizeof(nfsfh_t));

  error = nfs_request(nmp, NFSPROC_GETATTR,
                      (void *)&getattr, /* sizeof(struct FS3args) */
                      (sizeof(getattr.fs.fsroot.length) + nmp->nm_fhsize),
                      (void *)&resok, sizeof(struct rpc_reply_getattr));
  if (error)
    {
      PRINT_ERR("nfs_request failed: %d\n", error);
      goto bad;
    }

  /* Save the file attributes */

  (void)memcpy_s(&nmp->nm_fattr, sizeof(struct nfs_fattr), &resok.attr, sizeof(struct nfs_fattr));

  /* Mounted! */

  *handle = (void *)nmp;

  PRINT_INFO("Successfully mounted\n");
  return OK;

bad:
  if (nmp)
    {
      /* Disconnect from the server */

      if (nmp->nm_rpcclnt)
        {
          rpcclnt_disconnect(nmp->nm_rpcclnt);
          free(nmp->nm_rpcclnt);
          nmp->nm_rpcclnt = NULL;
        }

      /* Free connection-related resources */

      (void)pthread_mutex_destroy(&nmp->nm_mux);

      free(nmp);
      nmp = NULL;
    }

  return -error; /*lint !e438*/
}


static enum VnodeType filetype_to_vnodetype(uint32_t filetype)
{
  if (filetype < 0 || filetype > 7)
    {
      return VNODE_TYPE_UNKNOWN;
    }

  enum VnodeType transfer_table[8] =
    {
      VNODE_TYPE_UNKNOWN,
      VNODE_TYPE_REG,
      VNODE_TYPE_DIR,
      VNODE_TYPE_BLK,
      VNODE_TYPE_CHR,
      VNODE_TYPE_LNK,
      VNODE_TYPE_UNKNOWN,
      VNODE_TYPE_FIFO
    };

  return transfer_table[filetype];
}

int nfs_mount(const char *server_ip_and_path, const char *mount_path,
              unsigned int uid, unsigned int gid)
{
  struct nfs_args         nfs_args = {0};
  char                    *server_ip_addr = NULL;
  char                    *server_nfs_path = NULL;
  int                     found_colon = 0;
  unsigned int            pos;
  int                     ret = -1;
  struct sockaddr_in      *nfs_srv_addr = NULL;
  size_t len;

  rpcclnt_setuidgid(uid, gid);

  len = strlen(server_ip_and_path);
  for (pos = 0; pos < len; pos++)
    {
      if (*(server_ip_and_path + pos) == ':')
        {
          found_colon = 1;
          break;
        }
    }

  if (!found_colon)
    {
      set_errno(ENOENT);
      goto nfs_mount_out;
    }

  server_ip_addr = (char *)malloc(pos + 1);
  if (server_ip_addr == NULL)
    {
      PRINTK("malloc failure\n");
      set_errno(ENOMEM);
      goto nfs_mount_out;
    }

  ret = strncpy_s(server_ip_addr, pos + 1, server_ip_and_path, pos);
  if (ret != EOK)
    {
      set_errno(ENOBUFS);
      goto nfs_free_node_out;
    }
  *(server_ip_addr + pos) = '\0';
  server_nfs_path = (char *)(server_ip_and_path + pos + 1);

  (void)memset_s(&nfs_args, sizeof(nfs_args), 0, sizeof(nfs_args));

  if (g_NFSMOUNT_HOOK != NULL)
    {
      g_NFSMOUNT_HOOK(&nfs_args);
    }

  nfs_args.path = server_nfs_path; /* server nfs dir */

  nfs_srv_addr = (struct sockaddr_in *)&nfs_args.addr;
  nfs_srv_addr->sin_family = AF_INET;
  ret = inet_pton(AF_INET, server_ip_addr, &nfs_srv_addr->sin_addr.s_addr);
  if (ret != 1)
    {
      ret = -1;
      set_errno(ECONNREFUSED);
      goto nfs_free_node_out;
    }
  nfs_srv_addr->sin_port = htons(PMAPPORT);

  nfs_args.addrlen = sizeof(nfs_args.addr);
#if (NFS_PROTO_TYPE == NFS_IPPROTO_TCP)
  nfs_args.sotype = SOCK_STREAM;
#elif (NFS_PROTO_TYPE == NFS_IPPROTO_UDP)
  nfs_args.sotype = SOCK_DGRAM;
#endif

  PRINTK("Mount nfs on %s:%s, uid:%d, gid:%d\n", server_ip_addr, server_nfs_path, uid, gid);
  ret = mount((const char *)NULL, mount_path, "nfs", 0, &nfs_args);

nfs_free_node_out:
  free(server_ip_addr);

nfs_mount_out:
  if (ret)
    {
      perror("mount nfs error");
      return -1;
    }
  PRINTK("Mount nfs finished.\n");
  return 0;
}

int vfs_nfs_mount(struct Mount *mnt, struct Vnode *device, const void *data)
{
  struct nfsmount *nmp = NULL;
  struct Vnode *vp = NULL;

  int ret = VnodeAlloc(&nfs_vops, &vp);
  if (ret != OK)
    {
      return -EADDRNOTAVAIL;
    }

  struct nfsnode *root = zalloc(sizeof(struct nfsnode));
  if (root == NULL)
    {
      (void)VnodeFree(vp);
      return -EADDRNOTAVAIL;
    }

  ret = nfs_bind(NULL, data, (void **)(&nmp), NULL);
  if (ret != OK || nmp == NULL)
    {
      (void)VnodeFree(vp);
      free(root);
      return -EAGAIN;
    }
  vp->originMount = mnt;
  vp->fop = &nfs_fops;
  vp->data = root;
  root->n_fhsize = nmp->nm_fhsize;
  (void)memcpy_s(&(root->n_fhandle), root->n_fhsize, &(nmp->nm_fh), nmp->nm_fhsize);
  mnt->vnodeCovered = vp;
  mnt->data = nmp;

  ret = vfs_nfs_stat_internal(nmp, root);
  if (ret == OK)
    {
      vp->type = root->n_type;
    }
  nmp->nm_permission = mnt->vnodeBeCovered->mode & 0777;
  nmp->nm_gid = mnt->vnodeBeCovered->gid;
  nmp->nm_uid = mnt->vnodeBeCovered->uid;
  vp->mode = type_to_mode(vp->type, nmp->nm_permission);
  vp->gid = nmp->nm_gid;
  vp->uid = nmp->nm_uid;
  return OK;
}

int vfs_nfs_lookup(struct Vnode *parent, const char *path, int len, struct Vnode **vpp)
{
  int ret;
  struct nfs_fattr obj_attributes;
  struct nfsmount *nmp;
  char filename[len + 1];
  struct file_handle fhandle;
  struct nfsnode *parent_nfs_node = NULL;
  struct nfsnode *nfs_node = NULL;
  nmp = (struct nfsmount *)(parent->originMount->data);
  nfs_mux_take(nmp);
  parent_nfs_node = (struct nfsnode *)parent->data;
  fhandle.length = parent_nfs_node->n_fhsize;
  (void)memcpy_s(&(fhandle.handle), fhandle.length, &(parent_nfs_node->n_fhandle), parent_nfs_node->n_fhsize);
  filename[len] = '\0';
  (void)memcpy_s(filename, (len + 1), path, len);

  ret = nfs_lookup(nmp, filename, &fhandle, &obj_attributes, NULL);
  if (ret != OK)
    {
      nfs_mux_release(nmp);
      return -ENOENT;
    }

  /* Initialize the file private data.
   *
   * Copy the file handle.
   */
  nfs_node = zalloc(sizeof(struct nfsnode));
  nfs_node->n_fhsize = (uint8_t)fhandle.length;
  memcpy_s(&(nfs_node->n_fhandle), nfs_node->n_fhsize, &(fhandle.handle), fhandle.length);
  nfs_node->n_name = zalloc(sizeof(filename));
  memcpy_s(nfs_node->n_name, (len + 1), filename, sizeof(filename));

  /* Save the file attributes */
  nfs_attrupdate(nfs_node, &obj_attributes);

  (void)VnodeAlloc(&nfs_vops, vpp);
  (*vpp)->parent = parent;
  (*vpp)->fop = &nfs_fops;
  (*vpp)->originMount = parent->originMount;
  (*vpp)->data = nfs_node;
  (*vpp)->type = filetype_to_vnodetype(nfs_node->n_type);
  (*vpp)->mode = type_to_mode((*vpp)->type, nmp->nm_permission);
  (*vpp)->gid = nmp->nm_gid;
  (*vpp)->uid = nmp->nm_uid;
  nfs_mux_release(nmp);
  return OK;
}

int vfs_nfs_stat(struct Vnode *node, struct stat *buf)
{
  struct nfsnode *nfs_node = NULL;
  struct nfsmount *nmp = (struct nfsmount *)(node->originMount->data);
  nfs_mux_take(nmp);
  nfs_node = (struct nfsnode *)node->data;
  buf->st_mode = node->mode;
  buf->st_gid = node->gid;
  buf->st_uid = node->uid;
  buf->st_size    = (off_t)nfs_node->n_size;
  buf->st_blksize = 0;
  buf->st_blocks  = 0;
  buf->st_mtime   = nfs_node->n_mtime;
  buf->st_atime   = nfs_node->n_atime;
  buf->st_ctime   = nfs_node->n_ctime;
  nfs_mux_release(nmp);
  return OK;
}

int vfs_nfs_opendir(struct Vnode *node, struct fs_dirent_s *dir)
{
  int ret;
  struct nfsdir_s *nfs_dir = NULL;
  struct nfsnode *nfs_node = NULL;
  struct nfsmount *nmp = (struct nfsmount *)(node->originMount->data);
  if (node->type != VNODE_TYPE_DIR) {
      return -ENOTDIR;
  }
  nfs_mux_take(nmp);
  nfs_node = (struct nfsnode *)node->data;
  ret = nfs_checkmount(nmp);
  if (ret != OK) {
      ret = -ret;
      PRINTK("ERROR: nfs_checkmount failed: %d\n", ret);
      goto errout_with_mutex;
  }
  nfs_dir = (struct nfsdir_s *)malloc(sizeof(struct nfsdir_s));
  if (!nfs_dir) {
      ret = -ENOMEM;
      goto errout_with_mutex;
  }
  nfs_dir->nfs_fhsize = nfs_node->n_fhsize;
  nfs_dir->nfs_cookie[0] = 0;
  nfs_dir->nfs_cookie[1] = 0;
  (void)memcpy_s(nfs_dir->nfs_fhandle, DIRENT_NFS_MAXHANDLE, &(nfs_node->n_fhandle), DIRENT_NFS_MAXHANDLE);
  dir->u.fs_dir = (fs_dir_s)nfs_dir;
  ret = OK;

  nfs_dir->nfs_next = nmp->nm_dir;
  nmp->nm_dir = nfs_dir;
  nfs_dir->nfs_dir = dir;
  nfs_dir->nfs_entries = NULL;

errout_with_mutex:
  nfs_mux_release(nmp);
  return ret;
}

int vfs_nfs_readdir(struct Vnode *node, struct fs_dirent_s *dir)
{
  struct nfsmount *nmp;
  struct file_handle fhandle;
  struct nfs_fattr obj_attributes;
  struct nfsdir_s *nfs_dir = NULL;
  struct entry3   *entry = NULL;
  struct entry3   *entry_pos = NULL;

  /* Use 2 cookies */

  uint32_t cookies[2];
  uint32_t tmp;
  uint32_t *ptr = NULL;
  size_t d_name_size;
  int reqlen;
  int error = 0;
  int i = 0;

  /* Sanity checks */

  /* Recover our private data from the vnode instance */

  nmp = (struct nfsmount *)(node->originMount->data);

  /* Make sure that the mount is still healthy */

  nfs_mux_take(nmp);
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }

  /* Request a block directory entries, copying directory information from
   * the dirent structure.
   */
  nfs_dir = (struct nfsdir_s *)dir->u.fs_dir;
  cookies[0] = 0;
  cookies[1] = 0;

  if (nfs_dir && nfs_dir->nfs_entries && (nfs_dir->nfs_entries->file_id[0] == (uint32_t)EOF))
    {
      error = ENOENT;
      free(nfs_dir->nfs_entries);
      nfs_dir->nfs_entries = NULL;
      goto errout_with_mutex;
    }
  while (i < dir->read_cnt)
    {
      if (!nfs_dir->nfs_entries)
        {
          entry_pos = nfs_dir->nfs_entries;
          do
            {
              ptr     = (uint32_t *)&nmp->nm_msgbuffer.readdir.readdir;
              reqlen  = 0;

              /* Copy the variable length, directory file handle */

              *ptr++  = txdr_unsigned((uint32_t)nfs_dir->nfs_fhsize);
              reqlen += sizeof(uint32_t);

              (void)memcpy_s(ptr, nfs_dir->nfs_fhsize, nfs_dir->nfs_fhandle, nfs_dir->nfs_fhsize);
              reqlen += (int)nfs_dir->nfs_fhsize;
              ptr    += uint32_increment((int)nfs_dir->nfs_fhsize);

              /* Cookie and cookie verifier */

              ptr[0] = cookies[0];
              ptr[1] = cookies[1];
              ptr    += 2;
              reqlen += 2 * sizeof(uint32_t);

              (void)memcpy_s(ptr, DIRENT_NFS_VERFLEN, nfs_dir->nfs_verifier, DIRENT_NFS_VERFLEN);
              ptr    += uint32_increment(DIRENT_NFS_VERFLEN);
              reqlen += DIRENT_NFS_VERFLEN;

              /* Number of directory entries (We currently only process one entry at a time) */

              *ptr    = txdr_unsigned((uint32_t)nmp->nm_readdirsize);
              reqlen += sizeof(uint32_t);

              /* And read the directory */

              nfs_statistics(NFSPROC_READDIR);
              error = nfs_request(nmp, NFSPROC_READDIR,
                                  (void *)&nmp->nm_msgbuffer.readdir, reqlen,
                                  (void *)nmp->nm_iobuffer, nmp->nm_buflen);

              if (error != OK)
                {
                  PRINTK("ERROR: nfs_request failed: %d\n", error);
                  goto errout_with_mutex;
                }

              /* A new group of entries was successfully read.  Process the
               * information contained in the response header.  This information
               * includes:
               *
               * 1) Attributes follow indication - 4 bytes
               * 2) Directory attributes         - sizeof(struct nfs_fattr)
               * 3) Cookie verifier              - NFSX_V3COOKIEVERF bytes
               * 4) Values follows indication    - 4 bytes
               */

              ptr = (uint32_t *)&((struct rpc_reply_readdir *)nmp->nm_iobuffer)->readdir;

              /* Check if attributes follow, if 0 so Skip over the attributes */

              tmp = *ptr++;
              if (tmp != 0)
                {
                  /* Attributes are not currently used */

                  ptr += uint32_increment(sizeof(struct nfs_fattr));
                }

              /* Save the verification cookie */

              (void)memcpy_s(nfs_dir->nfs_verifier, DIRENT_NFS_VERFLEN, ptr, DIRENT_NFS_VERFLEN);
              ptr += uint32_increment(DIRENT_NFS_VERFLEN);

              /* Check if values follow.  If no values follow, then the EOF indication
               * will appear next.
               */

              tmp = *ptr++;
              if (tmp == 0)
                {
                  /* No values follow, then the reply should consist only of a 4-byte
                   * end-of-directory indication.
                   */

                  tmp = *ptr++;
                  if (tmp != 0)
                    {
                      error = ENOENT;
                    }

                  /* What would it mean if there were not data and we not at the end of
                   * file?
                   */

                  else
                    {
                      error = EAGAIN;
                    }
                  goto errout_with_mutex;
                }

              /* If we are not at the end of the directory listing, then a set of entries
               * will follow the header.  Each entry is of the form:
               *
               *    File ID (8 bytes)
               *    Name length (4 bytes)
               *    Name string (varaiable size but in multiples of 4 bytes)
               *    Cookie (8 bytes)
               *    next entry (4 bytes)
               */

               do
                {
                  NFS_DIR_ENTRY_MALLOC(entry);

                  /* There is an entry. Skip over the file ID and point to the length */

                  entry->file_id[0] = *ptr++;
                  entry->file_id[1] = *ptr++; /*lint !e662 !e661*/

                  /* Get the length and point to the name */

                  tmp    = *ptr++; /*lint !e662 !e661*/
                  entry->name_len = fxdr_unsigned(uint32_t, tmp);
                  entry->contents = (uint8_t *)malloc(entry->name_len + 1);
                  if (!entry->contents)
                    {
                      free(entry);
                      entry = NULL;
                      goto errout_with_memory;
                    }
                  (void)memset_s(entry->contents, entry->name_len + 1, 0, entry->name_len + 1);

                  error = strncpy_s((char *)entry->contents, entry->name_len + 1, (const char *)ptr, entry->name_len);
                  if (error != EOK)
                    {
                      free(entry->contents);
                      entry->contents = NULL;
                      free(entry);
                      entry = NULL;
                      error = ENOBUFS;
                      goto errout_with_memory;
                    }
                  /* Increment the pointer past the name (allowing for padding). ptr
                   * now points to the cookie.
                   */

                  ptr += uint32_increment(entry->name_len);

                  /* Save the cookie and increment the pointer to the next entry */

                  entry->cookie[0] = *ptr++;
                  entry->cookie[1] = *ptr++;

                  /* Get the file attributes associated with this name and return
                   * the file type.
                   */

                  if (strcmp((char *)entry->contents, ".") == 0 || strcmp((char *)entry->contents, "..") == 0)
                    {
                      NFS_DIR_ENTRY_FREE(entry);
                      continue;
                    }

                  if (!nfs_dir->nfs_entries)
                    {
                      entry_pos = entry;
                      nfs_dir->nfs_entries = entry;
                    }
                  else
                    {
                      entry_pos->next = entry;
                      entry_pos = entry;
                    }
                }
              while (*ptr++);
              if (entry_pos)
                {
                  cookies[0] = entry_pos->cookie[0];
                  cookies[1] = entry_pos->cookie[1];
                }
            }
          while (!(*ptr));

          if (!nfs_dir->nfs_entries)
            {
              error = ENOENT;
              goto errout_with_mutex;
            }

          NFS_DIR_ENTRY_MALLOC(entry);

          /* There is an entry. Skip over the file ID and point to the length */

          entry->file_id[0] = (uint32_t)EOF;
          if (!entry_pos)
            {
              error = ENOENT;
              NFS_DIR_ENTRY_FREE(entry);
              goto errout_with_mutex;
            }
          entry_pos->next = entry;
          entry_pos = entry;
        }

      entry_pos = nfs_dir->nfs_entries;
      if (nfs_dir->nfs_entries->file_id[0] == (uint32_t)EOF)
        {
          error = ENOENT;
          goto errout_with_mutex;
        }

      d_name_size = sizeof(dir->fd_dir[i].d_name);
      error = memcpy_s(dir->fd_dir[i].d_name, d_name_size, (const char *)entry_pos->contents, (size_t)entry_pos->name_len);
      if (error != EOK)
        {
          error = ENOBUFS;
          goto errout_with_memory;
        }
      if (entry_pos->name_len >= d_name_size)
        {
          dir->fd_dir[i].d_name[d_name_size - 1] = '\0';
        }
      else
        {
          dir->fd_dir[i].d_name[entry_pos->name_len] = '\0';
        }

      nfs_dir->nfs_entries = entry_pos->next;
      NFS_DIR_ENTRY_FREE(entry_pos);

      fhandle.length = (uint32_t)nfs_dir->nfs_fhsize;
      (void)memcpy_s(&fhandle.handle, DIRENT_NFS_MAXHANDLE, nfs_dir->nfs_fhandle, DIRENT_NFS_MAXHANDLE);

      error = nfs_lookup(nmp, dir->fd_dir[i].d_name, &fhandle, &obj_attributes, NULL);
      if (error != OK)
      {
        PRINTK("ERROR: nfs_lookup failed: %d\n", error);
        goto errout_with_memory;
      }

      /* Set the dirent file type */

      tmp = fxdr_unsigned(uint32_t, obj_attributes.fa_type);
      switch (tmp)
        {
        default:
        case NFNON:        /* Unknown type */
        case NFSOCK:       /* Socket */
        case NFLNK:        /* Symbolic link */
          break;

        case NFREG:        /* Regular file */
          dir->fd_dir[i].d_type = DT_REG;
          break;

        case NFDIR:        /* Directory */
          dir->fd_dir[i].d_type = DT_DIR;
          break;

        case NFBLK:        /* Block special device file */
          dir->fd_dir[i].d_type = DT_BLK;
          break;

        case NFFIFO:       /* Named FIFO */
        case NFCHR:        /* Character special device file */
          dir->fd_dir[i].d_type = DT_CHR;
          break;
        }
      dir->fd_position++;
      dir->fd_dir[i].d_off = dir->fd_position;
      dir->fd_dir[i].d_reclen = (uint16_t)sizeof(struct dirent);
      i++;
    }
  nfs_mux_release(nmp);
  return i;

errout_with_memory:
  for (entry_pos = nfs_dir->nfs_entries; entry_pos != NULL; entry_pos = nfs_dir->nfs_entries)
    {
      nfs_dir->nfs_entries = entry_pos->next;
      NFS_DIR_ENTRY_FREE(entry_pos);
    }
errout_with_mutex:
  nfs_mux_release(nmp);
  if (error == ENOENT && i > 0)
    {
      return i;
    }
  return -error;
}
extern int nfs_getfilename(char *dstpath, unsigned int dstpathLen, const char *srcpath, unsigned int maxlen);
extern int nfs_rename(struct Vnode *mountpt, const char *oldrelpath, const char *newrelpath);
int vfs_nfs_rename(struct Vnode *from_vnode, struct Vnode *to_parent,
                   const char *from_name, const char *to_name)
{
  int error;
  int reqlen;
  int namelen;
  uint32_t *ptr = NULL;
  struct Vnode *to_vnode = NULL;
  struct Vnode *from_parent = from_vnode->parent;
  struct nfsnode *from_node = NULL;
  struct nfsnode *to_node = NULL;
  struct nfsmount *nmp = (struct nfsmount *)(to_parent->originMount->data);

  nfs_mux_take(nmp);
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }

  from_node = (struct nfsnode *)from_parent->data;
  to_node = (struct nfsnode *)to_parent->data;

  ptr    = (uint32_t *)&nmp->nm_msgbuffer.renamef.rename;
  reqlen = 0;

  /* Copy the variable length, 'from' directory file handle */

  *ptr++  = txdr_unsigned(from_node->n_fhsize);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, from_node->n_fhsize, &from_node->n_fhandle, from_node->n_fhsize);
  reqlen += (int)from_node->n_fhsize;
  ptr    += uint32_increment(from_node->n_fhsize);

  /* Copy the variable-length 'from' object name */

  namelen = strlen(from_name);

  *ptr++  = txdr_unsigned(namelen);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, namelen, from_name, namelen);
  reqlen += uint32_alignup(namelen);
  ptr    += uint32_increment(namelen);

  /* Copy the variable length, 'to' directory file handle */

  *ptr++  = txdr_unsigned(to_node->n_fhsize);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, to_node->n_fhsize, &to_node->n_fhandle, to_node->n_fhsize);
  ptr    += uint32_increment(to_node->n_fhsize);
  reqlen += (int)to_node->n_fhsize;

  /* Copy the variable-length 'to' object name */

  namelen = strlen(to_name);

  *ptr++  = txdr_unsigned(namelen);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, namelen, to_name, namelen);
  reqlen += uint32_alignup(namelen);

  /* Perform the RENAME RPC */

  nfs_statistics(NFSPROC_RENAME);
  error = nfs_request(nmp, NFSPROC_RENAME,
      (void *)&nmp->nm_msgbuffer.renamef, reqlen,
      (void *)nmp->nm_iobuffer, nmp->nm_buflen);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_request returned: %d\n", error);
      goto errout_with_mutex;
    }

  error = vfs_nfs_lookup(to_parent, to_name, strlen(to_name), &to_vnode);
  if (error != OK)
    {
      error = -error;
      PRINTK("ERROR: nfs_rename not finish\n");
      goto errout_with_mutex;
    }
  vfs_nfs_reclaim(from_vnode);
  from_vnode->data = to_vnode->data;
  from_vnode->parent = to_parent;
  to_vnode->data = NULL;
  VnodeFree(to_vnode);

errout_with_mutex:
  nfs_mux_release(nmp);
  return -error;
}

int vfs_nfs_mkdir(struct Vnode *parent, const char *dirname, mode_t mode, struct Vnode **vpp)
{
  struct nfsmount *nmp = (struct nfsmount *)(parent->originMount->data);
  struct nfsnode *parent_nfs_node = NULL;
  struct nfs_fattr obj_attributes;
  struct nfsnode *target_node = NULL;
  struct file_handle fhandle;
  uint32_t          *ptr = NULL;
  uint32_t               tmp;
  int                    namelen;
  int                    reqlen;
  int                    error;

  /* Sanity checks */

  DEBUGASSERT(mountpt && mountpt->i_private);

  /* Check if the mount is still healthy */

  nfs_mux_take(nmp);
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount: %d\n", error);
      goto errout_with_mutex;
    }

  parent_nfs_node = (struct nfsnode *)parent->data;

  /* Format the MKDIR call message arguments */

  ptr    = (uint32_t *)&nmp->nm_msgbuffer.mkdir.mkdir;
  reqlen = 0;

  /* Copy the variable length, directory file handle */

  *ptr++  = txdr_unsigned(parent_nfs_node->n_fhsize);
  reqlen += sizeof(uint32_t);

  memcpy_s(ptr, parent_nfs_node->n_fhsize, &(parent_nfs_node->n_fhandle), parent_nfs_node->n_fhsize);
  ptr    += uint32_increment(parent_nfs_node->n_fhsize);
  reqlen += (int)parent_nfs_node->n_fhsize;

  /* Copy the variable-length directory name */

  namelen = strlen(dirname);

  *ptr++  = txdr_unsigned(namelen);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, namelen, dirname, namelen);
  ptr    += uint32_increment(namelen);
  reqlen += uint32_alignup(namelen);

  /* Set the mode.  NOTE: Here we depend on the fact that the NuttX and NFS
   * bit settings are the same (at least for the bits of interest).
   */

  *ptr++  = nfs_true; /* True: mode value follows */
  reqlen += sizeof(uint32_t);

  if (!mode)
    {
      mode = (NFSMODE_IXOTH | NFSMODE_IROTH |
              NFSMODE_IXGRP | NFSMODE_IRGRP |
              NFSMODE_IXUSR | NFSMODE_IWUSR | NFSMODE_IRUSR);
    }
  tmp = mode & (NFSMODE_IXOTH | NFSMODE_IWOTH | NFSMODE_IROTH |
                NFSMODE_IXGRP | NFSMODE_IWGRP | NFSMODE_IRGRP |
                NFSMODE_IXUSR | NFSMODE_IWUSR | NFSMODE_IRUSR);
  *ptr++  = txdr_unsigned(tmp);
  reqlen += sizeof(uint32_t);

  /* Set the user ID to zero */

  *ptr++  = nfs_true;             /* True: Uid value follows */
  *ptr++  = 0;                    /* UID = 0 (nobody) */
  reqlen += 2*sizeof(uint32_t);

  /* Set the group ID to one */

  *ptr++  = nfs_true;            /* True: Gid value follows */
  *ptr++  = htonl(1);            /* GID = 1 (nogroup) */
  reqlen += 2*sizeof(uint32_t);

  /* No size */

  *ptr++  = nfs_false; /* False: No size value follows */
  reqlen += sizeof(uint32_t);

  /* Don't change times */

  *ptr++  = htonl(NFSV3SATTRTIME_DONTCHANGE); /* Don't change atime */
  *ptr++  = htonl(NFSV3SATTRTIME_DONTCHANGE); /* Don't change mtime */
  reqlen += 2*sizeof(uint32_t);

  /* Perform the MKDIR RPC */

  nfs_statistics(NFSPROC_MKDIR);
  error = nfs_request(nmp, NFSPROC_MKDIR,
      (void *)&nmp->nm_msgbuffer.mkdir, reqlen,
      (void *)nmp->nm_iobuffer, nmp->nm_buflen);
  if (error)
    {
      PRINTK("ERROR: nfs_request failed: %d\n", error);
      goto errout_with_mutex;
    }

  fhandle.length = parent_nfs_node->n_fhsize;
  memcpy_s(&(fhandle.handle), DIRENT_NFS_MAXHANDLE, &(parent_nfs_node->n_fhandle), fhandle.length);
  error = nfs_lookup(nmp, dirname, &fhandle, &obj_attributes, NULL);
  if (error)
    {
      error = ENOENT;
      goto errout_with_mutex;
    }

  /* Initialize the file private data.
   *
   * Copy the file handle.
   */
  target_node = zalloc(sizeof(struct nfsnode));
  target_node->n_fhsize = (uint8_t)fhandle.length;
  memcpy_s(&(target_node->n_fhandle), target_node->n_fhsize, &(fhandle.handle), fhandle.length);
  target_node->n_name = zalloc(sizeof (dirname));
  memcpy_s(target_node->n_name, sizeof(dirname), dirname, sizeof (dirname));

  /* Save the file attributes */
  nfs_attrupdate(target_node, &obj_attributes);
  (void)VnodeAlloc(&nfs_vops, vpp);
  (*vpp)->parent = parent;
  (*vpp)->fop = &nfs_fops;
  (*vpp)->originMount = parent->originMount;
  (*vpp)->data = target_node;
  (*vpp)->type = filetype_to_vnodetype(target_node->n_type);
  (*vpp)->mode = type_to_mode((*vpp)->type, nmp->nm_permission);
  (*vpp)->gid = nmp->nm_gid;
  (*vpp)->uid = nmp->nm_uid;

errout_with_mutex:
  nfs_mux_release(nmp);
  return -error;
}

int vfs_nfs_write(struct file *filep, const char *buffer, size_t buflen)
{
  struct nfsmount       *nmp;
  struct nfsnode        *np;
  loff_t                f_pos;
  size_t                writesize;
  size_t                bufsize;
  size_t                byteswritten;
  size_t                reqlen;
  uint32_t              *ptr = NULL;
  uint32_t              tmp;
  int                   committed = NFSV3WRITE_UNSTABLE;
  int                   error;
  char                  *temp_buffer = NULL;
  struct file_handle    parent_fhandle;

  struct Vnode *node = filep->f_vnode;
  nmp = (struct nfsmount *)(node->originMount->data);
  DEBUGASSERT(nmp != NULL);

  /* Make sure that the mount is still healthy */

  nfs_mux_take(nmp);
  np  = (struct nfsnode *)node->data;
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }

  parent_fhandle.length = ((struct nfsnode *)node->parent->data)->n_fhsize;
  memcpy_s(&(parent_fhandle.handle), parent_fhandle.length,
      &(((struct nfsnode *)node->parent->data)->n_fhandle),
      ((struct nfsnode *)node->parent->data)->n_fhsize);

  if (filep->f_oflags & O_APPEND)
    {
      if (nfs_fileupdate(nmp, np->n_name, &parent_fhandle, np) == OK)
        {
          f_pos = np->n_size;
        }
      else
        {
          error = EAGAIN;
          goto errout_with_mutex;
        }
    }
  else
    {
      f_pos = filep->f_pos;
    }

  /* Check if the file size would exceed the range of off_t */

  if (np->n_size + buflen < np->n_size)
    {
      error = EFBIG;
      goto errout_with_mutex;
    }

  /* Allocate memory for data */

  bufsize = (buflen < nmp->nm_wsize) ? buflen : nmp->nm_wsize;
  temp_buffer = malloc(bufsize);
  if (temp_buffer == NULL)
    {
      error = ENOMEM;
      goto errout_with_mutex;
    }

  /* Now loop until we send the entire user buffer */

  writesize = 0;
  for (byteswritten = 0; byteswritten < buflen; )
    {
      /* Make sure that the attempted write size does not exceed the RPC
       * maximum.
       */

      writesize = buflen - byteswritten;
      if (writesize > nmp->nm_wsize)
        {
          writesize = nmp->nm_wsize;
        }

      /* Make sure that the attempted read size does not exceed the IO
       * buffer size.
       */

      bufsize = SIZEOF_rpc_call_write(writesize);
      if (bufsize > nmp->nm_buflen)
        {
          writesize -= (bufsize - nmp->nm_buflen);
        }

      /* Copy a chunk of the user data into the temporary buffer */

      if (LOS_CopyToKernel(temp_buffer, writesize, buffer, writesize) != 0)
        {
          error = EINVAL;
          goto errout_with_memfree;
        }

      /* Initialize the request.  Here we need an offset pointer to the write
       * arguments, skipping over the RPC header.  Write is unique among the
       * RPC calls in that the entry RPC calls messasge lies in the I/O buffer
       */

      ptr     = (uint32_t *)&((struct rpc_call_write *)
          nmp->nm_iobuffer)->write;
      reqlen  = 0;

      /* Copy the variable length, file handle */

      *ptr++  = txdr_unsigned((uint32_t)np->n_fhsize);
      reqlen += sizeof(uint32_t);

      (void)memcpy_s(ptr, np->n_fhsize, &np->n_fhandle, np->n_fhsize);
      reqlen += (int)np->n_fhsize;
      ptr    += uint32_increment((int)np->n_fhsize);

      /* Copy the file offset */

      txdr_hyper((uint64_t)f_pos, ptr);
      ptr    += 2;
      reqlen += 2*sizeof(uint32_t);

      /* Copy the count and stable values */

      *ptr++  = txdr_unsigned(writesize);
      *ptr++  = txdr_unsigned((uint32_t)committed);
      reqlen += 2*sizeof(uint32_t);

      /* Copy a chunk of the user data into the I/O buffer from temporary buffer */

      *ptr++  = txdr_unsigned(writesize);
      reqlen += sizeof(uint32_t);
      error = memcpy_s(ptr, writesize, temp_buffer, writesize);
      if (error != EOK)
        {
          error = ENOBUFS;
          goto errout_with_memfree;
        }
      reqlen += uint32_alignup(writesize);

      /* Perform the write */

      nfs_statistics(NFSPROC_WRITE);
      error = nfs_request(nmp, NFSPROC_WRITE,
          (void *)nmp->nm_iobuffer, reqlen,
          (void *)&nmp->nm_msgbuffer.write,
          sizeof(struct rpc_reply_write));
      if (error)
        {
          goto errout_with_memfree;
        }

      /* Get a pointer to the WRITE reply data */

      ptr = (uint32_t *)&nmp->nm_msgbuffer.write.write;

      /* Parse file_wcc.  First, check if WCC attributes follow. */

      tmp = *ptr++;
      if (tmp != 0)
        {
          /* Yes.. WCC attributes follow.  But we just skip over them. */

          ptr += uint32_increment(sizeof(struct wcc_attr));
        }

      /* Check if normal file attributes follow */

      tmp = *ptr++;
      if (tmp != 0)
        {
          /* Yes.. Update the cached file status in the file structure. */

          nfs_attrupdate(np, (struct nfs_fattr *)ptr);
          ptr += uint32_increment(sizeof(struct nfs_fattr));
        }

      /* Get the count of bytes actually written */

      tmp = fxdr_unsigned(uint32_t, *ptr);
      ptr++;

      if (tmp < 1 || tmp > writesize)
        {
          error = EIO;
          goto errout_with_memfree;
        }

      writesize = tmp;
      f_pos += writesize;
      filep->f_pos = f_pos;
      np->n_fpos = f_pos;

      /* Update the read state data */

      if (filep->f_pos > (loff_t)np->n_size)
        {
          np->n_size = f_pos;
        }
      byteswritten += writesize;
      buffer       += writesize;
  }

  free(temp_buffer);
  nfs_mux_release(nmp);
  return byteswritten;
errout_with_memfree:
  free(temp_buffer);
errout_with_mutex:
  nfs_mux_release(nmp);
  return -error;
}

off_t vfs_nfs_seek(struct file *filep, off_t offset, int whence)
{
  struct Vnode *node = filep->f_vnode;
  struct nfsnode  *np = NULL;
  struct nfsmount *nmp = (struct nfsmount *)(node->originMount->data);
  int                        error;
  off_t                      position;

  /* Make sure that the mount is still healthy */

  nfs_mux_take(nmp);
  np = (struct nfsnode *)node->data;
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }


  switch (whence)
    {
    case SEEK_SET: /* The offset is set to offset bytes. */
      position = offset;
      break;

    case SEEK_CUR: /* The offset is set to its current location plus offset bytes. */
      position = offset + filep->f_pos;
      break;

    case SEEK_END: /* The offset is set to the size of the file plus offset bytes. */
      position = offset + np->n_size;
      break;

    default:
      error = EINVAL;
      goto errout_with_mutex;
    }

  /* Attempts to set the position beyound the end of file will
   * work if the file is open for write access.
   */

  if ((position > (off_t)np->n_size) && ((np->n_oflags & O_WRONLY) == 0) &&
      ((np->n_oflags & O_RDWR) == 0))
  {
      position = np->n_size;
  }

  /* position less than 0 should be reset to 0 */

  if (position < 0)
  {
      position = 0;
  }

  np->n_fpos = (loff_t)position;
  filep->f_pos = np->n_fpos;
  if (position > (off_t)np->n_size)
  {
      np->n_size = (loff_t)position;
  }
  nfs_mux_release(nmp);
  return (off_t)filep->f_pos;

errout_with_mutex:
  nfs_mux_release(nmp);
  return -error;
}

ssize_t vfs_nfs_read(struct file *filep, char *buffer, size_t buflen)
{
  struct nfsnode            *np;
  struct rpc_reply_read     *read_response = NULL;
  size_t                     readsize;
  size_t                     tmp;
  size_t                     bytesread;
  size_t                     reqlen;
  uint32_t                  *ptr = NULL;
  int                        error = 0;
  struct file_handle         parent_fhandle;

  struct Vnode *node = filep->f_vnode;
  struct nfsmount *nmp = (struct nfsmount *)(node->originMount->data);

  DEBUGASSERT(nmp != NULL);

  /* Make sure that the mount is still healthy */

  nfs_mux_take(nmp);
  np  = (struct nfsnode *)node->data;
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }


  parent_fhandle.length = ((struct nfsnode *)node->parent->data)->n_fhsize;
  memcpy_s(&(parent_fhandle.handle), parent_fhandle.length,
      &(((struct nfsnode *)node->parent->data)->n_fhandle),
      ((struct nfsnode *)node->parent->data)->n_fhsize);
  error = nfs_fileupdate(nmp, np->n_name, &parent_fhandle, np);
  if (error != OK)
    {
      PRINTK("nfs_fileupdate failed: %d\n", error);
      goto errout_with_mutex;
    }

  /* Get the number of bytes left in the file and truncate read count so that
   * it does not exceed the number of bytes left in the file.
   */

  tmp = np->n_size - filep->f_pos;
  if (buflen > tmp)
    {
      buflen = tmp;
    }

  /* Now loop until we fill the user buffer (or hit the end of the file) */

  for (bytesread = 0; bytesread < buflen; )
    {
      /* Make sure that the attempted read size does not exceed the RPC maximum */

      readsize = buflen - bytesread;
      if (readsize > nmp->nm_rsize)
        {
          readsize = nmp->nm_rsize;
        }

      /* Make sure that the attempted read size does not exceed the IO buffer size */

      tmp = SIZEOF_rpc_reply_read(readsize);
      if (tmp > nmp->nm_buflen)
        {
          readsize -= (tmp - nmp->nm_buflen);
        }

      /* Initialize the request */

      ptr     = (uint32_t *)&nmp->nm_msgbuffer.read.read;
      reqlen  = 0;

      /* Copy the variable length, file handle */

      *ptr++  = txdr_unsigned((uint32_t)np->n_fhsize);
      reqlen += sizeof(uint32_t);

      memcpy_s(ptr, np->n_fhsize, &np->n_fhandle, np->n_fhsize);
      reqlen += (int)np->n_fhsize;
      ptr    += uint32_increment((int)np->n_fhsize);

      /* Copy the file offset */

      txdr_hyper((uint64_t)filep->f_pos, ptr);
      ptr += 2;
      reqlen += 2*sizeof(uint32_t);

      /* Set the readsize */

      *ptr = txdr_unsigned(readsize);
      reqlen += sizeof(uint32_t);

      /* Perform the read */

      nfs_statistics(NFSPROC_READ);
      error = nfs_request(nmp, NFSPROC_READ,
          (void *)&nmp->nm_msgbuffer.read, reqlen,
          (void *)nmp->nm_iobuffer, nmp->nm_buflen);
      if (error)
        {
          PRINTK("ERROR: nfs_request failed: %d\n", error);
          goto errout_with_mutex;
        }

      /* The read was successful.  Get a pointer to the beginning of the NFS
       * response data.
       */

      read_response = (struct rpc_reply_read *)nmp->nm_iobuffer;
      readsize = fxdr_unsigned(uint32_t, read_response->read.hdr.count);

      /* Copy the read data into the user buffer */

      if (LOS_CopyFromKernel(buffer, buflen, (const void *)read_response->read.data, readsize) != 0)
        {
          error = EINVAL;
          goto errout_with_mutex;
        }

      /* Update the read state data */

      filep->f_pos += readsize;
      np->n_fpos   += readsize;
      bytesread    += readsize;
      buffer       += readsize;

      /* Check if we hit the end of file */

      if (read_response->read.hdr.eof != 0)
        {
          break;
        }
    }

  nfs_mux_release(nmp);
  return bytesread;

errout_with_mutex:
  nfs_mux_release(nmp);
  return -error;
}

int vfs_nfs_create(struct Vnode *parent, const char *filename, int mode, struct Vnode **vpp)
{
  uint32_t           *ptr = NULL;
  uint32_t            tmp;
  int                 namelen;
  int                 reqlen;
  int                 error;
  struct nfsnode *parent_nfs_node = (struct nfsnode *)parent->data;
  struct nfsmount *nmp = (struct nfsmount *)(parent->originMount->data);
  struct nfsnode *np = zalloc(sizeof(struct nfsnode));
  nfs_mux_take(nmp);
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }
  ptr    = (uint32_t *)&nmp->nm_msgbuffer.create.create;
  reqlen = 0;

  /* Copy the variable length, directory file handle */

  *ptr++  = txdr_unsigned(parent_nfs_node->n_fhsize);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, parent_nfs_node->n_fhsize, &parent_nfs_node->n_fhandle, parent_nfs_node->n_fhsize);
  reqlen += (int)parent_nfs_node->n_fhsize;
  ptr    += uint32_increment(parent_nfs_node->n_fhsize);

  /* Copy the variable-length file name */

  namelen = strlen(filename);

  *ptr++  = txdr_unsigned(namelen);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, namelen, filename, namelen);
  ptr    += uint32_increment(namelen);
  reqlen += uint32_alignup(namelen);

  /* Set the creation mode */

#ifdef USE_GUARDED_CREATE
  *ptr++  = htonl(NFSV3CREATE_GUARDED);
#else
  *ptr++  = htonl(NFSV3CREATE_EXCLUSIVE);
#endif

  reqlen += sizeof(uint32_t);

  /* Mode information is not provided if EXCLUSIVE creation is used.
   * in this case, we must call SETATTR after successfully creating
   * the file.
   */

  /* Set the mode.  NOTE: Here we depend on the fact that the NuttX and NFS
   * bit settings are the same (at least for the bits of interest).
   */

  *ptr++  = nfs_true; /* True: mode value follows */
  reqlen += sizeof(uint32_t);

  tmp = mode & (NFSMODE_IWOTH | NFSMODE_IROTH | NFSMODE_IWGRP |
      NFSMODE_IRGRP | NFSMODE_IWUSR | NFSMODE_IRUSR);
  *ptr++  = txdr_unsigned(tmp);
  reqlen += sizeof(uint32_t);

  /* Set the user ID to zero */

  *ptr++  = nfs_true;             /* True: Uid value follows */
  *ptr++  = 0;                    /* UID = 0 (nobody) */
  reqlen += 2*sizeof(uint32_t);

  /* Set the group ID to one */

  *ptr++  = nfs_true;            /* True: Gid value follows */
  *ptr++  = htonl(1);            /* GID = 1 (nogroup) */
  reqlen += 2*sizeof(uint32_t);

  /* Set the size to zero */

  *ptr++  = nfs_true;            /* True: Size value follows */
  *ptr++  = 0;                   /* Size = 0 */
  *ptr++  = 0;
  reqlen += 3*sizeof(uint32_t);

  /* Don't change times */

  *ptr++  = htonl(NFSV3SATTRTIME_DONTCHANGE); /* Don't change atime */
  *ptr++  = htonl(NFSV3SATTRTIME_DONTCHANGE); /* Don't change mtime */
  reqlen += 2*sizeof(uint32_t);

  /* Send the NFS request.  Note there is special logic here to handle version 3
   * exclusive open semantics.
   */

  do
    {
      nfs_statistics(NFSPROC_CREATE);
      error = nfs_request(nmp, NFSPROC_CREATE,
          (void *)&nmp->nm_msgbuffer.create, reqlen,
          (void *)nmp->nm_iobuffer, nmp->nm_buflen);
    }
  while (0);

  /* Check for success */

  if (error != OK)
    {
      *vpp = NULL;
      goto errout_with_mutex;
    }
    
  /* Parse the returned data */

  ptr = (uint32_t *)&((struct rpc_reply_create *)
      nmp->nm_iobuffer)->create;

  /* Save the file handle in the file data structure */

  tmp = *ptr++;  /* handle_follows */
  if (!tmp)
    {
      PRINTK("ERROR: no file handle follows\n");
      error = EINVAL;
      goto errout_with_mutex;
    }

  tmp = *ptr++;
  tmp = fxdr_unsigned(uint32_t, tmp);
  DEBUGASSERT(tmp <= NFSX_V3FHMAX);

  np->n_fhsize      = (uint8_t)tmp;
  (void)memcpy_s(&np->n_fhandle, tmp, ptr, tmp);
  ptr += uint32_increment(tmp);

  /* Save the attributes in the file data structure */

  tmp = *ptr;  /* handle_follows */
  if (!tmp)
    {
      PRINTK("WARNING: no file attributes\n");
    }
  else
    {
      /* Initialize the file attributes */

      nfs_attrupdate(np, (struct nfs_fattr *)ptr);
    }

  /* Any following dir_wcc data is ignored for now */
  np->n_crefs = 1;

  /* Attach the private data to the struct file instance */

  /* Then insert the new instance at the head of the list in the mountpoint
   * tructure. It needs to be there (1) to handle error conditions that effect
   * all files, and (2) to inform the umount logic that we are busy.  We
   * cannot unmount the file system if this list is not empty!
   */

  np->n_next   = nmp->nm_head;
  nmp->nm_head = np;

  np->n_flags |= (NFSNODE_OPEN | NFSNODE_MODIFIED);
  np->n_name = zalloc(namelen + 1);
  memcpy_s(np->n_name, (namelen + 1), filename, (namelen + 1));

  (void)VnodeAlloc(&nfs_vops, vpp);
  (*vpp)->parent = parent;
  (*vpp)->fop = &nfs_fops;
  (*vpp)->originMount = parent->originMount;
  (*vpp)->data = np;
  (*vpp)->type = filetype_to_vnodetype(np->n_type);
  (*vpp)->mode = type_to_mode((*vpp)->type, nmp->nm_permission);
  (*vpp)->gid = nmp->nm_gid;
  (*vpp)->uid = nmp->nm_uid;

  nfs_mux_release(nmp);
  return OK;

errout_with_mutex:
  if (np)
    {
      free(np);
    }
  nfs_mux_release(nmp);
  return -error;
}

int vfs_nfs_unlink(struct Vnode *parent, struct Vnode *target, const char *filename)
{
  struct nfsmount *nmp = (struct nfsmount *)(parent->originMount->data);
  struct nfsnode  *parent_node = NULL;
  struct nfsnode  *target_node = NULL;
  int reqlen;
  int namelen;
  uint32_t *ptr = NULL;
  int error;

  nfs_mux_take(nmp);
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }

  parent_node = (struct nfsnode*)(parent->data);
  target_node = (struct nfsnode*)(target->data);

  if (target_node->n_type == NFDIR)
    {
      PRINTK("ERROR: try to remove a directory\n");
      error = EISDIR;
      goto errout_with_mutex;
    }

  /* Create the REMOVE RPC call arguments */

  ptr    = (uint32_t *)&nmp->nm_msgbuffer.removef.remove;
  reqlen = 0;

  /* Copy the variable length, directory file handle */

  *ptr++  = txdr_unsigned(parent_node->n_fhsize);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, parent_node->n_fhsize, &parent_node->n_fhandle, parent_node->n_fhsize);
  reqlen += (int)parent_node->n_fhsize;
  ptr    += uint32_increment(parent_node->n_fhsize);

  /* Copy the variable-length file name */

  namelen = strlen(filename);

  *ptr++  = txdr_unsigned(namelen);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, namelen, filename, namelen);
  reqlen += uint32_alignup(namelen);

  /* Perform the REMOVE RPC call */

  nfs_statistics(NFSPROC_REMOVE);
  error = nfs_request(nmp, NFSPROC_REMOVE,
      (void *)&nmp->nm_msgbuffer.removef, reqlen,
      (void *)nmp->nm_iobuffer, nmp->nm_buflen);

errout_with_mutex:
  nfs_mux_release(nmp);
  return -error;
}

int vfs_nfs_rmdir(struct Vnode *parent, struct Vnode *target, const char *dirname)
{
  struct nfsmount *nmp = (struct nfsmount *)(parent->originMount->data);
  struct nfsnode  *parent_node = NULL;
  struct nfsnode  *target_node = NULL;
  int reqlen;
  int namelen;
  uint32_t *ptr = NULL;
  int error;
  nfs_mux_take(nmp);
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }

  parent_node = (struct nfsnode*)(parent->data);
  target_node = (struct nfsnode*)(target->data);

  if (target_node->n_type != NFDIR)
    {
      PRINTK("ERROR: try to remove a non-dir\n");
      return -ENOTDIR;
    }

  /* Set up the RMDIR call message arguments */

  ptr    = (uint32_t *)&nmp->nm_msgbuffer.rmdir.rmdir;
  reqlen = 0;

  /* Copy the variable length, directory file handle */

  *ptr++  = txdr_unsigned(parent_node->n_fhsize);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, parent_node->n_fhsize, &parent_node->n_fhandle, parent_node->n_fhsize);
  reqlen += (int)parent_node->n_fhsize;
  ptr    += uint32_increment(parent_node->n_fhsize);

  /* Copy the variable-length directory name */

  namelen = strlen(dirname);

  *ptr++  = txdr_unsigned(namelen);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, namelen, dirname, namelen);
  reqlen += uint32_alignup(namelen);

  /* Perform the RMDIR RPC */

  nfs_statistics(NFSPROC_RMDIR);
  error = nfs_request(nmp, NFSPROC_RMDIR,
      (void *)&nmp->nm_msgbuffer.rmdir, reqlen,
      (void *)nmp->nm_iobuffer, nmp->nm_buflen);

errout_with_mutex:
  nfs_mux_release(nmp);
  return -nfs_2_vfs(error);
}


int vfs_nfs_close(struct Vnode *node)
{
  struct nfsmount *nmp = (struct nfsmount *)(node->originMount->data);
  struct nfsnode  *np = NULL;
  nfs_mux_take(nmp);
  np = (struct nfsnode*)(node->data);
  /* Decrement the reference count.  If the reference count would not
   * decrement to zero, then that is all we have to do.
   */

  if (np->n_crefs > 1)
    {
      np->n_crefs--;
    }
  nfs_mux_release(nmp);
  return OK;
}

int vfs_nfs_close_file(struct file *filep)
{
  struct Vnode *node = (struct Vnode *)filep->f_vnode;
  return vfs_nfs_close(node);
}

int vfs_nfs_closedir(struct Vnode *node, struct fs_dirent_s *dir)
{
  struct nfsmount *nmp = (struct nfsmount *)(node->originMount->data);
  struct nfsdir_s  *prev = NULL;
  struct nfsdir_s  *curr = NULL;
  struct nfsdir_s      *nfs_dir;
  struct entry3        *entry_pos = NULL;
  int                  ret;

  /* Sanity checks */
  nfs_dir = (struct nfsdir_s *)(dir->u.fs_dir);

  DEBUGASSERT(nmp != NULL);

  /* Get exclusive access to the mount structure. */
  nfs_mux_take(nmp);


  for (entry_pos = nfs_dir->nfs_entries; entry_pos != NULL; entry_pos = nfs_dir->nfs_entries)
    {
      nfs_dir->nfs_entries = entry_pos->next;
      NFS_DIR_ENTRY_FREE(entry_pos);
    }

  /* Assume file structure will not be found.  This should never happen. */

  ret = EINVAL;

  for (prev = (struct nfsdir_s  *)NULL, curr = nmp->nm_dir;
      curr;
      prev = curr, curr = curr->nfs_next)
    {
      /* Check if this node is ours */

      if (nfs_dir == curr)
        {
          /* Yes.. remove it from the list of file structures */

          if (prev)
            {
              /* Remove from mid-list */

              prev->nfs_next = nfs_dir->nfs_next;
            }
          else
            {
              /* Remove from the head of the list */

              nmp->nm_dir= nfs_dir->nfs_next;
            }

          /* Then deallocate the file structure and return success */

          free(nfs_dir);
          nfs_dir = NULL;
          ret = OK;
          break;
        }
    }
  nfs_mux_release(nmp);

  return -ret; /*lint !e438*/
}

/****************************************************************************
 * Name: nfs_fsinfo
 *
 * Description:
 *   Return information about root directory.
 *
 * Returned Value:
 *   0 on success; positive errno value on failure
 *
 * Assumptions:
 *   The caller has exclusive access to the NFS mount structure
 *
 ****************************************************************************/

int nfs_fsinfo(struct nfsmount *nmp)
{
  struct rpc_call_fs fsinfo;
  struct rpc_reply_fsinfo fsp;
  struct nfs_fsinfo *rep_info = NULL;
  uint32_t pref;
  uint32_t max;
  int error = 0;

  fsinfo.fs.fsroot.length = txdr_unsigned(nmp->nm_fhsize);
  fsinfo.fs.fsroot.handle = nmp->nm_fh;

  /* Request FSINFO from the server */

  nfs_statistics(NFSPROC_FSINFO);
  error = nfs_request(nmp, NFSPROC_FSINFO,
                      (void *)&fsinfo, sizeof(struct FS3args),
                      (void *)&fsp, sizeof(struct rpc_reply_fsinfo));
  if (error)
    {
      return error;
    }

  if (txdr_unsigned(fsp.fsinfo.obj_attributes.obj_attribute_follow) == 1)
    {
      rep_info = (struct nfs_fsinfo *)&fsp.fsinfo.fs_rtmax;
    }
  else
    {
      rep_info = (struct nfs_fsinfo *)((void *)(&fsp.fsinfo.obj_attributes.attributes));
    }

  /* Save the root file system attributes */
  pref = fxdr_unsigned(uint32_t, rep_info->fs_wtpref);
  if (pref < nmp->nm_wsize)
    {
      nmp->nm_wsize = (pref + NFS_FABLKSIZE - 1) & ~(NFS_FABLKSIZE - 1);
    }

  max = fxdr_unsigned(uint32_t, rep_info->fs_wtmax);
  if (max < nmp->nm_wsize)
    {
      nmp->nm_wsize = max & ~(NFS_FABLKSIZE - 1);
      if (nmp->nm_wsize == 0)
        {
          nmp->nm_wsize = max;
        }
    }

  pref = fxdr_unsigned(uint32_t, rep_info->fs_rtpref);
  if (pref < nmp->nm_rsize)
    {
      nmp->nm_rsize = (pref + NFS_FABLKSIZE - 1) & ~(NFS_FABLKSIZE - 1);
    }

  max = fxdr_unsigned(uint32_t, rep_info->fs_rtmax);
  if (max < nmp->nm_rsize)
    {
      nmp->nm_rsize = max & ~(NFS_FABLKSIZE - 1);
      if (nmp->nm_rsize == 0)
        {
          nmp->nm_rsize = max;
        }
    }

  pref = fxdr_unsigned(uint32_t, rep_info->fs_dtpref);
  if (pref < nmp->nm_readdirsize)
    {
      nmp->nm_readdirsize = (pref + NFS_DIRBLKSIZ - 1) & ~(NFS_DIRBLKSIZ - 1);
    }

  if (max < nmp->nm_readdirsize)
    {
      nmp->nm_readdirsize = max & ~(NFS_DIRBLKSIZ - 1);
      if (nmp->nm_readdirsize == 0)
        {
          nmp->nm_readdirsize = max;
        }
    }

  return OK;
}

int vfs_nfs_statfs(struct Mount *mountpt, struct statfs *sbp)
{
  struct nfsmount *nmp;
  struct rpc_call_fs *fsstat = NULL;
  struct rpc_reply_fsstat *sfp = NULL;
  struct nfs_statfs_ctx  *stfp = NULL;
  int error = 0;
  uint64_t tquad;

  /* Get the mountpoint private data from the vnode structure */

  nmp = (struct nfsmount *)mountpt->data;

  /* Check if the mount is still healthy */

  nfs_mux_take(nmp);
  error = nfs_checkmount(nmp);
  if (error != OK)
    {
      PRINTK("ERROR: nfs_checkmount failed: %d\n", error);
      goto errout_with_mutex;
    }

  /* Fill in the statfs info */

  sbp->f_type = NFS_SUPER_MAGIC;

  error = nfs_fsinfo(nmp);
  if (error)
    {
      PRINTK("ERROR: nfs_fsinfo failed: %d\n", error);
      goto errout_with_mutex;
    }

  fsstat = &nmp->nm_msgbuffer.fsstat;
  fsstat->fs.fsroot.length = txdr_unsigned(nmp->nm_fhsize);
  (void)memcpy_s(&fsstat->fs.fsroot.handle, sizeof(nfsfh_t), &nmp->nm_fh, sizeof(nfsfh_t));

  nfs_statistics(NFSPROC_FSSTAT);
  error = nfs_request(nmp, NFSPROC_FSSTAT,
                      (void *)fsstat, sizeof(struct FS3args),
                      (void *)nmp->nm_iobuffer, nmp->nm_buflen);
  if (error)
    {
      goto errout_with_mutex;
    }

  sfp                   = (struct rpc_reply_fsstat *)nmp->nm_iobuffer;
  if (txdr_unsigned(sfp->fsstat.attributes_follow) == 1)
    {
      stfp = (struct nfs_statfs_ctx *)&sfp->fsstat.sf_tbytes;
    }
  else
    {
      stfp = (struct nfs_statfs_ctx *)&sfp->fsstat.obj_attributes;
    }

  sbp->f_bsize          = NFS_FABLKSIZE;
  tquad                 = fxdr_hyper(&stfp->sf_tbytes); /*lint !e571*/
  sbp->f_blocks         = tquad / (uint64_t) NFS_FABLKSIZE;
  tquad                 = fxdr_hyper(&stfp->sf_fbytes); /*lint !e571*/
  sbp->f_bfree          = tquad / (uint64_t) NFS_FABLKSIZE;
  tquad                 = fxdr_hyper(&stfp->sf_abytes); /*lint !e571*/
  sbp->f_bavail         = tquad / (uint64_t) NFS_FABLKSIZE;
  tquad                 = fxdr_hyper(&stfp->sf_tfiles); /*lint !e571*/
  sbp->f_files          = tquad;
  tquad                 = fxdr_hyper(&stfp->sf_ffiles); /*lint !e571*/
  sbp->f_ffree          = tquad;
  sbp->f_namelen        = NAME_MAX;
  sbp->f_flags          = mountpt->mountFlags;

errout_with_mutex:
  nfs_mux_release(nmp);
  return -error;
}

static int vfs_nfs_rewinddir(struct Vnode *node, struct fs_dirent_s *dir)
{
  struct nfsdir_s *nfs_dir = NULL;
  struct entry3   *entry_pos = NULL;

  struct nfsmount *nmp = (struct nfsmount *)(node->originMount->data);
  nfs_mux_take(nmp);
  /* Reset the NFS-specific portions of dirent structure, retaining only the
   * file handle.
   */

  nfs_dir = (struct nfsdir_s *)dir->u.fs_dir;
  (void)memset_s(nfs_dir->nfs_verifier, DIRENT_NFS_VERFLEN, 0, DIRENT_NFS_VERFLEN);
  nfs_dir->nfs_cookie[0] = 0;
  nfs_dir->nfs_cookie[1] = 0;
  for (entry_pos = nfs_dir->nfs_entries; entry_pos != NULL; entry_pos = nfs_dir->nfs_entries)
    {
      nfs_dir->nfs_entries = entry_pos->next;
      NFS_DIR_ENTRY_FREE(entry_pos);
    }
  free(nfs_dir->nfs_entries);
  nfs_dir->nfs_entries = NULL;
  nfs_mux_release(nmp);
  return OK;
}

int vfs_nfs_truncate(struct Vnode *node, off_t length)
{
  uint32_t *ptr;
  int reqlen;
  int error;

  struct nfsmount *nmp = NULL;
  struct nfsnode  *np = NULL;

  nmp = (struct nfsmount *)(node->originMount->data);
  nfs_mux_take(nmp);
  np = (struct nfsnode*)(node->data);

  /* Create the SETATTR RPC call arguments */

  ptr    = (uint32_t *)&nmp->nm_msgbuffer.setattr.setattr;
  reqlen = 0;

  /* Copy the variable length, directory file handle */

  *ptr++  = txdr_unsigned(np->n_fhsize);
  reqlen += sizeof(uint32_t);

  (void)memcpy_s(ptr, np->n_fhsize, &np->n_fhandle, np->n_fhsize);
  reqlen += (int)np->n_fhsize;
  ptr    += uint32_increment(np->n_fhsize);

  /* Copy the variable-length attributes */

  *ptr++  = nfs_false;                        /* Don't change mode */
  *ptr++  = nfs_false;                        /* Don't change uid */
  *ptr++  = nfs_false;                        /* Don't change gid */
  *ptr++  = nfs_true;                         /* Use the following size */
  *ptr++  = length;                           /* Truncate to the specified length */
  *ptr++  = 0;
  *ptr++  = htonl(NFSV3SATTRTIME_TOSERVER);   /* Use the server's time */
  *ptr++  = htonl(NFSV3SATTRTIME_TOSERVER);   /* Use the server's time */
  *ptr++  = nfs_false;                        /* No guard value */
  reqlen += 9 * sizeof(uint32_t);

  /* Perform the SETATTR RPC */

  nfs_statistics(NFSPROC_SETATTR);
  error = nfs_request(nmp, NFSPROC_SETATTR,
                      (void *)&nmp->nm_msgbuffer.setattr, reqlen,
                      (void *)nmp->nm_iobuffer, nmp->nm_buflen);
  if (error != OK)
    {
      nfs_mux_release(nmp);
      PRINTK("ERROR: nfs_request failed: %d\n", error);
      return -error;
    }

  /* Indicate that the file now has zero length */

  np->n_size = length;
  nfs_mux_release(nmp);
  return OK;
}

static int vfs_nfs_unmount(struct Mount *mnt, struct Vnode **blkDriver)
{
  (void)blkDriver;
  struct nfsmount *nmp = (struct nfsmount *)mnt->data;
  int error;

  DEBUGASSERT(nmp);

  /* Get exclusive access to the mount structure */

  nfs_mux_take(nmp);

  /* Are there any open files?  We can tell if there are open files by looking
   * at the list of file structures in the mount structure.  If this list
   * not empty, then there are open files and we cannot unmount now (or a
   * crash is sure to follow).
   */

  if (nmp->nm_head != NULL || nmp->nm_dir != NULL)
    {
      PRINT_ERR("There are open files: %p or directories: %p\n", nmp->nm_head, nmp->nm_dir);

      /* This implementation currently only supports unmounting if there are
       * no open file references.
       */

      error = EBUSY;
      goto errout_with_mutex;
    }

  /* No open file... Umount the file system. */

  error = rpcclnt_umount(nmp->nm_rpcclnt);
  if (error)
    {
      PRINT_ERR("rpcclnt_umount failed: %d\n", error);
      goto errout_with_mutex;
    }

  /* Disconnect from the server */

  rpcclnt_disconnect(nmp->nm_rpcclnt);

  /* And free any allocated resources */

  nfs_mux_release(nmp);
  (void)pthread_mutex_destroy(&nmp->nm_mux);
  free(nmp->nm_rpcclnt);
  nmp->nm_rpcclnt = NULL;
  free(nmp);
  nmp = NULL;

  return -error;

errout_with_mutex:
  nfs_mux_release(nmp);
  return -error;
}

struct MountOps nfs_mount_operations =
{
  .Mount = vfs_nfs_mount,
  .Unmount = vfs_nfs_unmount,
  .Statfs= vfs_nfs_statfs,
};

struct VnodeOps nfs_vops =
{
  .Lookup = vfs_nfs_lookup,
  .Getattr = vfs_nfs_stat,
  .Opendir = vfs_nfs_opendir,
  .Readdir = vfs_nfs_readdir,
  .Rename = vfs_nfs_rename,
  .Mkdir = vfs_nfs_mkdir,
  .Create = vfs_nfs_create,
  .Unlink =  vfs_nfs_unlink,
  .Rmdir = vfs_nfs_rmdir,
  .Reclaim = vfs_nfs_reclaim,
  .Closedir = vfs_nfs_closedir,
  .Close = vfs_nfs_close,
  .Rewinddir = vfs_nfs_rewinddir,
  .Truncate = vfs_nfs_truncate,
};

struct file_operations_vfs nfs_fops =
{
  .seek = vfs_nfs_seek,
  .write = vfs_nfs_write,
  .read = vfs_nfs_read,
  .mmap = OsVfsFileMmap,
  .close = vfs_nfs_close_file,
};
FSMAP_ENTRY(nfs_fsmap, "nfs", nfs_mount_operations, FALSE, FALSE);
#endif
