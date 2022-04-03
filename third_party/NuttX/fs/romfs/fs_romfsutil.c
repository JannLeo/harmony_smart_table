/****************************************************************************
 * rm/romfs/fs_romfsutil.c
 *
 *   Copyright (C) 2008-2009, 2013, 2017 Gregory Nutt. All rights reserved.
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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>

#ifdef LOSCFG_FS_ROMFS
#include "fs_romfs.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: romfs_swap32
 *
 * Description:
 *   Convert the 32-bit big-endian value to little endian
 *
 ****************************************************************************/

inline uint32_t romfs_swap32(uint32_t value)
{
  return ((((value) & 0x000000ff) << 24) | (((value) & 0x0000ff00) << 8) |
          (((value) & 0x00ff0000) >>  8) | (((value) & 0xff000000) >> 24));
}

/****************************************************************************
 * Name: romfs_devread32
 *
 * Description:
 *   Read the big-endian 32-bit value from the mount device buffer
 *
 * Assumption:
 *   All values are aligned to 32-bit boundaries
 *
 ****************************************************************************/

static uint32_t romfs_devread32(struct romfs_mountpt_s *rm, int ndx)
{
  /* Extract the value */

  uint32_t value = *(uint32_t *)&rm->rm_buffer[ndx];

  /* Convert the big-endian value to native host endianness. */

  return romfs_swap32(value);
}

/****************************************************************************
 * Name: romfs_checkentry
 *
 * Description:
 *   Check if the entry at offset is a directory or file path segment
 *
 ****************************************************************************/

static inline int romfs_checkentry(struct romfs_mountpt_s *rm,
                                   uint32_t offset, const char *entryname,
                                   int entrylen,
                                   struct romfs_dirinfo_s *dirinfo)
{
  char name[NAME_MAX + 1];
  uint32_t linkoffset;
  uint32_t next;
  uint32_t info;
  uint32_t size;
  int ret;

  /* Parse the directory entry at this offset (which may be re-directed
   * to some other entry if HARLINKED).
   */

  ret = romfs_parsedirentry(rm, offset, &linkoffset, &next, &info, &size);
  if (ret < 0)
    {
      return ret;
    }

  /* Now we are pointing to the real entry of interest. Is it a
   * directory? Or a file?
   */

  if (IS_DIRECTORY(next) || IS_FILE(next))
    {
      /* Get the name of the directory entry. */

      ret = romfs_parsefilename(rm, offset, name);
      if (ret < 0)
        {
          return ret;
        }

      /* Then check if this the name segment we are looking for.  The
       * string comparison is awkward because there is no terminator
       * on entryname (there is a terminator on name, however)
       */

      if (memcmp(entryname, name, entrylen) == 0 &&
          strlen(name) == entrylen)
        {
          /* Found it -- save the component info and return success */

          if (IS_DIRECTORY(next))
            {
              dirinfo->rd_dir.fr_firstoffset = info;
              dirinfo->rd_dir.fr_curroffset  = info;
              dirinfo->rd_size               = 0;
            }
          else
            {
              dirinfo->rd_dir.fr_curroffset  = offset;
              dirinfo->rd_size               = size;
            }

          dirinfo->rd_next                   = next;
          return OK;
        }
    }

  /* The entry is not a directory or it does not have the matching name */

  return -ENOENT;
}

/****************************************************************************
 * Name: romfs_devcacheread
 *
 * Description:
 *   Read the specified sector for specified offset into the sector cache.
 *   Return the index into the sector corresponding to the offset
 *
 ****************************************************************************/

uint32_t romfs_devcacheread(struct romfs_mountpt_s *rm, uint32_t offset)
{
  return offset;
}

/****************************************************************************
 * Name: romfs_followhardlinks
 *
 * Description:
 *   Given the offset to a file header, check if the file is a hardlink.
 *   If so, traverse the hard links until the terminal, non-linked header
 *   so found and return that offset.
 *
 ****************************************************************************/

static int romfs_followhardlinks(struct romfs_mountpt_s *rm, uint32_t offset,
                                 uint32_t *poffset)
{
  uint32_t next;
  uint32_t ndx;
  int      i;

  /* Loop while we are redirected by hardlinks */

  for (i = 0; i < ROMF_MAX_LINKS; i++)
    {
      /* Read the sector containing the offset into memory */

      ndx = romfs_devcacheread(rm, offset);
      if (ndx < 0)
        {
          return ndx;
        }

      /* Check if this is a hard link */

      next = romfs_devread32(rm, ndx + ROMFS_FHDR_NEXT);
      if (!IS_HARDLINK(next))
        {
          *poffset = offset;
          return OK;
        }

      /* Follow the hard-link */

      offset = romfs_devread32(rm, ndx + ROMFS_FHDR_INFO);
    }

  return -ELOOP;
}

