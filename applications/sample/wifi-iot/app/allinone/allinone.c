#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <table_app.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include <wifi_apsta.h>
#include <mqtt_test.h>

#include "nfc_ljn.h"

#define I2C_TASK_STACK_SIZE 1024 * 8
#define I2C_TASK_PRIO 25
osSemaphoreId_t sem1; //信号量变量

/*****任务一：按键功能，蜂鸣亮灯等*****/
void thread1(void)
{
    while (1)
    {
        osSemaphoreAcquire(sem1, osWaitForever);
        int sum = 0;
        printf("This is BearPi Harmony Thread1----%d\r\n", sum++);
        led_app();
        beep_only();
        I2CTask();
        usleep(1000000);
        //此处若只申请一次信号量，则Thread_Semaphore2和Thread_Semaphore3会交替运行。
        osSemaphoreRelease(sem1);
    }
}

/*****任务二：建立AP与STA并配网*****/
void thread2(void)
{
    int sum = 0;
    //等待sem1信号量
    osSemaphoreAcquire(sem1, osWaitForever);
    printf("This is BearPi Harmony Thread2----%d\r\n", sum++);
    WifiHotspotTask();
    usleep(500000);
    printf("Thread2 End!----\r\n");
    osSemaphoreRelease(sem1);
}

/*****任务三：mqtt*****/
void thread3(void)
{
    // while(1){
    int sum = 0;
    //等待sem1信号量
    osSemaphoreAcquire(sem1, osWaitForever);
    printf("This is BearPi Harmony Thread3----%d\r\n", sum++);
    mqtt_test();
    usleep(500000);
    osSemaphoreRelease(sem1);
    // }
}

/*****任务四：nfc*****/
void thread4(void)
{
    int sum = 0;
    // while(1){
    osSemaphoreAcquire(sem1, osWaitForever);
    printf("This is BearPi Harmony Thread4----%d\r\n", sum++);
    I2CTask();
    usleep(500000);
    osSemaphoreRelease(sem1);
    // }
}

/*****任务创建*****/
static void Thread_example(void)
{
    osThreadAttr_t attr;

    attr.name = "thread1";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 20480;
    attr.priority = 25;

    if (osThreadNew((osThreadFunc_t)thread1, NULL, &attr) == NULL)
    {
        printf("Falied to create thread1!\n");
    }

    attr.name = "thread2";

    if (osThreadNew((osThreadFunc_t)thread2, NULL, &attr) == NULL)
    {
        printf("Falied to create thread2!\n");
    }
    attr.name = "thread3";

    if (osThreadNew((osThreadFunc_t)thread3, NULL, &attr) == NULL)
    {
        printf("Falied to create thread3!\n");
    }
    attr.name = "thread4";
    attr.stack_size = I2C_TASK_STACK_SIZE;
    if (osThreadNew((osThreadFunc_t)thread4, NULL, &attr) == NULL)
    {
        printf("Falied to create thread4!\n");
    }
    sem1 = osSemaphoreNew(4, 0, NULL);
    if (sem1 == NULL)
    {
        printf("Falied to create Semaphore1!\n");
    }
    osSemaphoreRelease(sem1);
}

APP_FEATURE_INIT(Thread_example);
// APP_RUN(Thread_example);