/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "linux/kernel.h"
#include "linux/module.h"
#include "math.h"
#include "limits.h"
#include "sys/statfs.h"
#include "los_sys_pri.h"
#include "los_swtmr.h"
#ifdef LOSCFG_KERNEL_DYNLOAD
#include "los_ld_elflib.h"
#endif
#ifdef LOSCFG_NET_LWIP_SACK
#include "lwip/sockets.h"
#include "lwip/api.h"
#include "lwip/netdb.h"
#endif
#include "linux/rbtree.h"
#include "los_vm_common.h"
#include "los_vm_zone.h"
#include "los_vm_lock.h"


#ifdef __LP64__
int dl_iterate_phdr(int (*callback)(void *info, size_t size, void *data), void *data)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    errno = ENOSYS;
    return -1;
}
#endif

void fs_show(const char *path)
{
    INT32 ret;
    struct statfs fss;
    if (path == NULL) {
        PRINTK("path is NULL\n");
        return;
    }
    ret = statfs(path, &fss);
    PRINTK("Filesystem %s info: \n", path);
    PRINTK("----------------------------------------\n");
    if (ret == ENOERR) {
        PRINTK("  Total clusters: %u \n", fss.f_blocks);
        PRINTK("  Cluster size: %u \n", fss.f_bsize);
        PRINTK("  Free clusters: %u \n", fss.f_bfree);
    } else {
        ret = get_errno();
        PRINT_ERR("Get fsinfo failed: %d \n", ret);
    }
}

#define MAX_JIFFY_OFFSET ((LONG_MAX >> 1) - 1)

unsigned long msecs_to_jiffies(const unsigned int m)
{
    /* Negative value, means infinite timeout: */
    if ((INT32)m < 0) {
        return (unsigned long)MAX_JIFFY_OFFSET;
    }

#if (HZ <= OS_SYS_MS_PER_SECOND) && !(OS_SYS_MS_PER_SECOND % HZ)
    /*
     * HZ is equal to or smaller than 1000, and 1000 is a nice
     * round multiple of HZ, divide with the factor between them,
     * but round upwards:
     */
    return ((m + (OS_SYS_MS_PER_SECOND / HZ)) - 1) / (OS_SYS_MS_PER_SECOND / HZ);
#else
    PRINT_ERR("HZ: %d is not supported in %s\n", HZ, __FUNCTION__);
    return ENOSUPP;
#endif
}

UINT64 jiffies_to_tick(unsigned long j)
{
    return j;
}

#define MAX_SCHEDULE_TIMEOUT UINT_MAX
signed long schedule_timeout(signed long timeout)
{
    UINT32 ret;

    if (OS_INT_ACTIVE) {
        PRINT_ERR("ERROR: OS_ERRNO_SWTMR_HWI_ACTIVE\n");
        return LOS_ERRNO_SWTMR_HWI_ACTIVE;
    }
    if (timeout < 0) {
        PRINT_ERR("schedule_timeout: wrong timeout\n");
        return 0;
    }
#ifdef __LP64__
    if (timeout > MAX_SCHEDULE_TIMEOUT) {
        timeout = LOS_WAIT_FOREVER;
    }
#endif
    ret = LOS_TaskDelay(timeout);
    if (ret == LOS_OK) {
        return ret;
    } else {
        PRINT_ERR("ERROR: OS_ERRNO_SWTMR_NOT_STARTED\n");
        return LOS_ERRNO_SWTMR_NOT_STARTED;
    }
}

UINT32 do_div_imp(UINT64 *n, UINT32 base)
{
    UINT32 r;

    if ((n == NULL) || (base == 0)) {
        PRINT_ERR("%s invalid input param, n:%p, base %u\n", __FUNCTION__, n, base);
        return 0;
    }

    r = *n % base;
    *n = *n / base;
    return r;
}

INT32 do_div_s64_imp(INT64 *n, INT32 base)
{
    INT32 r;

    if ((n == NULL) || (base == 0)) {
        PRINT_ERR("%s invalid input param, n:%p, base:%u\n", __FUNCTION__, n, base);
        return 0;
    }

    r = *n % base;
    *n = *n / base;
    return r;
}

char *basename(const char *path)
{
    STATIC const CHAR empty[] = ".";
    CHAR *first = (CHAR *)empty;
    register CHAR *last = NULL;

    if ((path != NULL) && *path) {
        first = (CHAR *)path;
        last = (CHAR *)path - 1;

        do {
            if ((*path != '/') && (path > ++last)) {
                last = first = (CHAR *)path;
            }
        } while (*++path);

        if (*first == '/') {
            last = first;
        }
        last[1] = 0;
    }

    return first;
}

void *__dso_handle = NULL;

/* Undo Linux compat changes. */
#undef RB_ROOT
#define RB_ROOT(head)   (head)->rbh_root

int panic_cmp(struct rb_node *one, struct rb_node *two)
{
    LOS_Panic("no cmp");
    return 0;
}

