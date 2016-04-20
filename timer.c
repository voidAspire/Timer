
/**********************************************************************************************************
 *  A. Standard Includes
*********************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>


/**********************************************************************************************************
 *  B. APP sepecific Includes
*********************************************************************************************************/

#include "timer.h"



/**********************************************************************************************************
 *  C. Object like Macro
*********************************************************************************************************/
#define TMR_CFG_DEBUG_TIMER 1

#define TMR_CFG_MUL_TIMER_RESET_SEC     (0u)
#define TMR_CFG_MUL_TIMER_RESET_USEC    (10000u)
#define TMR_CFG_MUL_TIMER_FIRST_SEC     (TMR_CFG_MUL_TIMER_RESET_SEC)
#define TMR_CFG_MUL_TIMER_FIRST_USEC    (TMR_CFG_MUL_TIMER_RESET_USEC)
#define TMR_CFG_MUL_TIMER_LEAST_PRECISE_MS  (TMR_CFG_MUL_TIMER_FIRST_SEC * 1000 + TMR_CFG_MUL_TIMER_RESET_USEC / 1000)


/**********************************************************************************************************
 *  D. Function like Macro
*********************************************************************************************************/

/**********************************************************************************************************
 *  E. Type defines
**********************************************************************************************************/

/* move this struct to the module internal*/
typedef struct tmr_zStatus_t
{
    struct _timer_info
    {
        TMR_eState_t        eState; //   the current and old state of timer 
        TMR_ePerodicity_t   ePerodicity; // the ePerodicity of timer, throwaway or periodic    
        uint32_t            intervalMsec;//  the times of least timer
        uint32_t            elapsedMsec; //    0 ~ interval, auto incremented with every least time gone

        TMR_pfnNotifier_t   pfnCallback; // timer callback function;
        void *              contxt; // callback function's param

        #ifdef TMR_CFG_DEBUG_TIMER
        //Test timer accuracy, just for testing
        struct timeval      beginTime; 
        struct timeval      endTime;
        #endif
        
    }TMR_info_t[ TMR_CFG_MAX_TIMER_CNT ];

    //__sighandler_t old_sigfunc;
    void (* old_sigfunc)();
    void (* new_sigfunc)();
    struct itimerval value; //the current and old least time of timer
    struct itimerval ovalue;
    
    bool_t bModuleTmrStarted;
    
}   tmr_zStatus_t;

#ifdef TMR_CFG_DEBUG_TIMER
//Test timer accuracy, just for testing
uint32_t errorNum = 0;

uint32_t TMR_ReturnErrorNum()
{
    return errorNum;
}
#endif

/**********************************************************************************************************
 *  F. Local Function Declarations
**********************************************************************************************************/
static void tmr_SignalFun();

/**********************************************************************************************************
 *  G. Local Object/Variable
*********************************************************************************************************/
static tmr_zStatus_t tmr_zStatus;

/**********************************************************************************************************
 *  h. Exported Object/Variable
*********************************************************************************************************/
 
/**********************************************************************************************************
 *  I. Local Function Implementations
**********************************************************************************************************/

/*********************************************************************************************************
 * @brief     Minimum scale timer response function.
 *                       Traversing the timer list, detect whether the timer expires
 *                           The callback function is triggered corresponding maturity
 *                           otherwise do nothing
 *
 * @param   
 * @usage
 *
 * @return  
 *
****************************************************************************************************/
static void tmr_SignalFun()
{
	static uint32_t myCount = 0uL;
    //printf(" Entered %ld\n", myCount++);

    uint16_t i;
    for( i = 0; i < TMR_CFG_MAX_TIMER_CNT; i++ )
    {
        if (tmr_zStatus.TMR_info_t[i].eState == TMR_eStateActivated)
        {
            tmr_zStatus.TMR_info_t[i].elapsedMsec += TMR_CFG_MUL_TIMER_LEAST_PRECISE_MS;
            if(tmr_zStatus.TMR_info_t[i].elapsedMsec >= tmr_zStatus.TMR_info_t[i].intervalMsec)
            {
                #ifdef TMR_CFG_DEBUG_TIMER
                 //printf("%d elapsedMsec: %d    intervalMsec: %d\n", i, tmr_zStatus.TMR_info_t[i].elapsedMsec, tmr_zStatus.TMR_info_t[i].intervalMsec);
                //just for testing
                gettimeofday(&tmr_zStatus.TMR_info_t[i].endTime, NULL);

                //printf("msec: %ld\n", ( tmr_zStatus.TMR_info_t[i].endTime.tv_usec - tmr_zStatus.TMR_info_t[i].beginTime.tv_usec + ( tmr_zStatus.TMR_info_t[i].endTime.tv_sec - tmr_zStatus.TMR_info_t[i].beginTime.tv_sec ) * 100000 ) / 1000 );

                if ( abs(100 - ( ( tmr_zStatus.TMR_info_t[i].endTime.tv_usec - tmr_zStatus.TMR_info_t[i].beginTime.tv_usec + 
                            ( tmr_zStatus.TMR_info_t[i].endTime.tv_sec - tmr_zStatus.TMR_info_t[i].beginTime.tv_sec ) * 100000 ) / 1000) ) >=2)
                   if (( tmr_zStatus.TMR_info_t[i].endTime.tv_usec - tmr_zStatus.TMR_info_t[i].beginTime.tv_usec + 
                                ( tmr_zStatus.TMR_info_t[i].endTime.tv_sec - tmr_zStatus.TMR_info_t[i].beginTime.tv_sec ) * 100000 ) / 1000 > 0)
                    errorNum++;
                gettimeofday(&tmr_zStatus.TMR_info_t[i].beginTime, NULL);
                #endif

                if (tmr_zStatus.TMR_info_t[i].ePerodicity == TMR_eProdicitySingleShot )
                {
                    tmr_zStatus.TMR_info_t[i].eState = TMR_eStateExpired;
                }
                
                if( tmr_zStatus.TMR_info_t[i].pfnCallback )
                {
                    tmr_zStatus.TMR_info_t[i].pfnCallback( i, tmr_zStatus.TMR_info_t[i].contxt);
                }
                
                tmr_zStatus.TMR_info_t[i].elapsedMsec = 0uL;
            }    
        }
    }

    //printf(" Exited \n");
}


