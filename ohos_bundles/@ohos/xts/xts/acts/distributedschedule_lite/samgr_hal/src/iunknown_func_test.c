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

#include <los_base.h>
#include <hos_errno.h>
#include "hctest.h"
#include "samgr_lite.h"

#define SERVICE_NAME "S_iunknown"
#define FEATURE_NAME "F_iunknown"

typedef struct DemoApi {
    INHERIT_IUNKNOWN;
} DemoApi;

typedef struct DemoFeature {
    INHERIT_FEATURE;
    INHERIT_IUNKNOWNENTRY(DemoApi);
    Identity identity;
} DemoFeature;

static const char *GetName(Service *service)
{
    (void)service;
    return SERVICE_NAME;
}

typedef struct ExampleService {
    INHERIT_SERVICE;
    Identity identity;
} ExampleService;

static BOOL Initialize(Service *service, Identity identity)
{
    ExampleService *example = (ExampleService *)service;
    example->identity = identity;
    return TRUE;
}

static BOOL MessageHandle(Service *service, Request *msg)
{
    (void)service;
    (void)msg;
    return FALSE;
}

static TaskConfig GetTaskConfig(Service *service)
{
    (void)service;
    TaskConfig config = {
        LEVEL_HIGH, 
        PRI_ABOVE_NORMAL,
        0x800, 
        20, 
        SHARED_TASK
        };
    return config;
}

static ExampleService g_service = {
    .GetName = GetName,
    .Initialize = Initialize,
    .MessageHandle = MessageHandle,
    .GetTaskConfig = GetTaskConfig,
    .identity = {-1, -1, NULL},
};

static const char *FEATURE_GetName(Feature *feature)
{
    (void)feature;
    return FEATURE_NAME;
}
static void FEATURE_OnInitialize(Feature *feature, Service *parent, Identity identity)
{
    (void)parent;
    DemoFeature *demoFeature = (DemoFeature *)feature;
    demoFeature->identity = identity;
}
static void FEATURE_OnStop(Feature *feature, Identity identity)
{
    (void)feature;
    (void)identity;
}

static BOOL FEATURE_OnMessage(Feature *feature, Request *request)
{
    (void)feature;
    (void)request;
    return TRUE;
}

static DemoFeature g_feature = {
    .GetName = FEATURE_GetName,
    .OnInitialize = FEATURE_OnInitialize,
    .OnStop = FEATURE_OnStop,
    .OnMessage = FEATURE_OnMessage,
    .ver = DEFAULT_VERSION,
    .ref = 1,
    .iUnknown = {
        DEFAULT_IUNKNOWN_IMPL,
    },
    .identity = {-1, -1, NULL},
};

static DemoApi g_api = {
    .AddRef = IUNKNOWN_AddRef,
    .QueryInterface = IUNKNOWN_QueryInterface,
    .Release = IUNKNOWN_Release,
};

LITE_TEST_SUIT(test, samgr, IUnknownTestSuite);

static BOOL IUnknownTestSuiteSetUp(void)
{
    BOOL result1 = SAMGR_GetInstance()->RegisterService((Service *)&g_service);
    BOOL result2 = SAMGR_GetInstance()->RegisterFeature(SERVICE_NAME, (Feature *)&g_feature);
    if (result1 == TRUE && result2 == TRUE) {
        return TRUE;
    } else {
        printf("[hctest]E failed to register service or feature \n");
        return FALSE;
    }
}

static BOOL IUnknownTestSuiteTearDown(void)
{
    SAMGR_GetInstance()->UnregisterFeature(SERVICE_NAME, FEATURE_NAME);
    SAMGR_GetInstance()->UnregisterService(SERVICE_NAME);
    return TRUE;
}

/**
 * @tc.number    : DMSLite_SAMGR_GetIUnknown_0010
 * @tc.name      : Use this macro GET_IUNKNOWN user can obtain the IUnknown interface from the subclass object.
 * @tc.desc      : [C- SOFTWARE -0200]
 * @tc.size      : MEDIUM
 * @tc.type      : FUNC
 * @tc.level     : Level 2
 */
LITE_TEST_CASE(IUnknownTestSuite, testGetIUnknown0010, LEVEL2)
{
    IUnknown *iUnknown = GET_IUNKNOWN(g_feature);
    TEST_ASSERT_EQUAL_INT(TRUE, iUnknown != NULL);
}

