#include "table_app.h"
#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

static void beep_oo(void)
{
    osDelay(300);
    printf("beep press start!\r\n");
    beep_on(); //按键蜂鸣一声
    // printf("beep one start!!\r\n");
    // beep_only(); //蜂鸣一声
    // printf("led press start!!\r\n");
    // led_app(); //按键亮灯
    printf("5\r\n");
}

APP_FEATURE_INIT(beep_oo);