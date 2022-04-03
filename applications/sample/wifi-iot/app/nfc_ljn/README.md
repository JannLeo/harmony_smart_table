# BearPi-HM_Nano开发板基础外设开发——I2C控制NFC芯片
本示例将演示如何在BearPi-HM_Nano开发板上使用I2C协议向NFC芯片写入数据

![BearPi-HM_Nano](/applications/BearPi/BearPi-HM_Nano/docs/figures/00_public/BearPi-HM_Nano.png)
## I2C API分析
本示例主要使用了以下API完成I2C采样的功能
## I2cInit()
```c
unsigned int I2cInit (WifiIotI2cIdx id, unsigned int baudrate )
```
 **描述：**

用指定的频率初始化I2C设备


**参数：**

|名字|描述|
|:--|:------| 
| id | I2C设备ID.  |
| baudrate |I2C频率|

## I2cSetBaudrate()
```c
unsigned int I2cSetBaudrate (WifiIotI2cIdx id, unsigned int baudrate )
```
 **描述：**

为I2C设备设置频率


**参数：**

|名字|描述|
|:--|:------| 
| id | I2C设备ID.  |
| baudrate |I2C频率|

## I2cWrite()
```c
unsigned int I2cWrite (WifiIotI2cIdx id, unsigned short deviceAddr, const WifiIotI2cData * i2cData )
```
 **描述：**

将数据写入I2C设备


**参数：**

|名字|描述|
|:--|:------| 
| id | I2C设备ID.  |
| deviceAddr |I2C设备地址|
| i2cData |表示写入的数据|

## I2cRead()
```c
unsigned int I2cRead (WifiIotI2cIdx id, unsigned short deviceAddr, const WifiIotI2cData * i2cData )
```
 **描述：**

从I2C设备读取数据。读取的数据将保存到i2cData指定的地址


**参数：**

|名字|描述|
|:--|:------| 
| id | I2C设备ID.  |
| deviceAddr |I2C设备地址|
| i2cData |表示要读取的数据指向的指针|



## 硬件设计
如下图所示，NFC芯片使用的是I2C协议，I2C_SCL与GPIO_0相连接，I2C_SDA与GPIO_1相连接，所以需要编写软件使用GPIO_0和GPIO_1产生I2C信号去控制NFC芯片

![](/applications/BearPi/BearPi-HM_Nano/docs/figures/B5_basic_i2c_nfc/NFC电路.png "E53接口电路")

## 软件设计

**主要代码分析**

这部分代码为I2C初始化的代码，首先用 `IoSetFunc()` 函数将GPIO_0复用为I2C1_SDA，GPIO_1复用为I2C1_SCL。然后调用I2cInit()函数初始化I2C1端口，最后使用 `I2cSetBaudrate()` 函数设置I2C1的频率为400kbps.
```c
IoSetFunc(WIFI_IOT_IO_NAME_GPIO_0, WIFI_IOT_IO_FUNC_GPIO_0_I2C1_SDA);   // GPIO_0复用为I2C1_SDA
IoSetFunc(WIFI_IOT_IO_NAME_GPIO_1, WIFI_IOT_IO_FUNC_GPIO_1_I2C1_SCL);   // GPIO_1复用为I2C1_SCL
I2cInit(WIFI_IOT_I2C_IDX_1, 400000); /* baudrate: 400kbps */
I2cSetBaudrate(WIFI_IOT_I2C_IDX_1, 400000);
```
这部分的代码是向NFC芯片写入数据，但需要写入2个记录时，第2个记录的位置需要用`NDEFLastPos`来定义;当需要写入3个记录时，第2个和第3个记录的位置分别需要用`NDEFMiddlePos`和`NDEFLastPos`来定义。
```c
ret=storeText(NDEFFirstPos, (uint8_t *)TEXT);
if(ret != 1)
{
    printf("NFC Write Data Falied :%d ",ret);
}
ret=storeUrihttp(NDEFLastPos, (uint8_t *)WEB);
if(ret != 1)
{
    printf("NFC Write Data Falied :%d ",ret);
}
```


## 编译调试

### 修改 BUILD.gn 文件


修改 `applications\BearPi\BearPi-HM_Nano\sample` 路径下 BUILD.gn 文件，指定 `i2c_example` 参与编译。

```r
#"B1_basic_led_blink:led_example",
#"B2_basic_button:button_example",
#"B3_basic_pwm_led:pwm_example",
#"B4_basic_adc:adc_example",
"B5_basic_i2c_nfc:i2c_example",
#"B6_basic_uart:uart_example",
```   

    


### 运行结果<a name="section18115713118"></a>

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，并请使用带有NFC功能的手机靠近开发板，能读取数据。
```c
=======================================

***********I2C_NFC_example**********

=======================================

Please use the mobile phone with NFC function close to the development board!

=======================================

***********I2C_NFC_example**********

=======================================

Please use the mobile phone with NFC function close to the development board!
```
