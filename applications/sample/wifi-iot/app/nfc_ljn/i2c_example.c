/*
 * Copyright (c) 2020 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_errno.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "wifiiot_i2c_ex.h"
#include "nfc.h"

#define I2C_TASK_STACK_SIZE 1024 * 8
#define I2C_TASK_PRIO 25

#define TEXT "Hello World!"
#define WEB "bilibili.com"

static void I2CTask(void)
{
    uint8_t ret;
    GpioInit();

    // GPIO_0复用为I2C1_SDA
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_0, WIFI_IOT_IO_FUNC_GPIO_0_I2C1_SDA);

    // GPIO_1复用为I2C1_SCL
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_1, WIFI_IOT_IO_FUNC_GPIO_1_I2C1_SCL);

    // baudrate: 400kbps
    I2cInit(WIFI_IOT_I2C_IDX_1, 400000);

    I2cSetBaudrate(WIFI_IOT_I2C_IDX_1, 400000);

    printf("I2C Test Start\n");

    ret = storeText(NDEFFirstPos, (uint8_t *)TEXT);
    if (ret != 1)
    {
        printf("NFC Write Data Falied :%d ", ret);
    }
    ret = storeUrihttp(NDEFLastPos, (uint8_t *)WEB);
    if (ret != 1)
    {
        printf("NFC Write Data Falied :%d ", ret);
    }
    while (1)
    {
        printf("=======================================\r\n");
        printf("***********I2C_NFC_example**********\r\n");
        printf("=======================================\r\n");
        printf("Please use the mobile phone with NFC function close to the development board!\r\n");
        usleep(1000000);
    }
}

static void I2CExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "I2CTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = I2C_TASK_STACK_SIZE;
    attr.priority = I2C_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)I2CTask, NULL, &attr) == NULL)
    {
        printf("Falied to create I2CTask!\n");
    }
}

APP_FEATURE_INIT(I2CExampleEntry);