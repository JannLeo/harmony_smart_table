#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_watchdog.h"
#include "wifiiot_pwm.h"

static int g_ledStates[3] = {0, 0, 0};
static int g_currentBright = 0;
static int g_beepState = 0;

static void *TrafficLightTask(const char *arg)
{
    (void)arg;

    printf("TrafficLightTask start!\r\n");
    WifiIotGpioIdx pins[] = {WIFI_IOT_GPIO_IDX_10, WIFI_IOT_GPIO_IDX_11, WIFI_IOT_GPIO_IDX_12};
    for (int i = 0; i < 4; i++) {
        for (unsigned int j = 0; j < 3; j++) {
            GpioSetOutputVal(pins[j], WIFI_IOT_GPIO_VALUE1);
            usleep(200*1000);

            GpioSetOutputVal(pins[j], WIFI_IOT_GPIO_VALUE0);
            usleep(100*1000);
        }
    }

    while (1) {
        for (unsigned int j = 0; j < 3; j++) {
            GpioSetOutputVal(pins[j], g_ledStates[j]);
        }
        if (g_beepState) {
            PwmStart(WIFI_IOT_PWM_PORT_PWM0, 20*1000, 40*1000);
        } else {
            PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        }
    }

    return NULL;
}

static void OnButtonPressed(char *arg)
{
    (void) arg;
    for (int i = 0; i < 3; i++) {
        if (i == g_currentBright) {
            g_ledStates[i] = 1;
        } else {
            g_ledStates[i] = 0;
        }
    }
    g_currentBright++;
    if (g_currentBright == 3) g_currentBright = 0;

    g_beepState = !g_beepState;
}

static void StartTrafficLightTask(void)
{
    osThreadAttr_t attr;

    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
        OnButtonPressed, NULL);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    WatchDogDisable();

    attr.name = "TrafficLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)TrafficLightTask, NULL, &attr) == NULL) {
        printf("[LedExample] Falied to create TrafficLightTask!\n");
    }
}

APP_FEATURE_INIT(StartTrafficLightTask);