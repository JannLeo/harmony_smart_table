#include "ohos_init.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "unistd.h"
#include "stdio.h" 

void led_example(void)
{
    GpioInit();//初始化GPIO
    //设置GPIO广角复用功能   引脚号，普通GPIO
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2,WIFI_IOT_IO_FUNC_GPIO_2_GPIO);
    //设置GPIO方向为输出模式   引脚号，输出模式
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2,WIFI_IOT_GPIO_DIR_OUT);
    //闪烁十次
    while(1){
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,1);
        usleep(1000000);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,0);
        usleep(1000000);
        printf("running!\r\n");
    }
    // //设置GPIO高低电频  1为高电平
    // GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,1);

}
//启动LED_example
APP_FEATURE_INIT(led_example);