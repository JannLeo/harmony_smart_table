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

/**
 * @defgroup fat Fat
 * @ingroup filesystem
 */

#ifndef _ERRCODE_FAT_H
#define _ERRCODE_FAT_H

#ifdef LOSCFG_FS_FAT_VIRTUAL_PARTITION
#define VIRERR_OK			0x00000000
#define VIRERR_BASE			0x10000000
#define VIRERR_MODIFIED			0x10000001
#define VIRERR_CHAIN_ERR		0x10000002
#define VIRERR_OCCUPIED			0x10000003
#define VIRERR_NOTCLEAR			0x10000004
#define VIRERR_NOTFIT			0x10000005
#define VIRERR_NOTMOUNT			0x10000006
#define VIRERR_INTER_ERR		0x10000007
#define VIRERR_NOPARAM			0x10000008
#define VIRERR_PARMLOCKED		0x10000009
#define VIRERR_PARMNUMERR		0x1000000A
#define VIRERR_PARMPERCENTERR		0x1000000B
#define VIRERR_PARMNAMEERR		0x1000000C
#define VIRERR_PARMDEVERR		0x1000000D
#endif
#endif
