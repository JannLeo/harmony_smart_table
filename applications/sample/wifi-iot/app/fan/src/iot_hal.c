#include "iot_hal.h"
#include "iot_errno.h"
#include "hi_io.h"
#include "hi_i2c.h"

/**
 * 设置IO的复用功能，一个引脚有多个功能，例如普通IO I2C SPI 某一个时间只能是其中的一个功能
 * id: 硬件引脚号
 * val: 复用功能号
 * return: 返回是否成功
 */ 
unsigned int IoSetFunc(IotIoName id, unsigned char *val)
{
    if(id == IOT_IO_NAME_MAX)
    {
        return IOT_FAILURE;
    }

    return hi_io_set_func((hi_io_name)id, val);
}

/**
* @ingroup  iot_io
* @brief Enables the I/O pull-up.CNcomment:设置某个IO上下拉功能。CNend
*
* @par 描述:
*           Enables the I/O pull-up.CNcomment:设置某个IO上下拉功能。CNend
*
* @attention None
* @param  id  [IN]  type #hi_io_name，I/O index.CNcomment:硬件管脚。CNend
* @param  val [IN]  type #hi_io_pull，I/O pull-up enable.CNcomment:待设置的上下拉状态。CNend
*
* @retval 0       Success
* @retval #HI_ERR_GPIO_INVALID_PARAMETER     Failure. Input invalid.
*/
unsigned int IoTSetPull(IotIoName id, IoTIoPull iotIoPull) 
{
    return hi_io_set_pull(id, (hi_io_pull)iotIoPull);
}

/**
 * I2C 写读
 * id: 硬件I2C序号
 * deviceAddr:设备地址
 * i2cData:I2C传输的数据结构，包括发送数据和接收数据
 * return: 返回是否成功
 */ 
unsigned int IoTI2cWriteread(unsigned int id, unsigned short deviceAddr, const IotI2cData *i2cData)
{
    return hi_i2c_writeread(id, deviceAddr, (const hi_i2c_data *)i2cData);
}