/****************************************************************************
 * Name: romfs_searchdir
 *
 * Description:
 *   This is part of the romfs_finddirentry log.  Search the directory
 *   beginning at dirinfo->fr_firstoffset for entryname.
 *
 ****************************************************************************/

int romfs_searchdir(struct romfs_mountpt_s *rm,
                                  const char *entryname, int entrylen, uint32_t firstoffset,
                                  struct romfs_dirinfo_s *dirinfo)
{
  uint32_t offset;
  uint32_t next;
  uint32_t ndx;
  int      ret;

  /* Then loop through the current directory until the directory
   * with the matching name is found.  Or until all of the entries
   * the directory have been examined.
   */

  offset = firstoffset;
  do
    {
      /* Read the sector into memory (do this before calling
       * romfs_checkentry() so we won't have to read the sector
       * twice in the event that the offset refers to a hardlink).
       */

      ndx = romfs_devcacheread(rm, offset);
      if (ndx < 0)
        {
          return ndx;
        }

      /* Because everything is chunked and aligned to 16-bit boundaries,
       * we know that most the basic node info fits into the sector.
       */

      next = romfs_devread32(rm, ndx + ROMFS_FHDR_NEXT) & RFNEXT_OFFSETMASK;

      /* Check if the name this entry is a directory with the matching
       * name
       */

      ret = romfs_checkentry(rm, offset, entryname, entrylen, dirinfo);
      if (ret == OK)
        {
          /* Its a match! Return success */

          return OK;
        }

      /* No match... select the offset to the next entry */

      offset = next;
    }
  while (next != 0);

  /* There is nothing in this directory with that name */

