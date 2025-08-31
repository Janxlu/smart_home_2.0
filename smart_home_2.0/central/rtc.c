#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
uint32_t timedata=1608362368;   //时间戳默认值

static void RtcTask(void *arg)
{
    (void)arg;
    while(1)
    {
        //每隔一秒，时间戳加1
        timedata++;
        sleep(1);
    }
}

static void RtcTaskHandle(void)
{
    osThreadAttr_t attr;
    attr.name = "RtcTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 512;
    attr.priority = osPriorityNormal;

    if (osThreadNew(RtcTask, NULL, &attr) == NULL) {
        printf("[RtcTaskHandle] Falied to create KeyTask!\n");
    }
}
APP_FEATURE_INIT(RtcTaskHandle);