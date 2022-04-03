#include "iot_gpio.h"
#include "iot_errno.h"
#include "iot_hal.h"

#include "led.h"

// LED灯引脚初始化
void LedInit(void) 
{
    // 初始化GPIO_2 LED
    IoTGpioInit(IOT_IO_NAME_GPIO_2);
    // 设置GPIO_2为输出模式
    IoTGpioSetDir(IOT_IO_NAME_GPIO_2, IOT_GPIO_DIR_OUT);
    // 初始化为关灯状态
    LedOff();
}

// LED灯亮
void LedOn(void) 
{
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_2, 1);
}

// LED灯灭
void LedOff(void) 
{
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_2, 0);
}
