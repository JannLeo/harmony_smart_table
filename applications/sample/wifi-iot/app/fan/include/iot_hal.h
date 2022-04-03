#ifndef __IOT_HAL_H__
#define __IOT_HAL_H__

#define I2C_IDX_BAUDRATE                  (400000)

#define IOT_IO_FUNC_GPIO_0_I2C1_SDA   6
#define IOT_IO_FUNC_GPIO_1_I2C1_SCL   6
#define IOT_I2C_IDX_1                 1

//硬件IO管脚定义
typedef enum {
    //GPIO hardware pin 0
    IOT_IO_NAME_GPIO_0,
    //GPIO hardware pin 1
    IOT_IO_NAME_GPIO_1,
    //GPIO hardware pin 2
    IOT_IO_NAME_GPIO_2,
    //GPIO hardware pin 3
    IOT_IO_NAME_GPIO_3,
    //GPIO hardware pin 4
    IOT_IO_NAME_GPIO_4,
    //GPIO hardware pin 5
    IOT_IO_NAME_GPIO_5,
    //GPIO hardware pin 6
    IOT_IO_NAME_GPIO_6,
    //GPIO hardware pin 7
    IOT_IO_NAME_GPIO_7,
    //GPIO hardware pin 8
    IOT_IO_NAME_GPIO_8,
    //GPIO hardware pin 9
    IOT_IO_NAME_GPIO_9,
    //GPIO hardware pin 10
    IOT_IO_NAME_GPIO_10,
    //GPIO hardware pin 11
    IOT_IO_NAME_GPIO_11,
    //GPIO hardware pin 12
    IOT_IO_NAME_GPIO_12,
    //GPIO hardware pin 13
    IOT_IO_NAME_GPIO_13,
    //GPIO hardware pin 14
    IOT_IO_NAME_GPIO_14,
    //Maximum value
    IOT_IO_NAME_MAX,
}IotIoName;

/**
 * @ingroup iot_io
 *
 * GPIO pull-up configuration.CNcomment:IO上下拉功能CNend
 */
typedef enum {
    IOT_IO_PULL_NONE,    /**< Disabled.CNcomment:无拉CNend */
    IOT_IO_PULL_UP,      /**< Pull-up enabled.CNcomment:上拉CNend */
    IOT_IO_PULL_DOWN,    /**< Pull-down enabled.CNcomment:下拉CNend */
    IOT_IO_PULL_MAX,     /**< Invalid.CNcomment:无效值CNend */
} IoTIoPull;

/**
 * @brief 定义I2C数据传输属性。 
 */
typedef struct {
    /** 指向存储要发送数据的缓冲区的指针  */
    unsigned char *sendBuf;
    /** 发送数据的长度 */
    unsigned int  sendLen;
    /** 指向存储要接收数据的缓冲区的指针  */
    unsigned char *receiveBuf;
    /** 接收数据的长度 */
    unsigned int  receiveLen;
} IotI2cData;

/**
 * @brief I2C硬件序号
 */
typedef enum {
    /** I2C 硬件序号0 */
    WIFI_IOT_I2C_IDX_0,
    /** I2C 硬件序号1 */
    WIFI_IOT_I2C_IDX_1,
} IotI2cIdx;

/**
 * 设置IO的复用功能，一个引脚有多个功能，例如普通IO I2C SPI 某一个时间只能是其中的一个功能
 * id: 硬件引脚号
 * val: 复用功能号
 * return: 返回是否成功
 */ 
unsigned int IoSetFunc(IotIoName id, unsigned char *val);

/**
* 设置某个IO上下拉功能。CNend
* @param  id  [IN]  硬件管脚。CNend
* @param  val [IN]  待设置的上下拉状态
*
* @retval 0       Success
* @retval #HI_ERR_GPIO_INVALID_PARAMETER     Failure. Input invalid.
*/
unsigned int IoTSetPull(IotIoName id, IoTIoPull iotIoPull); 

/**
 * I2C 写读
 * id: 硬件I2C序号
 * deviceAddr:设备地址
 * i2cData:I2C传输的数据结构，包括发送数据和接收数据
 * @retval: 返回是否成功
 */ 
unsigned int IoTI2cWriteread(unsigned int id, unsigned short deviceAddr, const IotI2cData *i2cData);

#endif