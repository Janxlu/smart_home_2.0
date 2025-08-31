#ifndef __TIMECONV_H
#define __TIMECONV_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#define FOURYEARDAY (((uint32_t)365+365+365+366))  //4年一个周期内的总天数（1970~2038不存在2100这类年份，故暂不优化）
#define TIMEZONE    (8)                //北京时区调整 



typedef struct rtc_time_struct
{
    uint16_t ui8Year;       // 1970~2038
    uint8_t ui8Month;       // 1~12
    uint8_t ui8DayOfMonth;  // 1~31
    uint8_t ui8Week;
    uint8_t ui8Hour;        // 0~23
    uint8_t ui8Minute;      // 0~59
    uint8_t ui8Second;      // 0~59
   
}rtc_time_t;

uint8_t isLeapYear(uint16_t year);
void covUnixTimeStp2Beijing(uint32_t unixTime, rtc_time_t *tempBeijing);
uint32_t covBeijing2UnixTimeStp(rtc_time_t *beijingTime);

#endif /*__TIMECONV_H*/

