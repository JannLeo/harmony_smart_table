/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hi_time.h"
#include "hi_task.h"
#include "hal_wifiiot_kal.h"

BSP_RAM_TEXT_SECTION void HalKalTickRegisterCallback(HalTickIdleKalCallback cb)
{
    hi_tick_register_callback(cb);
}

void HalKalThreadRegisterIdleCallback(HalTickIdleKalCallback cb)
{
    hi_task_register_idle_callback(cb);
}

