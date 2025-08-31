#ifndef  __CJSONPARSE_H
#define  __CJSONPARSE_H  
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

//天气存储结构体
typedef struct WeatherStruct{
	uint8_t low[3];     //最近三天的最低温度
    uint8_t high[3];    //最近三天的最高温度
    uint8_t humi[3];    //最近三天的湿度
    uint8_t code[3];    //最近三天的天气代码
    uint8_t nowtemp;    //现在的天气温度情况
    uint8_t nowcode;    //现在的天气代码
} weather;

//解析现在的天气JSON数据
int cJSON_NowWeatherParse(char *JSON,weather *Weather);

//解析最近三天天气JSON数据
int cJSON_TayWeatherParse(char *JSON,weather *Weather);

#endif /*__CJSONPARSE_H*/
