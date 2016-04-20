#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h> 
#include <math.h>
#include "../timer.h"

struct timeval nowtime;
static void get_format_time(char *tstr)
{
    // time_t t;
    
    // t = time(NULL);
    // strcpy(tstr, ctime(&t));
    // tstr[strlen(tstr)-1] = '\0';
    long oldusec = nowtime.tv_usec;
    // struct timeval time;
    gettimeofday(&nowtime, NULL);
    sprintf(tstr, "usec: %ld ", nowtime.tv_usec - oldusec);
    
    return;
}


TMR_handle_t hdl[100], call_cnt[3] = {0};
uint16_t pfnCallback1(TMR_handle_t handle, void *arg)
{
    // char tstr[200];
    // static uint16_t i, ret;
    
    // get_format_time(tstr);
    // printf("hello %s: pfnCallback1 is here.\n", tstr);
    // if(i >= 30)
    // {
    //     get_format_time(tstr);
    //     ret = Timer_DelTimer(hdl[0]);
    //     printf("pfnCallback1: %s Timer_DelTimer::ret=%d\n", tstr, ret);
    // }
    // i++;
    printf("p1\n");
    call_cnt[1]++;
    call_cnt[0]++;
    
    
    return (1);
}

uint16_t pfnCallback2(TMR_handle_t handle,void * arg)
{
    // char tstr[200];
    // static uint16_t i, ret;
    
    // get_format_time(tstr);
    // printf("hello %s: pfnCallback2 is here.\n", tstr);
    // if(i >= 5)
    // {
    //     get_format_time(tstr);
    //     ret = Timer_DelTimer(hdl[1]);
    //     printf("pfnCallback2: %s Timer_DelTimer::ret=%d\n", tstr, ret);
    // }
    // i++;
    printf("p2\n");
    call_cnt[2]++;
    call_cnt[0]++;
    
    return (1);
}


int main(void)
{
    char arg[50];
    char tstr[200];
    int ret;
    
    TMR_Init();
    gettimeofday(&nowtime, NULL);
    hdl[0] = TMR_SetTimer(100, 1, pfnCallback1, NULL );
    //printf("hdl[0]=%d\n", hdl[0]);
    hdl[1] = TMR_SetTimer(100, 1, pfnCallback2,  NULL);
    // printf("hdl[1]=%d\n", hdl[1]);
    hdl[2] = TMR_SetTimer(100, 1, pfnCallback2, NULL );
    // printf("hdl[2]=%d\n", hdl[2]);
    // hdl[3] = TMR_SetTimer(10, pfnCallback1, NULL, );
    // hdl[4] = TMR_SetTimer(10, pfnCallback1, NULL,);
    // hdl[5] = TMR_SetTimer(10, pfnCallback1, NULL,);
    // hdl[6] = TMR_SetTimer(10, pfnCallback1, NULL,);
    // hdl[7] = TMR_SetTimer(10, pfnCallback1, NULL, 0);
    // hdl[8] = TMR_SetTimer(10, pfnCallback1, NULL, 0);
    // hdl[9] = TMR_SetTimer(10, pfnCallback1, NULL, 0);
    // hdl[10] = TMR_SetTimer(10, pfnCallback1, NULL, 0);
    // hdl[11] = TMR_SetTimer(10, pfnCallback1, NULL, 0);
    // hdl[12] = TMR_SetTimer(10, pfnCallback1, NULL, 0);
    // hdl[13] = TMR_SetTimer(10, pfnCallback1, NULL, 0);
    while(1)
    {
        if (call_cnt[1] >= 50)
        {               
            //TMR_PauseTimer(hdl[0]);
            //TMR_CancleTimer(hdl[0]);
            TMR_Shutdown();
            printf("p1 count :  %d\n", call_cnt[1]);
            printf("p2 count :  %d\n", call_cnt[2]);
            break;
        }
        if(call_cnt[0] >= 200)
        {
            // get_format_time(tstr);
            // ret = TMR_Shutdown();
            // printf("main: %s TMR_Shutdown, ret=%d\n", tstr, ret);
            // call_cnt++;
            printf("p1 count :  %d\n", call_cnt[1]);
            printf("p2 count :  %d\n", call_cnt[2]);
            break;
        }

    }
    uint32_t errornum = TMR_ReturnErrorNum();
    printf("%d, %ld\n", call_cnt[0], errornum, (float)errornum / call_cnt[0] *100);
    
    return 0;
}
