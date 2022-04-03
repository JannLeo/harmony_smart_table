#ifndef __NETCFG_H__
#define __NETCFG_H__

#define NETCFG_TASK_STACK_SIZE (4096)       // 栈大小
#define NETCFG_TASK_PRIO (30)               // 任务优先级
#define NETCFG_TIME_COUNT 5                 // 配网重试次数
#define DEV_INFO_NUM 2                      // 设备信息数组长度
#define POWER_NUM (-52)                     // 功率 -52dBm 秒控配网级别 这个是可以设置的，不同级别发射功率不一样
#define MAX_DATA_LEN 4096                   // 最大数据长度
#define FAST_CONNECT_RETRY_NUM 3            // 秒控最大尝试次数

#define CHANNEL_80211B_ONLY 14              // 802.11b 只有14个通道
#define FREQ_OF_CHANNEL_1 2412              // 802.11b 开始频率
#define FREQ_OF_CHANNEL_80211B_ONLY 2484    // 802.11b 最大频率
#define WIFI_MIN_CHANNEL 1                  // 最小通道
#define WIFI_FREQ_INTERVAL 5                // 无线频率间隔
#define TEST_CONNECT_RETRY_COUNT 5          // 尝试重新连接的最大次数

typedef enum {
    NET_EVENT_NULL,         // 空
    NET_EVENT_CONFIG,       // 网络配置
    NET_EVENT_CONFIG_FAIL,  // 连接WiFi失败
    NET_EVENT_CONFIG_SUCC,  // 连接WiFi成功
    NET_EVENT_CONNECTTED,   // WiFi已连接
    NET_EVENT_DISCONNECT,   // WiFi断开
    NET_EVENT_RECV_DATA,    // 从FA接收到的数据
    NET_EVENT_SEND_DATA,    // 发送给FA的数据
    NET_EVENT_TYPE_NBR      // 未知类型
}NET_EVENT_TYPE;

/**
 * @brief: 网络配置服务回调
 *
 * @param event reference {@link NET_EVENT_TYPE}
 * @param data 事件数据: NET_EVENT_RECV_DATA 或者 NET_EVENT_SEND_DATA
 * @since 1.0
 * @version 1.0
 */
typedef int (*NetCfgEventCallback)(NET_EVENT_TYPE event, void *data);

/**
 * @brief 注册网络配置任务
 *
 * @param nEventCallback 网格模块的回调
 * @since 1.0
 * @version 1.0
 */
void NetCfgRegister(NetCfgEventCallback nEventCallback);

#endif //__NETCFG_H__