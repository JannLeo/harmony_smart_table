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

#ifndef _LOS_MEMBOX_PRI_H
#define _LOS_MEMBOX_PRI_H

#include "los_membox.h"

#define OS_MEMBOX_NEXT(addr, blkSize) (LOS_MEMBOX_NODE *)((UINT8 *)(addr) + (blkSize))

#ifdef LOS_MEMBOX_CHECK
#define OS_MEMBOX_MAGIC 0xa55a5aa5
#define OS_MEMBOX_SET_MAGIC(addr) (*((UINT32 *)(addr)) = OS_MEMBOX_MAGIC)
#define OS_MEMBOX_CHECK_MAGIC(addr) ((*((UINT32 *)(addr)) == OS_MEMBOX_MAGIC) ? LOS_OK : LOS_NOK)
#else
#define OS_MEMBOX_SET_MAGIC(addr)
#define OS_MEMBOX_CHECK_MAGIC(addr) LOS_OK
#endif

#define OS_MEMBOX_USER_ADDR(addr) ((VOID *)((UINT8 *)(addr) + LOS_MEMBOX_MAGIC_SIZE))
#define OS_MEMBOX_NODE_ADDR(addr) ((LOS_MEMBOX_NODE *)((UINT8 *)(addr) - LOS_MEMBOX_MAGIC_SIZE))


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_MEMBOX_PRI_H */
