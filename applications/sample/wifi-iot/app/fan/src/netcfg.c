#include <unistd.h>
#include "cmsis_os2.h"
#include "base64.h"

#include "ohos_types.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/ip4_addr.h"
#include "wifi_hotspot.h"
#include "wifi_device_config.h"
#include "netcfg.h"
#include "network_config_service.h"

#include "defines.h"
#include "kv_store.h"

WifiDeviceConfig g_netCfg = {0};
WifiEvent g_staEventHandler = {0};
struct netif *g_staNetif = NULL;

// 连接重试次数
int g_connectRetryCount = 0;

static NetCfgEventCallback g_netCfgEventCallback = NULL;

#define SET_NET_EVENT(e, d)    ({           \
    if (g_netCfgEventCallback != NULL) {    \
        g_netCfgEventCallback(e, d);        \
    }                                       \
})

const char *g_ssid = "ShenHongHui";
const char *g_pinCode = "11111111";
const char *g_productId = "9AG900";
const char *g_sn = "01234567890123450123456789012345";

static int g_state;

/**
 * 通道频率
 * @channel 通道号
 * @return  通道频率
 */ 
static unsigned int ChannelToFrequency(unsigned int channel)
{
    if (channel <= 0) {
        return 0;
    }

    if (channel == CHANNEL_80211B_ONLY) {
        return FREQ_OF_CHANNEL_80211B_ONLY;
    }

    return (((channel - WIFI_MIN_CHANNEL) * WIFI_FREQ_INTERVAL) + FREQ_OF_CHANNEL_1);
}

// 网络配置结果
static void NetCfgResult(signed char result)
{
    printf("Network configure done.(result=%d)\n", result);
    // 注销Wifi处理事件
    UnRegisterWifiEvent(&g_staEventHandler);
    // 通知网络配置结果
    NotifyNetCfgResult(result);
}

// 设备重设IP地址
static void StaResetAddr(struct netif *ptrLwipNetif)
{
    ip4_addr_t staGW;
    ip4_addr_t staIpaddr;
    ip4_addr_t staNetmask;

    if (ptrLwipNetif == NULL) {
        NET_ERR("Null param of netdev.\n");
        return;
    }

    IP4_ADDR(&staGW, 0, 0, 0, 0);
    IP4_ADDR(&staIpaddr, 0, 0, 0, 0);
    IP4_ADDR(&staNetmask, 0, 0, 0, 0);

    netifapi_netif_set_addr(ptrLwipNetif, &staIpaddr, &staNetmask, &staGW);
}

// Wifi连接处理任务
static void *WifiConnectTask(const char *arg)
{
    (void)arg;
    if (g_state == WIFI_STATE_AVALIABLE) {
        NetCfgResult(0);
        NET_INFO("WiFi: Connected.\n");
        // 开始设置DHCP
        netifapi_dhcp_start(g_staNetif);
    } else if (g_state == WIFI_STATE_NOT_AVALIABLE) {
        printf("WiFi: Disconnected retry = %d.\n", g_connectRetryCount);
        if (g_connectRetryCount < TEST_CONNECT_RETRY_COUNT) {
            g_connectRetryCount++;
            return NULL;
        }
        NetCfgResult(-1);
        // 停止设置DHCP
        netifapi_dhcp_stop(g_staNetif);
        // 重设IP地址
        StaResetAddr(g_staNetif);
    }
    return NULL;
}

// Wifi连接状态改变处理
static void WifiConnectionChangedHandler(int state, WifiLinkedInfo *info)
{
    (void)info;
    osThreadAttr_t attr;
    attr.name = "WifiConnectTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = NETCFG_TASK_STACK_SIZE;
    attr.priority = NETCFG_TASK_PRIO;
    g_state = state;
    if (osThreadNew((osThreadFunc_t)WifiConnectTask, NULL, &attr) == NULL) {
        NET_ERR("Falied to create WifiConnectTask!\n");
    }
}

// 开启sta 连接网络
static int StaStart(void)
{
    WifiErrorCode error;
    // 启用Wifi
    error = EnableWifi();
    if (error == ERROR_WIFI_BUSY) {
        NET_ERR("Sta had already connnected.\n");
        NetCfgResult(0);
    }
    // 启用失败
    if ((error != ERROR_WIFI_BUSY) && (error != WIFI_SUCCESS)) {
        printf("EnableWifi failed, error = %d.\n", error);
        return -1;
    }

    g_staEventHandler.OnWifiConnectionChanged = WifiConnectionChangedHandler;

    // 注册WiFi事件回调
    error = RegisterWifiEvent(&g_staEventHandler);
    if (error != WIFI_SUCCESS) {
        printf("RegisterWifiEvent failed, error = %d.\n", error);
        return -1;
    }

    // 查询Wifi是否处于ACTIVE状态
    if (IsWifiActive() == WIFI_STA_NOT_ACTIVE) {
        NET_ERR("Wifi station is not actived.\n");
        return -1;
    }
    
    // 获取wlan0无线接口
    g_staNetif = netif_find("wlan0");
    if (g_staNetif == NULL) {
        NET_ERR("Get netif failed.\n");
        return -1;
    }

    return 0;
}

