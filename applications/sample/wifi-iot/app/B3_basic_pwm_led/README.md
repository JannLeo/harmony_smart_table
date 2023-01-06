# BearPi-HM_Nano开发板基础外设开发——PWM输出
本示例将演示如何在BearPi-HM_Nano开发板上使用GPIO的PWM功能实现呼吸灯的效果

![BearPi-HM_Nano](/applications/BearPi/BearPi-HM_Nano/docs/figures/00_public/BearPi-HM_Nano.png)
## PWM API分析
本案例主要使用了以下几个API完成PWM功能实现呼吸灯功能
## GpioInit()
```c
unsigned int GpioInit (void )
```
 **描述：**

初始化GPIO外设
## IoSetFunc()
```c
unsigned int IoSetFunc (WifiIotIoName id, unsigned char val )
```
**描述：**

设置GPIO引脚复用功能

**参数：**

|名字|描述|
|:--|:------| 
| id | 表示GPIO引脚号.  |
| val | 表示GPIO复用功能 |

## GpioSetDir()
```c
unsigned int GpioSetDir (WifiIotGpioIdx id, WifiIotGpioDir dir )
```
**描述：**

设置GPIO输出方向

**参数：**

|名字|描述|
|:--|:------| 
| id | 表示GPIO引脚号.  |
| dir | 表示GPIO输出方向.  |


## PwmInit()
```c
unsigned int PwmInit (WifiIotPwmPort port)
```
**描述：**
初始化PWM功能

**参数：**

|名字|描述|
|:--|:------| 
| id | 表示GPIO引脚号.  |
| val | 表示要设置的上拉或下拉.  |


## PwmStart()
```c
unsigned int PwmStart (WifiIotPwmPort port, unsigned short duty, unsigned short freq )
```
**描述：**

根据输入参数输出PWM信号。

**参数：**

|名字|描述|
|:--|:------| 
| port | PWM端口号.  |
| duty| 占空比.  |
| freq| 分频倍数.  |


## 硬件设计
本案例将使用板载的LED来验证GPIO的PWM功能，在BearPi-HM_Nano开发板上LED的连接电路图如下图所示，LED的控制引脚与主控芯片的GPIO_2连接，所以需要编写软件去控制GPIO_2输出PWM波实现呼吸灯的效果。

![](/applications/BearPi/BearPi-HM_Nano/docs/figures/B3_basic_pwm_led/LED灯电路.png "LED灯电路")

## 软件设计

**主要代码分析**

PWMTask()为PWM测试主任务，该任务先调用 GpioInit()初始化GPIO，因为LED灯的控制引脚接在GPIO_2上，所以通过调用IoSetFunc()将GPIO_2复用为PWM功能，并通过PwmInit()初始化PWM2端口，最后在死循环里面间隔10us输出不同占空比的PWM波，实现呼吸灯的效果
```c
static void PWMTask(void)
{
    unsigned int i;

    GpioInit();//初始化GPIO
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_IO_FUNC_GPIO_2_PWM2_OUT);//设置GPIO_2引脚复用功能为PWM
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_GPIO_DIR_OUT);//设置GPIO_2引脚为输出模式
    PwmInit(WIFI_IOT_PWM_PORT_PWM2);//初始化PWM2端口

    while (1) 
        {
            for (i = 0; i < 40000; i += 100)
            {
                PwmStart(WIFI_IOT_PWM_PORT_PWM2, i, 40000); //输出不同占空比的PWM波
                usleep(10);
            }            
            i = 0;
        }    
}
```

## 编译调试

### 修改 usr_config.mk文件
修改`vendor\hisi\hi3861\hi3861\build\config` 路径下 usr_config.mk 文件，定位到第40行，打开PWM驱动使能。

```r
CONFIG_I2C_SUPPORT=y
# CONFIG_I2S_SUPPORT is not set
# CONFIG_SPI_SUPPORT is not set
# CONFIG_DMA_SUPPORT is not set
# CONFIG_SDIO_SUPPORT is not set
# CONFIG_SPI_DMA_SUPPORT is not sety
# CONFIG_UART_DMA_SUPPORT is not set
CONFIG_PWM_SUPPORT = y
# CONFIG_PWM_HOLD_AFTER_REBOOT is not set
CONFIG_AT_SUPPORT=y
CONFIG_FILE_SYSTEM_SUPPORT=y
CONFIG_UART0_SUPPORT=y
CONFIG_UART1_SUPPORT=y
```

### 修改 BUILD.gn 文件


修改`applications\BearPi\BearPi-HM_Nano\sample` 路径下 BUILD.gn 文件，指定 `pwm_example` 参与编译。

```r
#"B1_basic_led_blink:led_example",
#"B2_basic_button:button_example",
"B3_basic_pwm_led:pwm_example",
#"B4_basic_adc:adc_example",
#"B5_basic_i2c_nfc:i2c_example",
#"B6_basic_uart:uart_example",
```   

    


### 运行结果<a name="section18115713118"></a>

示例代码编译烧录代码后，按下开发板的RESET按键，开发板开始正常工作，LED开始不断变化亮度，实现呼吸灯的效果。

