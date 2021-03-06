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
#include "samgr_maintenance.h"
#include <log.h>
#include "service_impl.h"
#include "samgr_lite_inner.h"
#include "time_adapter.h"

void SAMGR_PrintServices(void)
{
    SamgrLiteImpl *samgr = GET_OBJECT(SAMGR_GetInstance(), SamgrLiteImpl, vtbl);
    int16 num = VECTOR_Num(&samgr->services);
    HILOG_INFO(HILOG_MODULE_SAMGR, "[Maintenance]Print Services(%d) Information:", num);
    if (num == 0) {
        return;
    }

    MUTEX_Lock(samgr->mutex);
    int16 size = VECTOR_Size(&(samgr->services));
    int i;
    for (i = 0; i < size; ++i) {
        ServiceImpl *impl = VECTOR_At(&(samgr->services), i);
        if (impl == NULL) {
            continue;
        }

        MUTEX_Unlock(samgr->mutex);
        const char *name = impl->service->GetName(impl->service);
        HILOG_INFO(HILOG_MODULE_SAMGR, "[Service:%d]<status:%d, name:%s, default:%p, features:%d, task:%p>",
                   impl->serviceId, impl->inited, name, impl->defaultApi, VECTOR_Num(&impl->features), impl->taskPool);

        TaskPool *pool = impl->taskPool;
        if (pool != NULL) {
            HILOG_INFO(HILOG_MODULE_SAMGR, "[TaskPool:%p]<tid:%p, qid:%p, ref:%d, pri:%d, stack:0x%x>",
                       pool, pool->tasks[0], pool->queueId, pool->ref, pool->priority, pool->stackSize);
        }

        int16 featureSize = VECTOR_Size(&impl->features);
        int j;
        for (j = 0; j < featureSize; ++j) {
            FeatureImpl *feature = VECTOR_At(&impl->features, j);
            if (feature == NULL) {
                continue;
            }
            name = feature->feature->GetName(feature->feature);
            HILOG_INFO(HILOG_MODULE_SAMGR, "[Feature:%d]<name:%s, api:%p>", j, name, feature->iUnknown);
        }
        MUTEX_Lock(samgr->mutex);
    }
    MUTEX_Unlock(samgr->mutex);
}

void SAMGR_PrintOperations(void)
{
    SamgrLiteImpl *samgr = GET_OBJECT(SAMGR_GetInstance(), SamgrLiteImpl, vtbl);
    int16 num = VECTOR_Num(&samgr->services);
    HILOG_INFO(HILOG_MODULE_SAMGR, "[Maintenance]Print Services(%d) Statistics:", num);
    if (num == 0) {
        return;
    }

    uint32 now = SAMGR_GetProcessTime();
    MUTEX_Lock(samgr->mutex);
    int16 size = VECTOR_Size(&(samgr->services));
    int i;
    for (i = 0; i < size; ++i) {
        ServiceImpl *impl = VECTOR_At(&(samgr->services), i);
        if (impl == NULL) {
            continue;
        }
        MUTEX_Unlock(samgr->mutex);
        const char *name = impl->service->GetName(impl->service);
        uint8 abnormal = impl->ops.abnormal;
        if (impl->inited == SVC_BUSY) {
            if (GET_INTERVAL(impl->ops.timestamp, now) > MSG_PROC_THRESHOLD) {
                abnormal = abnormal + 1;
            }
        }

        MQueueId queueId = ((impl->taskPool == NULL) ? NULL : impl->taskPool->queueId);
        HILOG_INFO(HILOG_MODULE_SAMGR, "[Qid:%p] <status:%d, name:%s, abnormal:%d, total:%d, time:%ums>",
                   queueId, impl->inited, name, abnormal, impl->ops.messages, impl->ops.timestamp);
        MUTEX_Lock(samgr->mutex);
    }
    MUTEX_Unlock(samgr->mutex);
}