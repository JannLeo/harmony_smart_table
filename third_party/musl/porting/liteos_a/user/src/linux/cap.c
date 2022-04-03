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

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "syscall.h"
#include "sys/capability.h"

static unsigned int __linux_caps_maps[] = {
	CAP_CHOWN, CAP_DAC_OVERRIDE, CAP_DAC_READ_SEARCH, CAP_FOWNER,
	CAP_KILL, CAP_SETGID, CAP_SETUID, CAP_SETPCAP, CAP_NET_BIND_SERVICE,
	CAP_NET_BROADCAST, CAP_NET_ADMIN, CAP_NET_RAW, CAP_SYS_PTRACE, CAP_SYS_ADMIN,
	CAP_SYS_NICE, CAP_SYS_TIME, CAP_SYS_BOOT};

static unsigned int __ohos_caps_maps[] = {
	OHOS_CAP_CHOWN, OHOS_CAP_DAC_EXECUTE, OHOS_CAP_DAC_WRITE, OHOS_CAP_DAC_READ_SEARCH, OHOS_CAP_FOWNER, OHOS_CAP_KILL,
	OHOS_CAP_SETGID, OHOS_CAP_SETUID, OHOS_CAP_NET_BIND_SERVICE, OHOS_CAP_NET_BROADCAST, OHOS_CAP_NET_ADMIN,
	OHOS_CAP_NET_RAW, OHOS_CAP_FS_MOUNT, OHOS_CAP_FS_FORMAT, OHOS_CAP_SCHED_SETPRIORITY,
	OHOS_CAP_SET_TIMEOFDAY, OHOS_CAP_CLOCK_SETTIME, OHOS_CAP_CAPSET, OHOS_CAP_REBOOT, OHOS_CAP_SHELL_EXEC};

static unsigned int linux_caps_to_ohos(unsigned int caps)
{
	int num = sizeof(__linux_caps_maps) / sizeof(int);
	int loop;
	unsigned int result = 0;
	for (loop = 0; loop < num; loop++) {
		if (!((1 << __linux_caps_maps[loop]) & caps)) {
			continue;
		}

		switch (__linux_caps_maps[loop]) {
			case CAP_CHOWN:
				result |= 1 << OHOS_CAP_CHOWN;
				break;
			case CAP_DAC_OVERRIDE:
				result |= 1 << OHOS_CAP_DAC_EXECUTE;
				result |= 1 << OHOS_CAP_DAC_WRITE;
				result |= 1 << OHOS_CAP_DAC_READ_SEARCH;
				break;
			case CAP_DAC_READ_SEARCH:
				result |= 1 << OHOS_CAP_DAC_READ_SEARCH;
				break;
			case CAP_FOWNER:
				result |= 1 << OHOS_CAP_FOWNER;
				break;
			case CAP_KILL:
				result |= 1 << OHOS_CAP_KILL;
				break;
			case CAP_SETGID:
				result |= 1 << OHOS_CAP_SETGID;
				break;
			case CAP_SETUID:
				result |= 1 << OHOS_CAP_SETUID;
				break;
			case CAP_SETPCAP:
				result |= 1 << OHOS_CAP_CAPSET;
				break;
			case CAP_NET_BIND_SERVICE:
				result |= 1 << OHOS_CAP_NET_BIND_SERVICE;
				break;
			case CAP_NET_BROADCAST:
				result |= 1 << OHOS_CAP_NET_BROADCAST;
				break;
			case CAP_NET_ADMIN:
				result |= 1 << OHOS_CAP_NET_ADMIN;
				break;
			case CAP_NET_RAW:
				result |= 1 << OHOS_CAP_NET_RAW;
				break;
			case CAP_SYS_PTRACE:
				result |= 1 << OHOS_CAP_SHELL_EXEC;
				break;
			case CAP_SYS_ADMIN:
				result |= 1 << OHOS_CAP_FS_MOUNT;
				result |= 1 << OHOS_CAP_FS_FORMAT;
				break;
			case CAP_SYS_NICE:
				result |= 1 << OHOS_CAP_SCHED_SETPRIORITY;
				break;
			case CAP_SYS_TIME:
				result |= 1 << OHOS_CAP_SET_TIMEOFDAY;
				result |= 1 << OHOS_CAP_CLOCK_SETTIME;
				break;
			case CAP_SYS_BOOT:
				result |= 1 << OHOS_CAP_REBOOT;
				break;
			default:
				break;
		}
	}

	return result;
}

