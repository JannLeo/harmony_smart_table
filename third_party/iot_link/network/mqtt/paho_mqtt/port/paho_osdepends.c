/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - convert to FreeRTOS
 *******************************************************************************/

#include "paho_osdepends.h"


int ThreadStart(Thread* thread, void (*fn)(void*), void* arg)
{
	int rc = 0;
	thread = thread;

	osThreadAttr_t attr;

    attr.name = "MQTTTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = osThreadGetPriority(osThreadGetId());

    rc = (int)osThreadNew((osThreadFunc_t)fn, arg, &attr);

	return rc;
}

static unsigned int g_tickPerSec = 0;
static unsigned int g_sysPerSec = 0;
static unsigned int g_sysPerTick = 0;

static int GetCurrentTime(struct timeval* now)
{
    if (now == NULL) {
        return -1;
    }

    unsigned int tickCount = osKernelGetTickCount();
    now->tv_sec = tickCount / g_tickPerSec;
    now->tv_usec = (tickCount % g_tickPerSec) * (1000*1000 / g_tickPerSec);

    unsigned int sysCount = osKernelGetSysTimerCount() % g_sysPerTick;
    now->tv_usec += sysCount * 1000*1000 / g_sysPerSec;
    return 0;
}

#define gettimeofday(tv, tz) GetCurrentTime(tv)

void TimerInit(Timer* timer)
{
	timer->end_time = (struct timeval){0, 0};
	if (g_tickPerSec == 0) {
        g_tickPerSec = osKernelGetTickFreq();
        g_sysPerSec = osKernelGetSysTimerFreq();
        g_sysPerTick = g_sysPerSec / g_tickPerSec;
    }
}

char TimerIsExpired(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	timeradd(&now, &interval, &timer->end_time);
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout, 0};
	timeradd(&now, &interval, &timer->end_time);
}


int TimerLeftMS(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}




int MutexInit(Mutex* mutex)
{
	mutex->mutex = osMutexNew(NULL);;
	if(mutex->mutex== NULL)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int MutexLock(Mutex* mutex)
{
	return osMutexAcquire(mutex->mutex, osWaitForever);
}

int MutexUnlock(Mutex* mutex)
{
	return osMutexRelease(mutex->mutex);
}

int MutexDestory(Mutex*mutex)
{
	return osMutexDelete(mutex->mutex);
}


