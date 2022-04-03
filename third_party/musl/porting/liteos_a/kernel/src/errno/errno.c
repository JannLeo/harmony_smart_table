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

#include "errno.h"
#include "los_errno.h"
#include "los_task_pri.h"

/* the specific errno get or set in interrupt service routine */
static int errno_isr;

void set_errno(int err_code) {
  LosTaskCB *runTask = NULL;

  /* errno can not be set to 0 as posix standard */
  if (err_code == 0)
    return;

  if (OS_INT_INACTIVE) {
    runTask = OsCurrTaskGet();
    runTask->errorNo = err_code;
  }
  else {
    errno_isr = err_code;
  }
}

int get_errno(void) {
  LosTaskCB *runTask = NULL;

  if (OS_INT_INACTIVE) {
    runTask = OsCurrTaskGet();
    return runTask->errorNo;
  }
  else {
    return errno_isr;
  }
}

int *__errno_location(void) {
  LosTaskCB *runTask = NULL;

  if (OS_INT_INACTIVE) {
    runTask = OsCurrTaskGet();
    return &runTask->errorNo;
  }
  else {
    return &errno_isr;
  }
}

volatile int *__errno(void) {
  LosTaskCB *runTask = NULL;

  if (OS_INT_INACTIVE) {
    runTask = OsCurrTaskGet();
    return (volatile int *)(&runTask->errorNo);
  }
  else {
    return (volatile int *)(&errno_isr);
  }
}
