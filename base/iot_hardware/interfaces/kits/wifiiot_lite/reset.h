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

/**
 * @addtogroup power
 * @{
 *
 * @brief Provides device power management functions.
 *
 * This module is used to reboot the device and set low power consumption for the device. \n
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file reset.h
 *
 * @brief Reboots the device.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef RESET_H
#define RESET_H

/**
 * @brief Enumerates reboot causes.
 */
typedef enum {
    /** Unknown cause */
    SYS_REBOOT_CAUSE_UNKNOWN = 0,
    /** System reboot */
    SYS_REBOOT_CAUSE_CMD,
    /** Reboot upon an upgrade */
    SYS_REBOOT_CAUSE_UPG,
    /** Reboot upon an upgrade on the backup partition */
    SYS_REBOOT_CAUSE_UPG_B,
    /** Reboot in Wi-Fi mode */
    SYS_REBOOT_CAUSE_WIFI_MODE,
    /** Normal reboot */
    SYS_REBOOT_CAUSE_USR_NORMAL_REBOOT,
    /** Custom cause 0 */
    SYS_REBOOT_CAUSE_USR0,
    /** Custom cause 1 */
    SYS_REBOOT_CAUSE_USR1,
    /** Reboot upon busy AT commands */
    SYS_REBOOT_CAUSE_AT_BUSY,
    /** Maximum value */
    SYS_REBOOT_CAUSE_MAX,
}RebootCause;

/**
 * @brief Reboots the device using different causes.
 *
 * @param cause Indicates the reboot cause.
 * @since 1.0
 * @version 1.0
 */
void RebootDevice(RebootCause cause);

#endif
