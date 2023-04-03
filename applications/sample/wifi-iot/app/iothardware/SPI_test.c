#include <stdio.h>
#include <string.h>
#include "hi_gpio.h"
#include <hi_spi.h>
#include <hi_io.h>
#include <unistd.h>
 
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_pwm.h"
#include "wifiiot_gpio.h"
#include "wifiiot_spi.h"
#include "wifiiot_gpio_ex.h"
#define RED_INDEX   0  //红色的索引值
#define GREEN_INDEX 1   //绿色的索引值
#define BLUE_INDEX  2   //蓝色的索引值
// #define Delay_Time  
// uint32_t exec1;
// id1 = osTimerNew(Timer1_Callback, osTimerPeriodic, &arg, NULL);
void Bit_delay(void){
    osDelay(1);
    // // Hi3861 1U=10ms,100U=1S
    // exec1 = 1U;
    // if (id1 == NULL)
    // {
    //     printf("[Timer Test] osTimerNew(periodic timer) failed.\r\n");
    //     return;
    // } else{
    //     printf("创建时钟成功!\r\n");
    // }
    // status = osTimerStart(id1, exec1);
    // if (status != osOK)
    // {
    //     printf("时钟不合规矩！\r\n");
    //     return;
    //     // Timer could not be started
    // }else {
    //     printf("时钟合规！\r\n");
    // }
} 


//功能：把 3BYTE 的RGB数据转换成 24BYTE SPI数据
//功能：把 1BYTE 的数据转换成 24Bit SPI数据
static void Button_send(unsigned char rgb)
{
    unsigned char data[24];
    unsigned char i, bit;
    unsigned char index = 0;
    for(i = 0; i < 8; i++) // GREEN  按位发送
    {
        bit = ((rgb << i) & 0x80);
        data[index] = (bit == 0x80) ? 0xfc : 0x3f;//spi发送0xfc 为按键芯片 1码 spi发送0x3f 为按键芯片 0码
        index++;
    } 
    // for(i = 0; i < 8; i++) // RED
    // {
    //     bit = ((rgb[RED_INDEX]<<i) & 0x80);
    //     data[index] = (bit==0x80) ? 0xfc : 0xe0;
    //     // index++;
    // }
    // for(i=0; i<8; i++) // BLUE
    // {
    //     bit = ((rgb[BLUE_INDEX]<<i) & 0x80);
    //     data[index] = (bit==0x80) ? 0xfc : 0xe0;
    //     index++;
    // }
    //主机 SPI 0通道 传输data 8个bit
    hi_spi_host_write(WIFI_IOT_SPI_ID_0, data, 8);
    // Bit_delay();
}
//灯带
unsigned char light[10]=
{
    0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA
};
 
static void WS2812Task(void)
{
 
    //初始化GPIO
    GpioInit();
    // spi 0号通道
    hi_spi_deinit(HI_SPI_ID_0); /* if wake_up from deep sleep, should deinit first */
    hi_spi_cfg_basic_info spi_cfg_basic_info;
    spi_cfg_basic_info.cpha = 0; //CPOL=0，CPHA=0 时钟信号idle状态为低电平，第一个时钟边沿采样数据,CPOL=0，CPHA=1 时钟信号idle状态为低电平，第二个时钟边沿采样数据。
    spi_cfg_basic_info.cpol = 1; //CPOL=1，CPHA=0 时钟信号idle状态为高电平，第一个时钟边沿采样数据,CPOL=1，CPHA=1 时钟信号idle状态为高电平，第二个时钟边沿采样数据。
    spi_cfg_basic_info.data_width = HI_SPI_CFG_DATA_WIDTH_E_8BIT; 
    spi_cfg_basic_info.endian = 0;
    spi_cfg_basic_info.fram_mode = 0;
    spi_cfg_basic_info.freq = 40000; 
    hi_spi_cfg_init_param spi_init_param = {0};
    spi_init_param.is_slave = 0;
    hi_spi_init(HI_SPI_ID_0, spi_init_param, &spi_cfg_basic_info); //基本参数配置
    hi_spi_set_loop_back_mode(HI_SPI_ID_0, 1);
    hi_io_set_func(HI_IO_NAME_GPIO_12, HI_IO_FUNC_GPIO_12_SPI0_CSN);//io配置
    hi_io_set_func(HI_IO_NAME_GPIO_10, HI_IO_FUNC_GPIO_10_SPI0_CK);
    hi_io_set_func(HI_IO_NAME_GPIO_11, HI_IO_FUNC_GPIO_11_SPI0_RXD);
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_SPI0_TXD);
    hi_io_set_driver_strength(HI_IO_NAME_GPIO_6, HI_IO_DRIVER_STRENGTH_2);//驱动能力
    hi_spi_set_irq_mode(HI_SPI_ID_0, 0);
    hi_spi_set_dma_mode(HI_SPI_ID_0, 1);
 
    unsigned char data[50];
    unsigned char begin_low[26];
    unsigned char begin_high[14];
    unsigned char rgb;
    unsigned int i;
    while (1)
    {
        for(i=0;i<26;i++){
            memset(begin_low, 0, sizeof(begin_low));
            hi_spi_host_write(WIFI_IOT_SPI_ID_0, begin_low, sizeof(begin_low));//前50byte为低电平break

        }
        for(i=0;i<14;i++){
            memset(begin_high, 1, sizeof(begin_high));
            hi_spi_host_write(WIFI_IOT_SPI_ID_0, begin_high, sizeof(begin_high));//前50byte为低电平break

        }
        for (i = 0; i < 10; i ++)
        {
            memset(data, 0, sizeof(data));
            // hi_spi_host_write(WIFI_IOT_SPI_ID_0, data, sizeof(data));//前50byte为低电平break
 
            
            rgb = light[i];//灯按钮
            
            Button_send(rgb);
            usleep(10 * 1000);
            
            
        }
        i = 0;
    }
    // while (1)
    // {
    //     for (i = 0; i < 256; i ++)
    //     {
    //         memset(data, 0, sizeof(data));
    //         hi_spi_host_write(WIFI_IOT_SPI_ID_0, data, sizeof(data));//前50byte为低电平break
 
    //         for (unsigned int j = 0; j < 256; j ++)//亮度
    //         {
    //             rgb[0] = color[i%7][0]&j;//色带
    //             rgb[1] = color[i%7][1]&j;
    //             rgb[2] = color[i%7][2]&j;
    //             for (unsigned int k = 0; k < 256; k ++)//控制256个灯
    //             Button_send(rgb);
    //             usleep(10 * 1000);
    //         }
            
    //     }
    //     i = 0;
    // }
}
 
static void WS2812ExampleEntry(void)
{
    osThreadAttr_t attr;
 
    attr.name = "WS2812Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityAboveNormal6 ;
 
    if (osThreadNew((osThreadFunc_t)WS2812Task, NULL, &attr) == NULL)//创建线程
    {
        printf("Falied to create WS2812Task!\n");
    }
}
 
APP_FEATURE_INIT(WS2812ExampleEntry);