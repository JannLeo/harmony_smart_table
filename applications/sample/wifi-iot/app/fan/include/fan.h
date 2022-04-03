#ifndef __FAN_H__
#define __FAN_H__

#include <cmsis_os2.h>

#define FAN_TASK_STACK_SIZE      (1024*4)    // 任务栈大小
#define FAN_TASK_PRIO            (25)        // 任务优先级
#define FAN_LOOP_DELAY           (500)       // 延时500ms
#define TIMER_HALF_HOUR          30          // 半小时
#define TIMER_ONE_HOUR           60          // 一小时
#define TIMER_MAX_HOURS          8           // 最多8小时
#define TIMER_60_SECOND          60          // 60秒

#define TICKS_NUMBER    (100)  // 1 ticks = 10ms
#define MULTIPLE        (10)

#define BUF_SIZE        64

#define MESSAGE_LEN     6
#define MSG_VAL_LEN     2

typedef enum {
    MESSAGE_POWER_OFF = 1,
    MESSAGE_MODE = 3,
    MESSAGET_TIMER_SET = 5,
}MESSAGE_TYPE;

typedef enum {
    FAN_MODE_USER = 1,
    FAN_MODE_AUTO = 2,
    FAN_MODE_NBR
}FAN_MODE;

// 计算数组长度
#define ARRAYSIZE(a)    (sizeof((a)) / sizeof((a)[0]))

typedef union {
    char msg[MESSAGE_LEN + 1];
    struct {
        char type[MSG_VAL_LEN];
        char value1[MSG_VAL_LEN];
        char value2[MSG_VAL_LEN];
    } msg_segment;
} MsgInfo;

typedef struct {
    int type;
    void (*ProcessFunc)(int value1, int value2);
} MsgProcess;

typedef struct {
    uint8 power_off;        // 开关
    uint8 mode;             // 模式
    boolean timer_flag;     // 定时标志位
    uint8 timer_hour;       // 时间
    uint8 timer_mins;       // 分钟
    uint32 timer_count;     // 秒
    osTimerId_t timerID;    // 定时器ID
}FanInfo;

#endif  /* __FAN_H__ */