/**********************************************************************************************************
 *  J. Public Function Implementations
*********************************************************************************************************/

/*********************************************************************************************************
 * @brief      init timer configuration
                    -set least timer
                    -set signal fun of sigalrm
 *
 * @param   
 * @usage
 *
 * @return  TRUE if no error. 
 *          FALSE otherwise.
 * 
****************************************************************************************************/
bool_t TMR_Init(void)
{
    uint16_t ret;

    memset(&tmr_zStatus, 0, sizeof(struct tmr_zStatus_t));
    if ( (tmr_zStatus.old_sigfunc = signal(SIGALRM, tmr_SignalFun)) != SIG_ERR)
    {
        tmr_zStatus.new_sigfunc = tmr_SignalFun;
    
        tmr_zStatus.value.it_value.tv_sec = TMR_CFG_MUL_TIMER_FIRST_SEC;
        tmr_zStatus.value.it_value.tv_usec = TMR_CFG_MUL_TIMER_FIRST_USEC;
        tmr_zStatus.value.it_interval.tv_sec = TMR_CFG_MUL_TIMER_RESET_SEC;
        tmr_zStatus.value.it_interval.tv_usec = TMR_CFG_MUL_TIMER_RESET_USEC;
        if ( !(ret = setitimer(ITIMER_REAL, &tmr_zStatus.value, &tmr_zStatus.ovalue)) )
        {
            tmr_zStatus.bModuleTmrStarted = TRUE;
        }
    }

    return tmr_zStatus.bModuleTmrStarted;
}

/*********************************************************************************************************
 * @brief    shutdown timer module
 *
 * @param
 * @usage
 *
 * @return  TRUE if no error. 
 *          FALSE otherwise.
 * 
****************************************************************************************************/
bool_t TMR_Shutdown(void)
{
    bool_t   bRet = FALSE;
    
    if( tmr_zStatus.bModuleTmrStarted )
    {
        if( ( signal( SIGALRM, tmr_zStatus.old_sigfunc )) != SIG_ERR)
        {
            if ( !( setitimer(ITIMER_REAL, &tmr_zStatus.ovalue, &tmr_zStatus.value)) )
            {
                memset(&tmr_zStatus, 0, sizeof(struct tmr_zStatus_t ));
                bRet = TRUE;
            }
        }
    }
   
    return bRet;
}

/*********************************************************************************************************
 * @brief     destroy a timer
 *
 * @param   handle :  number of timer that needs to be destroyed
 * @usage
 *
 * @return  the remaining time.
 *          0 if timer is not valid, or expired.
 * 
****************************************************************************************************/
uint32_t TMR_CancleTimer( TMR_handle_t handle)
{
    uint32_t remainingtime = 0uL;
    
    if( tmr_zStatus.bModuleTmrStarted )
    {
        if(handle >= 0 && handle <= TMR_CFG_MAX_TIMER_CNT)
        {
            remainingtime = tmr_zStatus.TMR_info_t[handle].elapsedMsec;
            memset(&tmr_zStatus.TMR_info_t[handle], 0, sizeof(tmr_zStatus.TMR_info_t[handle]));
        }
    }
    
    return remainingtime;
}