// 连接wifi
static int WapStaConnect(WifiDeviceConfig *config)
{
    int netId = 0;
    WifiErrorCode error;
    config->securityType = (config->preSharedKey[0] == '\0') ? WIFI_SEC_TYPE_OPEN : WIFI_SEC_TYPE_PSK;
    error = AddDeviceConfig(config, &netId);
    if (error != WIFI_SUCCESS) {
        printf("AddDeviceConfig add config failed, error=%d.\n", error);
        return -1;
    }
    int count = 0;
    // 尝试3次连接
    while (count < FAST_CONNECT_RETRY_NUM) {
        error = ConnectTo(netId);
        if (error == WIFI_SUCCESS) {
            break;
        }
        NET_INFO("[sample]continue.\n");
        count++;
    }

    if (error != WIFI_SUCCESS) {
        printf("ConnectTo conn failed %d.\n", error);
        return -1;
    }

    NET_INFO("WapSta connecting...\n");
    return 0;
}

static void *CfgNetTask(const char *arg)
{
    (void)arg;

    if (StaStart() != 0) {
        SET_NET_EVENT(NET_EVENT_CONFIG_FAIL, NULL);
        return NULL;
    }

    if (WapStaConnect(&g_netCfg) != 0) {
        SET_NET_EVENT(NET_EVENT_CONFIG_FAIL, NULL);
        return NULL;
    }

    printf("ssid:%s, password:%s\n", g_netCfg.ssid, g_netCfg.preSharedKey);

    SET_NET_EVENT(NET_EVENT_CONNECTTED, NULL);

    return NULL;
}

static int CreateCfgNetTask(void)
{
    osThreadAttr_t attr;
    attr.name = "CfgNetTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = NETCFG_TASK_STACK_SIZE;
    attr.priority = NETCFG_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)CfgNetTask, NULL, &attr) == NULL) {
        NET_ERR("Falied to create NanCfgNetTask!");
        return -1;
    }

    return 0;
}

// 处理SSID和密码
static void DealSsidPwd(const WifiDeviceConfig *config)
{
    if (config == NULL) {
        NET_ERR("Input config is illegal.\n");
        return;
    }

    if (memcpy_s(&g_netCfg, sizeof(WifiDeviceConfig), config, sizeof(WifiDeviceConfig)) != 0) {
        NET_ERR("memcpy netCfg failed.\n");
        return;
    }

    NET_INFO("DealSsidPwd");
    g_connectRetryCount = 0;
    // 创建处理网络配置任务
    if (CreateCfgNetTask() != 0) {
        NET_ERR("Create cfgnet task failed.\n");
    }
}

// 获取PIN码
int GetPinCode(unsigned char *pinCode, unsigned int size, unsigned int *len)
{
    if (pinCode == NULL) {
        return -1;
    }
    memset_s(pinCode, size, 0, size);
    if (strncpy_s((char *)pinCode, size, g_pinCode, strlen(g_pinCode)) != 0) {
        NET_ERR("GetPinCode copy pinCode failed.\n");
        return -1;
    }

    *len = strlen((char *)pinCode);
    
    return 0;
}

// WiFi信息拷贝
int FastConnect(const WifiInfo *wifiInfo, WifiDeviceConfig *destCfg)
{
    if (memcpy_s(destCfg->ssid, sizeof(destCfg->ssid), wifiInfo->ssid, wifiInfo->ssidLen) != EOK) {
        NET_ERR("FastConnect copy ssid failed.\n");
        return -1;
    }
    if (memcpy_s(destCfg->preSharedKey, sizeof(destCfg->preSharedKey), wifiInfo->psk, wifiInfo->pskLen) != EOK) {
        NET_ERR("FastConnect copy pwd failed.\n");
        return -1;
    }
    if (memcpy_s(destCfg->bssid, sizeof(destCfg->bssid), wifiInfo->bssid, wifiInfo->bssidLen) != EOK) {
        NET_ERR("FastConnect copy bssid failed.\n");
        return -1;
    }
    // 安全类型
    destCfg->securityType = wifiInfo->authMode;
    // WIFI 频率
    destCfg->freq = ChannelToFrequency(wifiInfo->channelNumber);
    // WPA/2-PSK 密码格式 有两种格式 长度是8~63的ASCII码格式和64字节的hex 格式
    destCfg->wapiPskType = WIFI_PSK_TYPE_HEX;
    return 0;
}

// 解析网络配置数据
int ParseNetCfgData(const WifiInfo *wifiInfo, const unsigned char *vendorData, unsigned int len)
{
    printf("ParseWifiData vendorData len:%d.\n", len);
    if (wifiInfo == NULL) {
        NET_ERR("wifiInfo is NULL.\n");
        return -1;
    }

    WifiDeviceConfig netConfig;
    memset_s(&netConfig, sizeof(netConfig), 0, sizeof(netConfig));

    // WiFi信息拷贝
    FastConnect(wifiInfo, &netConfig);

    if (vendorData != NULL) {
        /* process vendorData */
        printf("vendorData:%s\n",vendorData);
    }

    DealSsidPwd(&netConfig);
    return 0;
}