RB_GENERATE(linux_root, rb_node, __entry, panic_cmp);

#define IS_PERIPH_ADDR(addr) \
    (((addr) >= U32_C(PERIPH_PMM_BASE)) && ((addr) <= U32_C(PERIPH_PMM_BASE) + U32_C(PERIPH_PMM_SIZE)))
#define IS_MEMORY_ADDR(addr) \
    (((addr) >= U32_C(DDR_MEM_ADDR)) && ((addr) <= U32_C(DDR_MEM_ADDR) + U32_C(DDR_MEM_SIZE)))

VOID *ioremap(PADDR_T paddr, unsigned long size)
{
    if (IS_PERIPH_ADDR(paddr) && IS_PERIPH_ADDR(paddr + size)) {
        return (VOID *)(UINTPTR)IO_DEVICE_ADDR(paddr);
    }

    VM_ERR("ioremap failed invalid addr or size %p %d", paddr, size);
    return (VOID *)(UINTPTR)paddr;
}

VOID iounmap(VOID *vaddr) {}

VOID *ioremap_nocache(PADDR_T paddr, unsigned long size)
{
    if (IS_PERIPH_ADDR(paddr) && IS_PERIPH_ADDR(paddr + size)) {
        return (VOID *)(UINTPTR)IO_UNCACHED_ADDR(paddr);
    }

    if (IS_MEMORY_ADDR(paddr) && IS_MEMORY_ADDR(paddr + size)) {
        return (VOID *)(UINTPTR)MEM_UNCACHED_ADDR(paddr);
    }

    VM_ERR("ioremap_nocache failed invalid addr or size %p %d", paddr, size);
    return (VOID *)(UINTPTR)paddr;
}

VOID *ioremap_cached(PADDR_T paddr, unsigned long size)
{
    if (IS_PERIPH_ADDR(paddr) && IS_PERIPH_ADDR(paddr + size)) {
        return (VOID *)(UINTPTR)IO_CACHED_ADDR(paddr);
    }

    if (IS_MEMORY_ADDR(paddr) && IS_MEMORY_ADDR(paddr + size)) {
        return (VOID *)(UINTPTR)MEM_CACHED_ADDR(paddr);
    }

    VM_ERR("ioremap_cached failed invalid addr or size %p %d", paddr, size);
    return (VOID *)(UINTPTR)paddr;
}

#ifdef LOSCFG_KERNEL_VM
int remap_pfn_range(VADDR_T vaddr, unsigned long pfn, unsigned long size, unsigned long prot)
{
    STATUS_T status = LOS_OK;
    int ret;
    LosVmMapRegion *region = NULL;
    unsigned long vpos;
    unsigned long end;
    unsigned long paddr = pfn << PAGE_SHIFT;
    LosVmSpace *space = LOS_SpaceGet(vaddr);

    if (size == 0) {
        VM_ERR("invalid map size %u", size);
        return LOS_ERRNO_VM_INVALID_ARGS;
    }
    size = ROUNDUP(size, PAGE_SIZE);

    if (!IS_PAGE_ALIGNED(vaddr) || pfn == 0) {
        VM_ERR("invalid map map vaddr %x or pfn %x", vaddr, pfn);
        return LOS_ERRNO_VM_INVALID_ARGS;
    }

    if (space == NULL) {
        VM_ERR("aspace not exists");
        return LOS_ERRNO_VM_NOT_FOUND;
    }

    (VOID)LOS_MuxAcquire(&space->regionMux);

    region = LOS_RegionFind(space, vaddr);
    if (region == NULL) {
        VM_ERR("region not exists");
        status = LOS_ERRNO_VM_NOT_FOUND;
        goto OUT;
    }
    end = vaddr + size;
    if (region->range.base + region->range.size < end) {
        VM_ERR("out of range:base=%x size=%d vaddr=%x len=%u",
               region->range.base, region->range.size, vaddr, size);
        status = LOS_ERRNO_VM_INVALID_ARGS;
        goto OUT;
    }

    /* check */
    for (vpos = vaddr; vpos < end; vpos += PAGE_SIZE) {
        status = LOS_ArchMmuQuery(&space->archMmu, (VADDR_T)vpos, NULL, NULL);
        if (status == LOS_OK) {
            VM_ERR("remap_pfn_range, address mapping already exist");
            status = LOS_ERRNO_VM_INVALID_ARGS;
            goto OUT;
        }
    }

    /* map all */
    ret = LOS_ArchMmuMap(&space->archMmu, vaddr, paddr, size >> PAGE_SHIFT, prot);
    if (ret <= 0) {
        VM_ERR("ioremap LOS_ArchMmuMap failed err = %d", ret);
        goto OUT;
    }

    status = LOS_OK;

OUT:
    (VOID)LOS_MuxRelease(&space->regionMux);
    return status;
}
#endif
