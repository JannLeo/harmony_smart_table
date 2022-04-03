#ifndef __NETWORK_CONFIG_SERVICE_H__
#define __NETWORK_CONFIG_SERVICE_H__

//SoftAp 参数长度
#define SSID_MAX_LEN 32
#define PWD_MAX_LEN 64

//WiFi 参数长度
#define WIFI_SSID_MAX_LEN 32
#define WIFI_PWD_MAX_LEN 64
#define WIFI_PSK_LEN 32
#define WIFI_BSSID_LEN 6

typedef enum {
    WIFI_PARIWISE_UNKNOWN = 0, // UNKNOWN
    WIFI_PARIWISE_AES,         // AES加密 WPA2
    WIFI_PARIWISE_TKIP,        // 临时密钥 WPA
    WIFI_PARIWISE_TKIP_AES_MIX // 混合加密 WPA/WPA2
}WifiPairwise;

// Wifi认证模式
typedef enum {
    WIFI_SECURITY_OPEN = 0,    // 开放
    WIFI_SECURITY_WEP,         
    WIFI_SECURITY_WPA2PSK,     
    WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
    WIFI_SECURITY_WPAPSK,      
    WIFI_SECURITY_WPA,         
    WIFI_SECURITY_WPA2,
    WIFI_SECURITY_SAE,         // WAP3
    WIFI_SECURTIY_UNKNOWN
}WifiAuthMode;

// 网络配置方式
typedef enum {
    NETCFG_SOFTAP = 0,        // softap
    NETCFG_SOFTAP_NAN,        // softap+nan混合模式
    NETCFG_BUTT,              // 结尾
}NetCfgType;

// 网络配置错误码
typedef enum {
    NETCFG_OK = 0,                      // 配网成功
    NETCFG_ERROR = -1,                  // 配网失败
    NETCFG_DEV_INFO_INVALID = -2,       // 设备信息无效
    NETCFG_SOFTAP_PARAM_INVALID = -3,   // 热点参数无效
    NETCFG_MODE_INVALID = -4,           // 模式无效
    NETCFG_POWER_INVALID = -5,          // 功率无效
}NetworkCfgErrorCode;

// Wifi信息源
typedef enum {
    WIFI_INFO_FROM_NAN = 0,             // NAN
    WIFI_INFO_FROM_SOFTAP,              // SOFTAP
}WifiInfoSource;

// 网络配置状态
typedef enum {
    NETCFG_IDENTIFY_DISABLE = 0,        // 关闭网络配置
    NETCFG_IDENTIFY_ENABLE = 1,         // 开启网络配置
    NETCFG_WORKING = 2                  // 网络配置正在运行
}NetCfgStatus;

// 热点参数
typedef struct {
    char ssid[SSID_MAX_LEN + 1];    //ap名
    char password[PWD_MAX_LEN + 1]; //ap密码
    int authType;   //认证方式
    int pairwise;   //加密方式
    int isHidden;   //是否隐藏
    int channelNum; //频段
}SoftAPParam;

// 设备参数
typedef struct {
    const char *key;    
    const char *value;
}DevInfo;

// Wifi参数
typedef struct {
    unsigned char ssid[WIFI_SSID_MAX_LEN + 1];  //wifi名
    unsigned char pwd[WIFI_PWD_MAX_LEN + 1];    //wifi密码
    unsigned char psk[WIFI_PSK_LEN + 1];        //密钥
    unsigned char bssid[WIFI_BSSID_LEN + 1];    //mac 地址
    unsigned char ssidLen;                      //wifi名长度
    unsigned char pwdLen;                       //wifi密码长度
    unsigned char pskLen;                       //密钥长度
    unsigned char bssidLen;                     //mac长度
    int authMode;                               //认证方式
    int wifiInfoSrc;                            //WiFi信息
    int channelNumber;                          //频段
}WifiInfo;

typedef struct {
    // 获取设备的 PIN 代码。PIN 码的最大长度为 16 字节。
    int (*GetPinCode)(unsigned char *pinCode, unsigned int size, unsigned int *len);
    // 在网络配置过程中处理 Wi-Fi 信息和供应商数据
    int (*ParseNetCfgData)(const WifiInfo *wifiInfo, const unsigned char *vendorData, unsigned int vendorDataLen);
    // 处理应用控制数据。
    int (*RecvRawData)(const char *svcId, unsigned int mode, const char *data);
    /**
      * 通知网络配置服务状态。 
      * 例如，当设备处于NAN模式时，可以调用此功能来控制LED的闪烁或蜂鸣器用于通知。
      * 状态表示网络配置状态。 
      * 0: 表示停止蜂鸣或LED闪烁。 
      * 1: 表示开始蜂鸣或LED闪烁。 
      * 2: 表示设备处于网络配置状态。
      */ 
    void (*NotifyNetCfgStatus)(NetCfgStatus status);
    /**
      * 断开网络配置。 
      * 此功能用于在网络配置过程中发生内部错误时通知应用程序或收到来自手机的断开连接请求。
      * errCode值如下：
      * 0 ：应用程序无法接收网络配置数据，网络配置服务会自动启动。 
      * 1 ：已将网络配置数据通知应用程序，需要确定是停止还是重新启动网络配置服务。 
      */ 
    void (*OnDisconnect)(int errCode);
}NetCfgCallback;

/**
 * 设置超短距离传输的传输通道功率
 * power 表示超短距离传输的功率 单位dBm。
 * 取值范围：
 * -70、-67、-65、-63、-61、-58、-55、-42、-48、-45、-42
 * 为了安全，请确保产品表面(包括外部天线)上的任何点的无线传输功率不超过 -65 dBm。
 */ 
int SetSafeDistancePower(signed char power);

// 设置热点参数
int SetSoftAPParameter(const SoftAPParam *param);

// 注册网络配置回调，请在启用网络配置服务之前，调用此功能
int RegNetCfgCallback(const NetCfgCallback *callback);

/**
 * 开始网络配置
 * 默认情况下，侦听具有多播地址238.238.238.238和端口号5683的数据包。
 * 注意：在NAN模式下，成功启动SoftAP后调用此函数。
 */ 
int StartNetCfg(const DevInfo *devInfoList, unsigned int listSize, NetCfgType mode);

// 停止网络配置
int StopNetCfg(void);

// 通知网络配置结果
int NotifyNetCfgResult(signed int result);

// 发送数据(字符串类型)
int SendRawData(const char *data);

#endif //__NETWORK_CONFIG_SERVICE_H__