// 通知网络配置状态
void NotifyNetCfgStatus(NetCfgStatus status)
{
    if(status == NETCFG_IDENTIFY_DISABLE) {
        LedOff();
    }else if(status == NETCFG_IDENTIFY_ENABLE) {
        LedOn();
    }
    return;
}

// 接收数据
static int RecvRawData(const char *svcId, unsigned int mode, const char *data)
{
    (void)svcId;
    (void)mode;

    printf("data : %s.\n", data);
    // 处理接收数据
    SET_NET_EVENT(NET_EVENT_RECV_DATA, (void *)data);
    return 0;
}

static void *NetCfgTask(void *arg)
{
    (void)arg;
    int ret;
    // 确保设备发现距离约 30 厘米以内
    ret = SetSafeDistancePower(POWER_NUM); 
    if (ret != 0) {
        NET_ERR("Set saft distance power failed.\n");
        return NULL;
    }

    // 热点信息配置
    SoftAPParam config = {0};
    // 热点信息配置初始化
    memset_s(&config, sizeof(SoftAPParam), 0, sizeof(SoftAPParam));
    // 初始化SSID
    strncpy_s(config.ssid, sizeof(config.ssid), g_ssid, strlen(g_ssid));
    // 热点认证类型 Open
    config.authType = WIFI_SECURITY_OPEN;
    // 设置热点参数
    ret = SetSoftAPParameter(&config);
    if (ret != 0) {
        NET_ERR("Set softAP parameters failed.\n");
        return NULL;
    }

    // 网络配置
    NetCfgCallback hook;
    // 网络配置初始化
    memset_s(&hook, sizeof(NetCfgCallback), 0, sizeof(NetCfgCallback));
    hook.GetPinCode = GetPinCode;           // 获取PIN码
    hook.ParseNetCfgData = ParseNetCfgData; // 解析网络配置信息
    hook.RecvRawData = RecvRawData;         // 接收数据
    hook.NotifyNetCfgStatus = NotifyNetCfgStatus; // 网络状态改变通知
    ret = RegNetCfgCallback(&hook);         // 注册网络配置任务
    if (ret != 0) {
        NET_ERR("Register config callback failed.\n");
        return NULL;
    }

    SET_NET_EVENT(NET_EVENT_CONFIG, NULL);

    // 设备信息
    DevInfo devInfo[DEV_INFO_NUM];
    // 设备信息初始化
    memset_s(&devInfo, sizeof(devInfo), 0, sizeof(devInfo)); 
    devInfo[0].key = "productId";   // 设备Id
    devInfo[1].key = "sn";          // sn号
    devInfo[0].value = g_productId; // 设备Id
    devInfo[1].value = g_sn;        // sn号
    // 开始配置网络 SoftAP_Nan
    ret = StartNetCfg(devInfo, DEV_INFO_NUM, NETCFG_SOFTAP_NAN);
    if (ret != 0) {
        NET_ERR("Start config wifi fail.\n");
        return NULL;
    }

    return NULL;
}

// 网络配置注册任务
void NetCfgRegister(NetCfgEventCallback nEventCallback)
{
    NET_INFO("NetCfgRegister enter.");
    osThreadAttr_t attr;
    // 网络事件处理回调
    g_netCfgEventCallback = nEventCallback;

    // 创建任务
    attr.name = "NetCfgTask";
    attr.attr_bits = 0U;    // 线程属性位
    attr.cb_mem = NULL;     // 线程控制块内存
    attr.cb_size = 0U;      // 线程控制块内存大小
    attr.stack_mem = NULL;  // 线程堆栈内存
    attr.stack_size = NETCFG_TASK_STACK_SIZE;   // 线程堆栈大小
    attr.priority = NETCFG_TASK_PRIO;           // 线程优先级

    /**
     * 线程启动 
     * osThreadId_t osThreadNew(osThreadFunc_t	func, void *argument,const osThreadAttr_t *attr )	
     * 函数osThreadNew通过将线程添加到活动线程列表并将其设置为就绪状态来启动线程函数。
     * 线程函数的参数使用参数指针*argument传递。
     * 当创建的thread函数的优先级高于当前运行的线程时，创建的thread函数立即启动并成为新的运行线程。
     * 线程属性是用参数指针attr定义的。属性包括线程优先级、堆栈大小或内存分配的设置。
     * 可以在RTOS启动(调用 osKernelStart)之前安全地调用该函数，但不能在内核初始化 (调用 osKernelInitialize)之前调用该函数。
     * func	线程函数.
     * argument	作为启动参数传递给线程函数的指针
     * attr	线程属性
     */
    if (osThreadNew((osThreadFunc_t)NetCfgTask, NULL, &attr) == NULL) {
        NET_ERR("Falied to create NetCfgTask!\n");
    }
}