/*********************************************************************************************************
 * @brief     set a timer of custom
 *
 * @param   uIntervalMsec: time in msec
 * @param   bIsPeriodic:    if the timer is periodic.
 * @param   pfnCallback :  timer expires response function
 * @usage
 *
 * @return  timer handle if no error.
            TMR_CFG_INVALID_HANDLE otherwise.
 *
 * 
****************************************************************************************************/
TMR_handle_t TMR_SetTimer(uint32_t uIntervalMsec, bool_t bIsPeriodic, TMR_pfnNotifier_t pfnCallback, void *contxt)
{
    TMR_handle_t    handle = TMR_CFG_INVALID_HANDLE;
    uint16_t        i;

    if( tmr_zStatus.bModuleTmrStarted )
    {

        if(( pfnCallback != NULL ) && ( uIntervalMsec > 0))
        {   

            for(i = 0; i < TMR_CFG_MAX_TIMER_CNT; i++)
            {
                if(tmr_zStatus.TMR_info_t[i].eState != TMR_eStateInvalid)
                {
                    continue;
                }

                memset(&tmr_zStatus.TMR_info_t[i], 0, sizeof(tmr_zStatus.TMR_info_t[i]));
                tmr_zStatus.TMR_info_t[i].pfnCallback = pfnCallback;
                tmr_zStatus.TMR_info_t[i].contxt = contxt;
                if (uIntervalMsec % TMR_CFG_MUL_TIMER_LEAST_PRECISE_MS > 0)
                {
                    tmr_zStatus.TMR_info_t[i].intervalMsec = ( uIntervalMsec / TMR_CFG_MUL_TIMER_LEAST_PRECISE_MS + 1 ) * TMR_CFG_MUL_TIMER_LEAST_PRECISE_MS;
                }
                else
                {
                    tmr_zStatus.TMR_info_t[i].intervalMsec = uIntervalMsec;
                }
                tmr_zStatus.TMR_info_t[i].elapsedMsec = 0uL;
                tmr_zStatus.TMR_info_t[i].eState = TMR_eStateActivated;
                if (bIsPeriodic)
                {
                    tmr_zStatus.TMR_info_t[i].ePerodicity = TMR_eProdicityMultiple;
                }
                else
                {
                    tmr_zStatus.TMR_info_t[i].ePerodicity = TMR_eProdicitySingleShot;
                }

                #ifdef TMR_CFG_DEBUG_TIMER
                //just for testing
                gettimeofday(&tmr_zStatus.TMR_info_t[i].beginTime, NULL);
                #endif
                
                handle = (TMR_handle_t)i;
                break;
            }   
        } 
    }
    
    return handle;
}


/*********************************************************************************************************
 * @brief     pause a timer, do nothing if the timer is already paused.
 *            return FALSE if the timer is expired, or it is invalid.
 *
 * @param   handle :  number of timer
 * @usage
 *
 * @return  TRUE if no error. 
 *          FALSE otherwise.
 * 
****************************************************************************************************/
bool_t TMR_PauseTimer( TMR_handle_t handle)
{
    bool_t  bRet = FALSE;
    
    if( ( tmr_zStatus.bModuleTmrStarted ) &&
        ( handle >= 0u ) && ( handle < TMR_CFG_MAX_TIMER_CNT ))
    {
        
        if (( tmr_zStatus.TMR_info_t[handle].eState == TMR_eStateInvalid ) || 
            ( tmr_zStatus.TMR_info_t[handle].eState == TMR_eStateExpired ))
        {
            bRet = FALSE;
        }
        else if (tmr_zStatus.TMR_info_t[handle].eState != TMR_eStatePaused)
        {
            tmr_zStatus.TMR_info_t[handle].eState = TMR_eStatePaused;
            bRet = TRUE;
        }
        else
        {
            bRet = TRUE;
        }
    }
    
    return bRet;
}

/*********************************************************************************************************
 * @brief     resume a paused timer.
 *            do nothing if the timer is not paused, and still active.
 *            return FALSE if timer is not valid or expired.
 *
 * @param   handle :  number of timer that needs to be destroyed
 * @usage
 *
 * @return  TRUE if no error. 
 *          FALSE otherwise.
 * 
****************************************************************************************************/
bool_t TMR_ResumeTimer( TMR_handle_t handle)
{
    bool_t  bRet = FALSE;
    
    if( ( tmr_zStatus.bModuleTmrStarted ) &&
        ( handle >= 0u ) && ( handle < TMR_CFG_MAX_TIMER_CNT ))
    {
        if (tmr_zStatus.TMR_info_t[handle].eState == TMR_eStateInvalid || tmr_zStatus.TMR_info_t[handle].eState == TMR_eStateExpired)
        {
            bRet = FALSE;
        }
        else if (tmr_zStatus.TMR_info_t[handle].eState == TMR_eStatePaused)
        {
            tmr_zStatus.TMR_info_t[handle].eState = TMR_eStateActivated;
            bRet = TRUE;
        }
        else
        {
            bRet = TRUE;
        }
    }
    
    return TRUE;   
}




