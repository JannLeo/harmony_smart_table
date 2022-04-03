#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <stdio.h>
#include <string.h>

#include <cmsis_os2.h>

// 普通LOG 打印
#define LOG_ERR(fmt, args...) printf("[ERROR][%s|%d]", fmt, __func__, __LINE__, ##args)
#define LOG_DEBUG(fmt, args...) printf("[DEBUG][%s|%d]", fmt, __func__, __LINE__, ##args)
#define LOG_INFO(fmt, args...) printf("[INFO][%s|%d]", fmt, __func__, __LINE__, ##args)
// 风扇信息打印
#define FAN_ERR(fmt, args...) printf("[FAN_ERROR][%s|%d]", fmt, __func__, __LINE__, ##args)
#define FAN_DEBUG(fmt, args...) printf("[FAN_DEBUG][%s|%d]", fmt, __func__, __LINE__, ##args)
#define FAN_INFO(fmt, ...) printf("[FAN_INFO][%s|%d]", fmt, __func__, __LINE__, __VA_ARGS__)

// 网络信息打印
#define NET_ERR(fmt, args...) printf("[NET_ERROR][%s|%d]", fmt, __func__, __LINE__, ##args)
#define NET_DEBUG(fmt, args...) printf("[NET_DEBUG][%s|%d]", fmt, __func__, __LINE__, ##args)
#define NET_INFO(fmt, args...) printf("[NET_INFO][%s|%d]", fmt, __func__, __LINE__, ##args)
// 键盘信息打印
#define KEY_ERR(fmt, args...) printf("[KEY_ERROR][%s|%d]", fmt, __func__, __LINE__, ##args)
#define KEY_DEBUG(fmt, args...) printf("[KEY_DEBUG][%s|%d]", fmt, __func__, __LINE__, ##args)
#define KEY_INFO(fmt, args...) printf("[KEY_INFO][%s|%d]", fmt, __func__, __LINE__, ##args)

#endif //__DEFINES_H__