static unsigned int ohos_caps_to_linux(unsigned int caps)
{
	int num = sizeof(__ohos_caps_maps) / sizeof(int);
	int loop;
	unsigned int result = 0;
	for (loop = 0; loop < num; loop++) {
		if (!((1 << __ohos_caps_maps[loop]) & caps)) {
			continue;
		}

		switch (__ohos_caps_maps[loop]) {
			case OHOS_CAP_CHOWN:
				result |= 1 << CAP_CHOWN;
				break;
			case OHOS_CAP_DAC_EXECUTE:
			case OHOS_CAP_DAC_WRITE:
				result |= 1 << CAP_DAC_OVERRIDE;
				break;
			case OHOS_CAP_DAC_READ_SEARCH:
				result |= 1 << CAP_DAC_READ_SEARCH;
				break;
			case OHOS_CAP_FOWNER:
				result |= 1 << CAP_FOWNER;
				break;
			case OHOS_CAP_KILL:
				result |= 1 << CAP_KILL;
				break;
			case OHOS_CAP_SETGID:
				result |= 1 << CAP_SETGID;
				break;
			case OHOS_CAP_SETUID:
				result |= 1 << CAP_SETUID;
				break;
			case OHOS_CAP_CAPSET:
				result |= 1 << CAP_SETPCAP;
				break;
			case OHOS_CAP_NET_BIND_SERVICE:
				result |= 1 << CAP_NET_BIND_SERVICE;
				break;
			case OHOS_CAP_NET_BROADCAST:
				result |= 1 << CAP_NET_BROADCAST;
				break;
			case OHOS_CAP_NET_ADMIN:
				result |= 1 << CAP_NET_ADMIN;
				break;
			case OHOS_CAP_NET_RAW:
				result |= 1 << CAP_NET_RAW;
				break;
			case OHOS_CAP_SHELL_EXEC:
				result |= 1 << CAP_SYS_PTRACE;
				break;
			case OHOS_CAP_FS_MOUNT:
			case OHOS_CAP_FS_FORMAT:
				result |= 1 << CAP_SYS_ADMIN;
				break;
			case OHOS_CAP_SCHED_SETPRIORITY:
				result |= 1 << CAP_SYS_NICE;
				break;
			case OHOS_CAP_SET_TIMEOFDAY:
			case OHOS_CAP_CLOCK_SETTIME:
				result |= 1 << CAP_SYS_TIME;
				break;
			case OHOS_CAP_REBOOT:
				result |= 1 << CAP_SYS_BOOT;
				break;
			default:
				break;
		}
	}

	return result;
}

int linux_capget(cap_user_header_t hdr_ptr, cap_user_data_t data_ptr)
{
	unsigned int capvalue = 0;

	if (hdr_ptr == NULL || data_ptr == NULL) {
		errno = EINVAL;
		return -1;
	}

	switch (hdr_ptr->version) {
		case _LINUX_CAPABILITY_VERSION_1:
		case _LINUX_CAPABILITY_VERSION_2:
		case _LINUX_CAPABILITY_VERSION_3:
			break;
		default:
			errno = EINVAL;
			return -1;
	}

	if (syscall(SYS_ohoscapget, hdr_ptr->pid, &capvalue)) {
		return -1;
	}

	data_ptr[0].effective = ohos_caps_to_linux(capvalue);
	data_ptr[0].permitted = ohos_caps_to_linux(capvalue);
	data_ptr[0].inheritable = ohos_caps_to_linux(capvalue);
	return 0;
}

int linux_capset(cap_user_header_t hdr_ptr, const cap_user_data_t data_ptr)
{
	unsigned int capvalue = 0;

	if (hdr_ptr == NULL || data_ptr == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (hdr_ptr->pid) {
		errno = EPERM;
		return -1;
	}

	switch (hdr_ptr->version) {
		case _LINUX_CAPABILITY_VERSION_1:
		case _LINUX_CAPABILITY_VERSION_2:
		case _LINUX_CAPABILITY_VERSION_3:
			break;
		default:
			errno = EINVAL;
			return -1;
	}

	capvalue = linux_caps_to_ohos(data_ptr[0].effective);
	return syscall(SYS_ohoscapset, capvalue);
}

weak_alias(linux_capget, capget);
weak_alias(linux_capset, capset);

int ohos_capget(pid_t pid, unsigned int *caps)
{
	return syscall(SYS_ohoscapget, pid, caps);
}

int ohos_capset(unsigned int caps)
{
	return syscall(SYS_ohoscapset, caps);
}