/**
 * @tc.number    : DMSLite_SAMGR_GetObject_0010
 * @tc.name      : Use this macro GET_OBJECT user can obtain a outside object.
 * @tc.desc      : [C- SOFTWARE -0200]
 * @tc.size      : MEDIUM
 * @tc.type      : FUNC
 * @tc.level     : Level 2
 */
LITE_TEST_CASE(IUnknownTestSuite, testGetObject0010, LEVEL2)
{
    IUnknown *iUnknown = GET_IUNKNOWN(g_feature);
    IUnknownEntry *entry = GET_OBJECT(iUnknown, IUnknownEntry, iUnknown);
    TEST_ASSERT_EQUAL_INT(DEFAULT_VERSION, entry->ver);
    TEST_ASSERT_EQUAL_INT(1, entry->ref);
}

/**
 * @tc.number    : DMSLite_SAMGR_QueryInterface_0010
 * @tc.name      : Use this api QueryInterface user can convert the object to the required subclass type.
 * @tc.desc      : [C- SOFTWARE -0200]
 * @tc.size      : MEDIUM
 * @tc.type      : FUNC
 * @tc.level     : Level 2
 */
LITE_TEST_CASE(IUnknownTestSuite, testQueryInterface0010, LEVEL2)
{
    SAMGR_GetInstance()->RegisterFeatureApi(SERVICE_NAME, FEATURE_NAME, GET_IUNKNOWN(g_feature));

    IUnknown *iUnknown = SAMGR_GetInstance()->GetFeatureApi(SERVICE_NAME, FEATURE_NAME);
    DemoApi *demoApi = NULL;
    int resultQuery = iUnknown->QueryInterface(iUnknown, DEFAULT_VERSION, (void **)&demoApi);
    TEST_ASSERT_EQUAL_INT(EC_SUCCESS, resultQuery);
    TEST_ASSERT_EQUAL_INT(TRUE, demoApi != NULL);
    
    int resultAdd = iUnknown->AddRef(iUnknown);
    int resultRelease = iUnknown->Release(iUnknown);
    TEST_ASSERT_EQUAL_INT(1, resultAdd - resultRelease);

    SAMGR_GetInstance()->UnregisterFeatureApi(SERVICE_NAME, FEATURE_NAME);
}

/**
 * @tc.number    : DMSLite_SAMGR_QueryInterface_0020
 * @tc.name      : User can not get the required subclass type if the version does not have permission.
 * @tc.desc      : [C- SOFTWARE -0200]
 * @tc.size      : MEDIUM
 * @tc.type      : FUNC
 * @tc.level     : Level 2
 */
LITE_TEST_CASE(IUnknownTestSuite, testQueryInterface0020, LEVEL2)
{
    SAMGR_GetInstance()->RegisterFeatureApi(SERVICE_NAME, FEATURE_NAME, GET_IUNKNOWN(g_feature));
    
    IUnknown *iUnknown = SAMGR_GetInstance()->GetFeatureApi(SERVICE_NAME, FEATURE_NAME);
    DemoApi *demoApi = NULL;
    int resultQuery = iUnknown->QueryInterface(iUnknown, 0x30, (void **)&demoApi);
    TEST_ASSERT_EQUAL_INT(EC_INVALID, resultQuery);

    SAMGR_GetInstance()->UnregisterFeatureApi(SERVICE_NAME, FEATURE_NAME);
}

/**
 * @tc.number    : DMSLite_SAMGR_QueryInterface_0030
 * @tc.name      : User can rewrite this api "QueryInterface".
 * @tc.desc      : [C- SOFTWARE -0200]
 * @tc.size      : MEDIUM
 * @tc.type      : FUNC
 * @tc.level     : Level 2
 */
LITE_TEST_CASE(IUnknownTestSuite, testQueryInterface0030, LEVEL2)
{
    SAMGR_GetInstance()->RegisterFeatureApi(SERVICE_NAME, FEATURE_NAME, (IUnknown *)&g_api);
    IUnknown *iUnknown = SAMGR_GetInstance()->GetFeatureApi(SERVICE_NAME, FEATURE_NAME);
    DemoApi *demoApi = NULL;
    int resultQuery = iUnknown->QueryInterface(iUnknown, 0, (void **)&demoApi);
    TEST_ASSERT_EQUAL_INT(EC_SUCCESS, resultQuery);
    TEST_ASSERT_EQUAL_INT(TRUE, demoApi != NULL);

    SAMGR_GetInstance()->UnregisterFeatureApi(SERVICE_NAME, FEATURE_NAME);
}

RUN_TEST_SUITE(IUnknownTestSuite);