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

#ifndef LITE_PUB_SUB_IMPLEMENT_H
#define LITE_PUB_SUB_IMPLEMENT_H

#include "broadcast_interface.h"
#include "pub_sub_feature.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

enum MessageID {
    MSG_PUBLISH = 1,
    MSG_BUTT
};

typedef struct PubSubImplement {
    INHERIT_IUNKNOWNENTRY(PubSubInterface);
    PubSubFeature *feature;
} PubSubImplement;

PubSubImplement *BCE_CreateInstance(Feature *feature);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_PUB_SUB_IMPLEMENT_H
