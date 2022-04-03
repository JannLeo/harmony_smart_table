/*
 * Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
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

#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

#include <hi_types_base.h>

typedef struct {
    hi_u16 gpio6_cfg;
    hi_u16 gpio8_cfg;
    hi_u16 gpio10_cfg;
    hi_u16 gpio11_cfg;
    hi_u16 gpio12_cfg;
    hi_u16 gpio13_cfg;
    hi_u16 sfc_csn_cfg;
} app_iocfg_backup;

#endif // __APP_MAIN_H__
