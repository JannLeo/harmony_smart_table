#include <hi_types_base.h>
#include "hi_stdlib.h"
#include <hi_time.h>
#include "stdio.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_reset.h"
#include "SmartTable_Reset.h"

bool SmartTable_Reset(hi_sys_reboot_cause cause){
    switch (cause)
    {
    case HI_SYS_REBOOT_CAUSE_TEMP:
        hi_hard_reboot(cause);
        break;
    case HI_SYS_REBOOT_CAUSE_MEMORY:
    case HI_SYS_REBOOT_CAUSE_WIFI_MODE:
        hi_soft_reboot(cause);
        break;
    default:
        hi_hard_reboot(cause);
        return 0;
    }
    return 1;

}