  return -ENOENT;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: romfs_semtake
 ****************************************************************************/

void romfs_semtake(struct romfs_mountpt_s *rm)
{
  int ret;

  do
    {
      /* Take the semaphore (perhaps waiting) */

      ret = sem_wait(&rm->rm_sem);

      /* The only case that an error should occur here is if the wait was
       * awakened by a signal.
       */

      DEBUGASSERT(ret == OK || ret == -EINTR);
    }
  while (ret == -EINTR);
}

/****************************************************************************
 * Name: romfs_semgive
 ****************************************************************************/

void romfs_semgive(struct romfs_mountpt_s *rm)
{
  (void)sem_post(&rm->rm_sem);
}

/****************************************************************************
 * Name: romfs_hwconfigure
 *
 * Description:
 *   This function is called as part of the ROMFS mount operation   It
 *   configures the ROMFS filestem for use on this block driver.  This includes
 *   the accounting for the geometry of the device, setting up any XIP modes
 *   of operation, and/or allocating any cache buffers.
 *
 ****************************************************************************/

int romfs_hwconfigure(struct romfs_mountpt_s *rm)
{
  uint32_t total_size;

  rm->rm_buffer = (uint8_t *)DMA_TO_VMM_ADDR(RAMDISK_ADDR);
  total_size = romfs_devread32(rm, ROMFS_VHDR_SIZE);

  rm->rm_hwnsectors = total_size;
  rm->rm_hwsectorsize = 1;
  rm->rm_cachesector = (uint32_t)-1;
  rm->rm_volsize = total_size;

  rm->rm_buffer = (uint8_t *)malloc(total_size);
  if (!rm->rm_buffer)
    {
      return -ENOMEM;
    }

  memcpy(rm->rm_buffer, (void *)DMA_TO_VMM_ADDR(RAMDISK_ADDR), total_size);

  return OK;
}

/****************************************************************************
 * Name: romfs_fsconfigure
 *
 * Description:
 *   This function is called as part of the ROMFS mount operation   It
 *   sets up the mount structure to include configuration information contained
 *   in the ROMFS header.  This is the place where we actually determine if
 *   the media contains a ROMFS filesystem.
 *
 ****************************************************************************/

int romfs_fsconfigure(struct romfs_mountpt_s *rm)
{
  const char *name;
  uint32_t    ndx;

  /* Then get information about the ROMFS filesystem on the devices managed
   * by this block driver.  Read sector zero which contains the volume header.
   */

  ndx = romfs_devcacheread(rm, 0);
  if (ndx < 0)
    {
      return ndx;
    }

  /* Verify the magic number at that identifies this as a ROMFS filesystem */

  if (memcmp(rm->rm_buffer, ROMFS_VHDR_MAGIC, 8) != 0)
    {
      return -EINVAL;
    }

  /* The root directory entry begins right after the header */

  name              = (FAR const char *)&rm->rm_buffer[ROMFS_VHDR_VOLNAME];
  rm->rm_rootoffset = ROMFS_ALIGNUP(ROMFS_VHDR_VOLNAME + strlen(name) + 1);

  /* and return success */

  rm->rm_mounted    = true;
  return OK;
}

/****************************************************************************
 * Name: romfs_checkmount
 *
 * Description: Check if the mountpoint is still valid.
 *
 *   The caller should hold the mountpoint semaphore
 *
 ****************************************************************************/

int romfs_checkmount(struct romfs_mountpt_s *rm)
{
  return OK;
}

/****************************************************************************
 * Name: romfs_parsedirentry
 *
 * Description:
 *   Return the directory entry at this offset.  If rf is NULL, then the
 *   mount device resources are used.  Otherwise, file resources are used.
 *
 ****************************************************************************/

int romfs_parsedirentry(struct romfs_mountpt_s *rm, uint32_t offset,
                        uint32_t *poffset, uint32_t *pnext, uint32_t *pinfo,
                        uint32_t *psize)
{
  uint32_t save;
  uint32_t next;
  uint32_t ndx;
  int      ret;

  /* Read the sector into memory */

  ndx = romfs_devcacheread(rm, offset);
  if (ndx < 0)
    {
      return ndx;
    }

  /* Yes.. Save the first 'next' value.  That has the offset needed to
   * traverse the parent directory.  But we may need to change the type
   * after we follow the hard links.
   */

  save = romfs_devread32(rm, ndx + ROMFS_FHDR_NEXT);

  /* Traverse hardlinks as necessary to get to the real file header */

  ret = romfs_followhardlinks(rm, offset, poffset);
  if (ret < 0)
    {
      return ret;
    }

  if (*poffset != offset)
    {
      ndx = romfs_devcacheread(rm, *poffset);
      if (ndx < 0)
        {
          return ndx;
        }
    }

  /* Because everything is chunked and aligned to 16-bit boundaries,
   * we know that most the basic node info fits into the sector.  The
   * associated name may not, however.
   */

  next   = romfs_devread32(rm, ndx + ROMFS_FHDR_NEXT);
  *pnext = (save & RFNEXT_OFFSETMASK) | (next & RFNEXT_ALLMODEMASK);
  *pinfo = romfs_devread32(rm, ndx + ROMFS_FHDR_INFO);
  *psize = romfs_devread32(rm, ndx + ROMFS_FHDR_SIZE);

  return OK;
}

/****************************************************************************
 * Name: romfs_parsefilename
 *
 * Description:
 *   Return the filename from directory entry at this offset
 *
 ****************************************************************************/

int romfs_parsefilename(struct romfs_mountpt_s *rm, uint32_t offset,
                        char *pname)
{
  uint32_t ndx;
  uint16_t namelen;
  uint16_t chunklen;
  bool     done;

  /* Loop until the whole name is obtained or until NAME_MAX characters
   * of the name have been parsed.
   */

  offset += ROMFS_FHDR_NAME;
  for (namelen = 0, done = false; namelen < NAME_MAX && !done; )
    {
      /* Read the sector into memory */

      ndx = romfs_devcacheread(rm, offset + namelen);
      if (ndx < 0)
        {
          return ndx;
        }

      /* Is the name terminated in this 16-byte block */

      if (rm->rm_buffer[ndx + 15] == '\0')
        {
          /* Yes.. then this chunk is less than 16 */

          chunklen = strlen((FAR char *)&rm->rm_buffer[ndx]);
          done     = true;
        }
      else
        {
          /* No.. then this chunk is 16 bytes in length */

          chunklen = 16;
        }

      /* Check if we would exceed the NAME_MAX */

      if (namelen + chunklen > NAME_MAX)
        {
          chunklen = NAME_MAX - namelen;
          done     = true;
        }

      /* Copy the chunk */

      memcpy(&pname[namelen], &rm->rm_buffer[ndx], chunklen);
      namelen += chunklen;
    }

  /* Terminate the name (NAME_MAX+1 chars total) and return success */

  pname[namelen] = '\0';

  return OK;
}

/****************************************************************************
 * Name: romfs_datastart
 *
 * Description:
 *   Given the offset to a file header, return the offset to the start of
 *   the file data
 *
 ****************************************************************************/

int romfs_datastart(struct romfs_mountpt_s *rm, uint32_t offset,
                    uint32_t *start)
{
  uint32_t ndx;
  int      ret;

  /* Traverse hardlinks as necessary to get to the real file header */

  ret = romfs_followhardlinks(rm, offset, &offset);
  if (ret < 0)
    {
      return ret;
    }

  /* Loop until the header size is obtained. */

  offset += ROMFS_FHDR_NAME;
  for (; ; )
    {
      /* Read the sector into memory */

      ndx = romfs_devcacheread(rm, offset);
      if (ndx < 0)
        {
          return ndx;
        }

      /* Get the offset to the next chunk */

      offset += 16;
      if (offset >= rm->rm_volsize)
        {
          return -EIO;
        }

      /* Is the name terminated in this 16-byte block */

      if (rm->rm_buffer[ndx + 15] == '\0')
        {
          /* Yes.. then the data starts at the next chunk */

          *start = offset;
          return OK;
        }
    }

  return -EINVAL; /* Won't get here */
}

#endif