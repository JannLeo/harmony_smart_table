#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "securec.h"
#include "ohos_types.h"
#include "cmsis_os2.h"
#include "netcfg.h"
#include "led.h"
#include "defines.h"
#include "fan.h"
#include "network_config_service.h"
#include "E53_IA1.h"

E53_IA1_Data_TypeDef E53_IA1_Data;

static FanInfo g_fan;
static boolean g_netstatus = FALSE;

static void ShowPowerStatus(void)
{
    printf("power %s.\n", g_fan.power_off == 1 ? "OFF" : "ON");
}

static void ShowNetStatus(void)
{
    printf("net %s.\n", g_netstatus ? "connectted" : "disconnect");
}

static void ShowPowerOffTimer(void)
{
    char timer_buf[BUF_SIZE] = {0};
    if (g_fan.timer_flag) {
        if (sprintf_s(timer_buf, BUF_SIZE, "%02dH %02dM", g_fan.timer_hour, g_fan.timer_mins) < 0) {
            FAN_ERR("sprintf_s failed!\n");
            return;
        }
    } else {
        if (sprintf_s(timer_buf, BUF_SIZE, "OFF") < 0) {
            FAN_ERR("sprintf_s failed!\n");
            return;
        }
    }
    printf("Timer %s.\n", timer_buf);
}

static void FanShowInfo(void)
{
    ShowPowerStatus();
    ShowNetStatus();
    ShowPowerOffTimer();
}

static void FanDealTimer(int value1, int value2)
{
    if (g_fan.timer_hour != value1 || g_fan.timer_mins != value2) {
        g_fan.timer_hour = value1;
        g_fan.timer_mins = value2;

        if (g_fan.timer_hour == 0 && g_fan.timer_mins == 0) {
            g_fan.timer_flag = 0;
        } else {
            g_fan.timer_flag = 1;
            g_fan.timer_count = 0;
        }
        ShowPowerOffTimer();
    }
}

static void FanDealPoweroff(int value1, int value2)
{
    (void)value2;
    if (value1 == 0) {
        g_fan.power_off = 1;
        Light_StatusSet(OFF);
        Motor_StatusSet(OFF);
    } else {
        g_fan.power_off = 0;
        Light_StatusSet(ON);
        Motor_StatusSet(ON);
    }
    ShowPowerStatus();
}

static int fan_atoi(char *str, int length)
{
    char buf[MESSAGE_LEN] = {0};
    if (length >= MESSAGE_LEN) {
        FAN_ERR("invliad length!\n");
        return -1;
    }

    if (strncpy_s(buf, MESSAGE_LEN, str, length) != 0) {
        FAN_ERR("strncpy_s failed!\n");
        return 0;
    }

    return atoi(buf);
}

static void FanDealMode(int value1, int value2)
{
    (void)value2;
    if (g_fan.mode != value1) {
        g_fan.mode = value1;
    }
}

static MsgInfo g_msgInfo;
static MsgProcess g_msgProcess[] = {
    {MESSAGE_POWER_OFF, FanDealPoweroff},    // ??????????????????
    {MESSAGE_MODE, FanDealMode},             // ????????????
    {MESSAGET_TIMER_SET, FanDealTimer}      // ????????????????????????
};

static void FanProcessAppMessage(const char *data, int data_len)
{
    if (data_len != MESSAGE_LEN) {
        FAN_ERR("data len invalid!\n");
        return;
    }

    if (strncpy_s(g_msgInfo.msg, MESSAGE_LEN + 1, data, data_len) != 0) {
        FAN_ERR("strncpy_s failed!\n");
        return;
    }

    for (uint8 i = 0; i < ARRAYSIZE(g_msgProcess); i++) {
        if (g_msgProcess[i].type == fan_atoi(g_msgInfo.msg_segment.type, MSG_VAL_LEN)) {
            g_msgProcess[i].ProcessFunc(fan_atoi(g_msgInfo.msg_segment.value1, MSG_VAL_LEN),
                                        fan_atoi(g_msgInfo.msg_segment.value2, MSG_VAL_LEN));
        }
    }
}

// ??????????????????
static int FanNetEventHandler(NET_EVENT_TYPE event, void *data)
{
    switch (event) {
        case NET_EVENT_CONNECTTED:                                      // ??????????????????
            g_netstatus = TRUE;
            ShowNetStatus();                                            // ?????????????????????
            break;
        case NET_EVENT_RECV_DATA:                                       // ?????????????????????(FA???????????????)
            FanProcessAppMessage((const char *)data, strlen(data));     // ?????????????????????
            break;
        case NET_EVENT_SEND_DATA:
            SendRawData((char *) data);
            break;
        case NET_EVENT_CONFIG_FAIL:
            FAN_ERR("Wifi config fail!\n");
            break;
        default:
            break;
    }
    return 0;
}

// ?????????????????????
static void FanTimerHandler(void *arg)
{
    (void)arg;

    if (g_fan.timer_flag) {
        g_fan.timer_count++;
        if (g_fan.timer_count >= (uint32)((g_fan.timer_hour * (TIMER_60_SECOND * TIMER_60_SECOND)) +
            g_fan.timer_mins * TIMER_60_SECOND)) {
            g_fan.power_off = 1;
        }
    } else {
        g_fan.timer_count = 0;
    }
}

// ???????????????
static void FanStartTimer(void)
{
    // ??????????????????????????????
    g_fan.timerID = osTimerNew(&FanTimerHandler, osTimerPeriodic, NULL, NULL);
    // ??????????????? 1???????????????
    osTimerStart(g_fan.timerID, TICKS_NUMBER);
}

// ???????????????
static void FanInit(void)
{
    // ???????????????
    E53_IA1_Init();
    // LED?????????
    LedInit();
}

static void FanLoop(void)
{
    while (1) {
        if (g_fan.power_off) {
            Light_StatusSet(OFF);
            Motor_StatusSet(OFF);
        }
        // ????????????????????????(???????????????????????????????????????)
        if (g_fan.mode == FAN_MODE_AUTO) {     
            E53_IA1_Read_Data();
            // ??????????????????
            float temp = E53_IA1_Data.Temperature;    
            printf("temp = %.1f.", temp);
            if(temp > 30) {
                Motor_StatusSet(ON);
            }else {
                Motor_StatusSet(OFF);
            }
        }
        osDelay(FAN_LOOP_DELAY);
    }
}

static void *FanTask(const char *arg)
{
    (void)arg;
    FAN_DEBUG("FanTask Enter!");
    FanInit();
    (void)memset_s(&g_fan, sizeof(g_fan), 0x00, sizeof(g_fan));
    g_fan.mode = FAN_MODE_USER;                       // ?????????????????????????????????
    g_fan.power_off = 1;

    FanStartTimer();                                  // ??????????????????
    NetCfgRegister(FanNetEventHandler);               // ?????????????????????????????????????????????
    FanShowInfo();                                    // ????????????????????????

    FanLoop();
}

void FanDemoEntry(void)
{
    osThreadAttr_t attr;
    attr.name = "FanTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = FAN_TASK_STACK_SIZE;
    attr.priority = FAN_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)FanTask, NULL, &attr) == NULL) {
        FAN_ERR("Falied to create FanTask!\n");
    }
}

SYS_RUN(FanDemoEntry);
