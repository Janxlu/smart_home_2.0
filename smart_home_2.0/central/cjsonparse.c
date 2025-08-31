#include "cjsonparse.h"
#include "ohos_init.h"
#include "cmsis_os2.h"

//字符型转换为整型
static uint8_t str2int(char *str)
{
    uint8_t len,res = 0;
    len = strlen((const char *)str);
    switch(len)
    {
    case 1:
        res = str[0]-0x30;
        break;
    case 2:
        res = (str[0]-0x30)*10+(str[1]-0x30);
        break;
    default:
        break;
    }
    return res;
}

//解析现在的天气Json数据
int cJSON_NowWeatherParse(char *JSON,weather *Weather)
{
    cJSON *json,*arrayItem,*object,*subobject,*item;

    json = cJSON_Parse(JSON); 
    if(json == NULL)      
    {
        printf("Error before: [%s]\n",cJSON_GetErrorPtr()); 
        return 1;
    }
    else
    {
        if((arrayItem = cJSON_GetObjectItem(json,"results")) != NULL)
        {
            cJSON_GetArraySize(arrayItem);    

            if((object = cJSON_GetArrayItem(arrayItem,0)) != NULL)
            {
                
                if((subobject = cJSON_GetObjectItem(object,"location")) != NULL)
                {

                }
                
                if((subobject = cJSON_GetObjectItem(object,"now")) != NULL)
                {
                    printf("---------------------------------now-------------------------------\n");
                    if((item = cJSON_GetObjectItem(subobject,"text")) != NULL)
                    {
                        printf("%s : %s\n",item->string,item->valuestring);
                    }
                    if((item = cJSON_GetObjectItem(subobject,"code")) != NULL)
                    {
                        printf("%s : %s\n",item->string,item->valuestring);
                        Weather->nowcode = str2int(item->valuestring);
                    }
                    if((item = cJSON_GetObjectItem(subobject,"temperature")) != NULL)
                    {
                        printf("%s : %s\n",item->string,item->valuestring);
                        Weather->nowtemp = str2int(item->valuestring);
                    }
                }
                if((subobject = cJSON_GetObjectItem(object,"last_update")) != NULL)
                {
                    printf("----------------------------last_update----------------------------\n");
                    printf("%s : %s\n\n",subobject->string,subobject->valuestring);
                }
            }
        }
    }
    cJSON_Delete(json); 
    return 0;
}

//解析最近三天的天气Json数据
int cJSON_TayWeatherParse(char *JSON,weather *weather)
{
    cJSON *root;
    cJSON *pSub;
    cJSON *arrayItem;
    cJSON *pItem;
    cJSON *pSubItem;
    cJSON *pChildItem;
    cJSON *pLastItem;
    char *pr;
    root = cJSON_Parse((const char *)JSON);
    printf("root parse1 \n");
    if(root != NULL)
    {
        printf("root \n");
        pSub = cJSON_GetObjectItem(root,"results");
        if(pSub != NULL)
        {
            printf("psub \n");
            arrayItem = cJSON_GetArrayItem(pSub,0);
            pr = cJSON_Print(arrayItem);
            pItem = cJSON_Parse(pr);
            if(pItem != NULL)
            {
                printf("pitem \n");
                pSubItem = cJSON_GetObjectItem(pItem,"daily");
                if(pSubItem != NULL)
                {
                    printf("psubitem \n");
                    int size = cJSON_GetArraySize(pSubItem);
                    for(int i=0; i<size; i++)
                    {
                        if(i==3)break;
                        arrayItem = cJSON_GetArrayItem(pSubItem,i);
                        pr = cJSON_Print(arrayItem);
                        pLastItem = cJSON_Parse(pr);
                        if(pLastItem != NULL)
                        {
                            printf("plastitem \n");
                            if((pChildItem =  cJSON_GetObjectItem(pLastItem,"high")) != NULL)
                            {
                                printf("%s : %s\n",pChildItem->string,pChildItem->valuestring);
                                weather->high[i] = str2int(pChildItem->valuestring);
                                printf("weather.high %d",i,weather->high[i]);
                            }

                            if((pChildItem =  cJSON_GetObjectItem(pLastItem,"low")) != NULL)
                            {
                                printf("%s : %s\n",pChildItem->string,pChildItem->valuestring);
                                weather->low[i] = str2int(pChildItem->valuestring);
                                printf("weather.low %d",i,weather->low[i]);
                            }

                            if((pChildItem =  cJSON_GetObjectItem(pLastItem,"code_day"))!=NULL)
                            {
                                printf("%s : %s\n",pChildItem->string,pChildItem->valuestring);
                                weather->code[i] = str2int(pChildItem->valuestring);
                            }
                            if((pChildItem =  cJSON_GetObjectItem(pLastItem,"humidity"))!=NULL)
                            {
                                printf("%s : %s\n",pChildItem->string,pChildItem->valuestring);
                                weather->humi[i] = str2int(pChildItem->valuestring);
                                printf("weather.humi %d",i,weather->humi[i]);
                            }
                        }
                        cJSON_Delete(pLastItem);
                    }
                }
            }
            cJSON_Delete(pItem);
        }
    }
    cJSON_Delete(root);

    return 0;
}