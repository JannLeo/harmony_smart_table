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

#ifndef __LINUX_TZDST_H__
#define __LINUX_TZDST_H__

#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __LITEOS__

#define SECSPERMIN  60
#define MINSPERHOUR 60
#define HOURSPERDAY 24
#define DAYSPERWEEK 7
#define DAYSPERNYEAR    365
#define DAYSPERLYEAR    366
#define SECSPERHOUR (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY  ((int_fast32_t) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR 12

#define TM_YEAR_BASE    1900

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

/**
 * @ingroup localtime
 * @brief Set Time Zone.
 *
 * @par Description:
 * This API is used to set the time zone.
 *
 * @attention
 * <ul>
 * <li>When setting time zones, format is required to be "tzn[+/-]hh[:mm[:ss]][dzn]" .</li>
 * </ul>
 *
 * @param buff The string point to the time zone going to be setting.
 *
 * @retval   None
 */
void settimezone(const char *);

/**
 * @ingroup localtime
 * @brief Disable daylight saving time.
 *
 * @par Description:
 * This API is used to make daylight saving time useless.
 *
 * @param None.
 *
 * @retval   None
 */
int dst_disable(void);

/**
 * @ingroup localtime
 * @brief Enable daylight saving time..
 *
 * @par Description:
 * This API is used to configure daylight saving time.
 *
 * @attention
 * <ul>
 * <li>When config dst, The configuration format has strict requirements.</li>
 * <li>The first configuration format for example "Feb-03 03:00:00"</li>
 * <li>The second configuration format for example "Oct-1st-Fri 03:00:00"</li>
 * <li>The abbreviations for the months are "Jan",  "Feb", "Mar",  "Apr", "May", "Jun", "Jul", "Aug",
 * "Sep", "Oct", "Nov", and "Dec".</li>
 * <li>The abbreviations for the days of the week are "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", and "Sat".</li>
 * <li>The abbreviations for the weeks of the month are "1st", "2nd", "3rd", "4th", "5th"</li>
 * <li>When it's only four weekdays this month, the effect of "5th" is the same as "Last"</li>
 * </ul>
 *
 * @param dstStartTime   The string point to the DST start going to be setting.
 * @param dstEndTime     The string point to the DST end going to be setting.
 * @param forwardSeconds Indicates the number of seconds that time is allocated when entering DST interval.
 *
 * @retval   0 Succeed.
 * @retval  -1 Failed.
 */
int dst_enable(const char *dstStartTime, const char *dstEndTime, int forwardSeconds);

/**
 * @ingroup localtime
 * @brief Inquire daylight saving time.
 *
 * @par Description:
 * This API is used to inquire daylight saving time.
 *
 * @param year      Represents the year to query start from 1900.
 * @param dstStart  Used to store daylight savings time start time
 * @param dstEnd    Used to store daylight savings time end time
 *
 * @retval   0 Succeed.
 * @retval  -1 Failed.
 */
int dst_inquire(int year, struct tm *dstStart, struct tm *dstEnd);

#endif /* __LITEOS__ */
#ifdef __cplusplus
}
#endif

#endif /* __LINUX_TZDST_H__ */