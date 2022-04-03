/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "stdlib.h"
#include "string.h"
#include "los_vm_map.h"

/*
 * Allocates the requested memory and returns a pointer to it. The requested
 * size is nitems each size bytes long (total memory requested is nitems*size).
 * The space is initialized to all zero bits.
 */

void* calloc(size_t nitems, size_t size) { /*lint !e578*/
  size_t real_size;
  void* ptr = NULL;

  if (nitems == 0 || size == 0) {
    return NULL;
  }

  real_size = (size_t)(nitems * size);
  ptr = LOS_KernelMalloc((UINT32)real_size);
  if (ptr != NULL) {
    (void)memset_s((void *)ptr, real_size, 0, real_size);
  }
  return ptr;
}

/*
 * Deallocates the memory previously allocated by a call to calloc, malloc, or
 * realloc. The argument ptr points to the space that was previously allocated.
 * If ptr points to a memory block that was not allocated with calloc, malloc,
 * or realloc, or is a space that has been deallocated, then the result is undefined.
 */

void free(void* ptr) {
  if (ptr == NULL) {
    return;
  }

  LOS_KernelFree(ptr);
}

/*
 * Allocates the requested memory and returns a pointer to it. The requested
 * size is size bytes. The value of the space is indeterminate.
 */

void* malloc(size_t size) { /*lint !e31 !e10*/
  if (size == 0) {
    return NULL;
  }

  return LOS_KernelMalloc((UINT32)size);
}

void* zalloc(size_t size) { /*lint !e10*/
  void* ptr = NULL;

  if (size == 0) {
    return NULL;
  }

  ptr = LOS_KernelMalloc((UINT32)size);
  if (ptr != NULL) {
    (void)memset_s(ptr, size, 0, size);
  }
  return ptr;
}

/*
 * allocates a block of size bytes whose address is a multiple of boundary.
 * The boundary must be a power of two!
 */

void* memalign(size_t boundary, size_t size) {
  if (size == 0) {
    return NULL;
  }

  return LOS_KernelMallocAlign((UINT32)size, (UINT32)boundary);
}

/*
 * Attempts to resize the memory block pointed to by ptr that was previously
 * allocated with a call to malloc or calloc. The contents pointed to by ptr are
 * unchanged. If the value of size is greater than the previous size of the
 * block, then the additional bytes have an undeterminate value. If the value
 * of size is less than the previous size of the block, then the difference of
 * bytes at the end of the block are freed. If ptr is null, then it behaves like
 * malloc. If ptr points to a memory block that was not allocated with calloc
 * or malloc, or is a space that has been deallocated, then the result is
 * undefined. If the new space cannot be allocated, then the contents pointed
 * to by ptr are unchanged. If size is zero, then the memory block is completely
 * freed.
 */

void* realloc(void* ptr, size_t size) {
  if (ptr == NULL) {
    ptr = malloc(size);
    return ptr;
  }

  if (size == 0) {
    free(ptr);
    return NULL;
  }

  return LOS_KernelRealloc(ptr, (UINT32)size);
}