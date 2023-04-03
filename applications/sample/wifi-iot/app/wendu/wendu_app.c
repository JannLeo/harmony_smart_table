
#include "wendu.h"

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "wifiiot_i2c_ex.h"

void Aht20TestTask(void* arg)
{
    (void) arg;
    uint32_t retval = 0;
    //GPIO_0复用为I2C1_SDA
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_0, WIFI_IOT_IO_FUNC_GPIO_0_I2C1_SDA);

    //GPIO_1复用为I2C1_SCL
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_1, WIFI_IOT_IO_FUNC_GPIO_1_I2C1_SCL);

    //baudrate: 400kbps
    I2cInit(WIFI_IOT_I2C_IDX_1, 400000);
    I2cSetBaudrate(WIFI_IOT_I2C_IDX_1, 400000);


    // IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    // IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);

    // I2cInit(WIFI_IOT_I2C_IDX_0, 400*1000);

    retval = AHT20_Calibrate();
    printf("AHT20_Calibrate: %d\r\n", retval);

    while (1) {
        float temp = 0.0, humi = 0.0;

        retval = AHT20_StartMeasure();
        printf("AHT20_StartMeasure: %d\r\n", retval);

        retval = AHT20_GetMeasureResult(&temp, &humi);
        printf("AHT20_GetMeasureResult: %d, temp = %.2f, humi = %.2f\r\n", retval, temp, humi);
        if((temp >= 65&&humi <= 10)||(temp <= -20 && humi <= 10)){
            // GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 0);
            printf("温度过高！请及时降温！\r\n");
        }
        else{
            // GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 1);
        }
        sleep(1);
    }
}

void Aht20Test(void)
{
    osThreadAttr_t attr;
    // IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    // GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    attr.name = "Aht20Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(Aht20TestTask, NULL, &attr) == NULL) {
        printf("[Aht20Test] Failed to create Aht20TestTask!\n");
    }
}
APP_FEATURE_INIT(Aht20Test);