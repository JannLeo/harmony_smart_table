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

#ifndef _LOS_QUEUE_PRI_H
#define _LOS_QUEUE_PRI_H

#include "los_queue.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef enum {
    OS_QUEUE_READ,
    OS_QUEUE_WRITE
} QueueReadWrite;

typedef enum {
    OS_QUEUE_HEAD,
    OS_QUEUE_TAIL
} QueueHeadTail;

typedef enum {
    OS_QUEUE_NOT_POINT,
    OS_QUEUE_POINT
} QueuePointOrNot;

#define OS_QUEUE_OPERATE_TYPE(ReadOrWrite, HeadOrTail, PointOrNot)  \
                (((UINT32)(PointOrNot) << 2) | ((UINT32)(HeadOrTail) << 1) | (ReadOrWrite))
#define OS_QUEUE_READ_WRITE_GET(type) ((type) & (0x01))
#define OS_QUEUE_READ_HEAD     (OS_QUEUE_READ | (OS_QUEUE_HEAD << 1))
#define OS_QUEUE_READ_TAIL     (OS_QUEUE_READ | (OS_QUEUE_TAIL << 1))
#define OS_QUEUE_WRITE_HEAD    (OS_QUEUE_WRITE | (OS_QUEUE_HEAD << 1))
#define OS_QUEUE_WRITE_TAIL    (OS_QUEUE_WRITE | (OS_QUEUE_TAIL << 1))
#define OS_QUEUE_OPERATE_GET(type) ((type) & (0x03))
#define OS_QUEUE_IS_POINT(type)    ((type) & (0x04))
#define OS_QUEUE_IS_READ(type)     (OS_QUEUE_READ_WRITE_GET(type) == OS_QUEUE_READ)
#define OS_QUEUE_IS_WRITE(type)    (OS_QUEUE_READ_WRITE_GET(type) == OS_QUEUE_WRITE)
#define OS_READWRITE_LEN           2

/**
  * @ingroup los_queue
  * Queue information block structure
  */
typedef struct {
    UINT8 *queue;      /**< Pointer to a queue handle */
    UINT16 queueState; /**< Queue state */
    UINT16 queueLen;   /**< Queue length */
    UINT16 queueSize;  /**< Node size */
    UINT16 queueID;    /**< queueID */
    UINT16 queueHead;  /**< Node head */
    UINT16 queueTail;  /**< Node tail */
    UINT16 readWriteableCnt[OS_READWRITE_LEN]; /**< Count of readable or writable resources, 0:readable, 1:writable */
    LOS_DL_LIST readWriteList[OS_READWRITE_LEN]; /**< Pointer to the linked list to be read or written,
                                                      0:readlist, 1:writelist */
    LOS_DL_LIST memList; /**< Pointer to the memory linked list */
} LosQueueCB;

/* queue state */
/**
  *  @ingroup los_queue
  *  Message queue state: not in use.
  */
#define OS_QUEUE_UNUSED        0

/**
  *  @ingroup los_queue
  *  Message queue state: used.
  */
#define OS_QUEUE_INUSED        1

/**
  *  @ingroup los_queue
  *  Not in use.
  */
#define OS_QUEUE_WAIT_FOR_POOL 1

/**
  *  @ingroup los_queue
  *  Normal message queue.
  */
#define OS_QUEUE_NORMAL        0

/**
  *  @ingroup los_queue
  *  Queue information control block
  */
extern LosQueueCB *g_allQueue;

/**
  *  @ingroup los_queue
  *  Obtain a handle of the queue that has a specified ID.
  */
#define GET_QUEUE_HANDLE(QueueID) (((LosQueueCB *)g_allQueue) + (QueueID))

/**
  *  @ingroup los_queue
  * Obtain the head node in a queue doubly linked list.
  */
#define GET_QUEUE_LIST(ptr) LOS_DL_LIST_ENTRY(ptr, LosQueueCB, readWriteList[OS_QUEUE_WRITE])

/**
 * @ingroup los_queue
 * @brief Alloc a stationary memory for a mail.
 *
 * @par Description:
 * This API is used to alloc a stationary memory for a mail according to queueID.
 * @attention
 * <ul>
 * <li>Do not alloc memory in unblocking modes such as interrupt.</li>
 * <li>This API cannot be called before the Huawei LiteOS is initialized.</li>
 * <li>The argument timeOut is a relative time.</li>
 * </ul>
 *
 * @param queueID        [IN]        Queue ID. The value range is [1,LOSCFG_BASE_IPC_QUEUE_LIMIT].
 * @param mailPool       [IN]        The memory poll that stores the mail.
 * @param timeOut        [IN]        Expiry time. The value range is [0,LOS_WAIT_FOREVER].
 *
 * @retval   #NULL                    The memory allocation is failed.
 * @retval   #mem                     The address of alloc memory.
 * @par Dependency:
 * <ul><li>los_queue_pri.h: the header file that contains the API declaration.</li></ul>
 * @see OsQueueMailFree
 */
extern VOID *OsQueueMailAlloc(UINT32 queueID, VOID *mailPool, UINT32 timeOut);

/**
 * @ingroup los_queue
 * @brief Free a stationary memory of a mail.
 *
 * @par Description:
 * This API is used to free a stationary memory for a mail according to queueID.
 * @attention
 * <ul>
 * <li>This API cannot be called before the Huawei LiteOS is initialized.</li>
 * </ul>
 *
 * @param queueID         [IN]        Queue ID. The value range is [1,LOSCFG_BASE_IPC_QUEUE_LIMIT].
 * @param mailPool        [IN]        The mail memory poll address.
 * @param mailMem         [IN]        The mail memory block address.
 *
 * @retval   #LOS_OK                                 0x00000000: The memory free successfully.
 * @retval   #OS_ERRNO_QUEUE_MAIL_HANDLE_INVALID     0x02000619: The handle of the queue passed-in when the memory for
                                                                 the queue is being freed is invalid.
 * @retval   #OS_ERRNO_QUEUE_MAIL_PTR_INVALID        0x0200061a: The pointer to the memory to be freed is null.
 * @retval   #OS_ERRNO_QUEUE_MAIL_FREE_ERROR         0x0200061b: The memory for the queue fails to be freed.
 * @par Dependency:
 * <ul><li>los_queue_pri.h: the header file that contains the API declaration.</li></ul>
 * @see OsQueueMailAlloc
 */
extern UINT32 OsQueueMailFree(UINT32 queueID, VOID *mailPool, VOID *mailMem);

/**
 * @ingroup los_queue
 * @brief Initialization queue.
 *
 * @par Description:
 * This API is used to initialization queue.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param None.
 *
 * @retval   UINT32  Initialization result.
 * @par Dependency:
 * <ul><li>los_queue_pri.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 OsQueueInit(VOID);

/**
 * @ingroup los_queue
 * @brief Handle when read or write queue.
 *
 * @par Description:
 * This API is used to handle when read or write queue.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param queueID        [IN]       Queue id.
 * @param operateType    [IN]       Operate type
 * @param bufferAddr     [IN]       Buffer address.
 * @param bufferSize     [IN]       Buffer size.
 * @param timeOut        [IN]       Timeout.
 *
 * @retval   UINT32  Handle result.
 * @par Dependency:
 * <ul><li>los_queue_pri.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 OsQueueOperate(UINT32 queueID, UINT32 operateType, VOID *bufferAddr, UINT32 *bufferSize,
                             UINT32 timeOut);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_QUEUE_PRI_H */
