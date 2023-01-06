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

#ifndef _LOS_MEMORY_PRI_H
#define _LOS_MEMORY_PRI_H

#include "los_memory.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define OS_MEM_ENABLE_MEM_STATISTICS

/*
 * memcheck error code: the stack have not inited
 * Value: 0x02000100
 * Solution: do memcheck must after stack mem init
 */
#define  OS_ERRNO_MEMCHECK_NOT_INIT      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x0)

/*
 *  memcheck error code: the pPtr is NULL
 *  Value: 0x02000101
 *  Solution: don't give a NULL parameter
 */
#define  OS_ERRNO_MEMCHECK_PARA_NULL      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x1)

/*
 *  memcheck error code: the pPtr addr not in the suit range
 *  Value: 0x02000102
 *  Solution: check pPtr and comfirm it included by stack
 */
#define  OS_ERRNO_MEMCHECK_OUTSIDE      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x2)

/*
 *  memcheck error code: can't find the ctrl node
 *  Value: 0x02000103
 *  Solution: confirm the pPtr if this node has been freed or has not been alloced
 */
#define  OS_ERRNO_MEMCHECK_NO_HEAD      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x3)

/*
 *  memcheck error code: the para level is wrong
 *  Value: 0x02000104
 *  Solution: checkout the memcheck level by the func "OS_GetMemCheck_Level"
 */
#define  OS_ERRNO_MEMCHECK_WRONG_LEVEL      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x4)

/*
 *  memcheck error code: memcheck func not open
 *  Value: 0x02000105
 *  Solution: enable memcheck by the func "OS_SetMemCheck_Level"
 */
#define  OS_ERRNO_MEMCHECK_DISABLED      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x5)

/**
 * @ingroup los_memory
 * @brief Initialization the memory system.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to initialization the memory system.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  None.
 *
 * @retval UINT32 Initialization result.
 * @par Dependency:
 * <ul><li>los_memory_pri.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 OsMemSystemInit(VOID);

/**
 * @ingroup los_memory
 * Memory linked list node structure
 */
typedef struct tagLosMemDynNode {
    LOS_DL_LIST freeNodeInfo;         /**< Free memory node  */
    struct tagLosMemDynNode *preNode; /**< Pointer to the previous memory node */
    UINT32 sizeAndFlag;               /**< Size and flag of the current node(the highest bit
                                           represents a flag, and the rest bits specify the size) */
} LosMemDynNode;

#define OS_MEM_TASKID_SET(node, ID) \
    do { \
        UINTPTR tmp = (UINTPTR)(((LosMemDynNode *)(node))->freeNodeInfo.pstNext); \
        tmp = tmp & 0xffff0000; \
        tmp |= (ID); \
        ((LosMemDynNode *)(node))->freeNodeInfo.pstNext = (LOS_DL_LIST *)(tmp); \
    } while (0)

#define OS_MEM_TASKID_GET(node) ((UINT32)(UINTPTR)(((LosMemDynNode *)(node))->freeNodeInfo.pstNext) & 0xffff)

#define OS_MEM_MODID_SET(node, ID) \
    do { \
        UINTPTR tmp = (UINTPTR)(((LosMemDynNode *)(node))->freeNodeInfo.pstNext); \
        tmp = tmp & 0xffff; \
        tmp |= (ID) << 16; \
        ((LosMemDynNode *)(node))->freeNodeInfo.pstNext = (LOS_DL_LIST *)(tmp); \
    } while (0)

#define OS_MEM_MODID_GET(node) ((UINT32)(((LosMemDynNode *)(node))->freeNodeInfo.pstNext) >> 16)

#define OS_MEM_NODE_HEAD_SIZE    sizeof(LosMemDynNode)
#define OS_MEM_MIN_POOL_SIZE     (OS_DLNK_HEAD_SIZE + (2 * OS_MEM_NODE_HEAD_SIZE) + sizeof(LOS_MEM_POOL_INFO))
#define OS_MEM_ALIGN_SIZE        4
#define OS_MEM_NODE_USED_FLAG    0x80000000
#define OS_MEM_NODE_ALIGNED_FLAG 0x40000000
#define OS_MEM_NODE_ALIGN_SIZE   64
#define OS_MEM_NODE_NUM          2
#define OS_MEM_NODE_DATA_SIZE    4
#define OS_MEM_NODE_COUNT_NUM    8
#define OS_MULTI_DLNK_SIZE       (OS_MAX_MULTI_DLNK_LOG2 - OS_MIN_MULTI_DLNK_LOG2)

#define OS_MEM_NODE_GET_ALIGNED_FLAG(sizeAndFlag) ((sizeAndFlag) & OS_MEM_NODE_ALIGNED_FLAG)
#define OS_MEM_NODE_SET_ALIGNED_FLAG(sizeAndFlag) ((sizeAndFlag) = ((sizeAndFlag) | OS_MEM_NODE_ALIGNED_FLAG))
#define OS_MEM_NODE_GET_ALIGNED_GAPSIZE(sizeAndFlag) ((sizeAndFlag) & (~OS_MEM_NODE_ALIGNED_FLAG))
#define OS_MEM_NODE_GET_USED_FLAG(sizeAndFlag) ((sizeAndFlag) & OS_MEM_NODE_USED_FLAG)
#define OS_MEM_NODE_SET_USED_FLAG(sizeAndFlag) ((sizeAndFlag) = ((sizeAndFlag) | OS_MEM_NODE_USED_FLAG))
#define OS_MEM_NODE_GET_SIZE(sizeAndFlag) ((sizeAndFlag) & (~OS_MEM_NODE_USED_FLAG))
#define OS_MEM_IS_NODE_NEXT_EXIST(node, poolInfo) (((UINT32)(node) + (node)->sizeAndFlag) < \
                                                         ((UINT32)(poolInfo) + (poolInfo)->uwPoolSize))
#define OS_MEM_HEAD(pool, size) OS_DLNK_HEAD(OS_MEM_HEAD_ADDR(pool), size)
#define OS_MEM_HEAD_ADDR(pool) ((VOID *)((UINT32)(UINTPTR)(pool) + sizeof(LOS_MEM_POOL_INFO)))
#define OS_MEM_NEXT_NODE(node) ((LosMemDynNode *)((UINT8 *)(node) + \
                                   OS_MEM_NODE_GET_SIZE((node)->sizeAndFlag)))
#define OS_MEM_FIRST_NODE(pool) ((LosMemDynNode *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE))
#define OS_MEM_END_NODE(pool, size) ((LosMemDynNode *)(((UINT8 *)(pool) + (size)) - OS_MEM_NODE_HEAD_SIZE))
#define OS_MEM_MIDDLE_ADDR_OPEN_END(startAddr, middleAddr, endAddr)  \
    (((UINT8 *)(startAddr) <= (UINT8 *)(middleAddr)) && ((UINT8 *)(middleAddr) < (UINT8 *)(endAddr)))
#define OS_MEM_MIDDLE_ADDR(startAddr, middleAddr, endAddr) (((UINT8 *)(startAddr) <= (UINT8 *)(middleAddr)) && \
                                                            ((UINT8 *)(middleAddr) <= (UINT8 *)(endAddr)))
#define OS_MEM_SET_MAGIC(value) (value) = (LOS_DL_LIST *)(UINTPTR)((UINT32)(UINTPTR)(&(value)) ^ 0xffffffff)
#define OS_MEM_MAGIC_VALID(value) ((((UINT32)(UINTPTR)(value)) ^ ((UINT32)(UINTPTR)(&(value)))) == 0xffffffff)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_MEMORY_PRI_H */
