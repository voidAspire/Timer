#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

#define TMR_CFG_MAX_TIMER_CNT           (20u)
#define TMR_CFG_INVALID_HANDLE          (TMR_CFG_MAX_TIMER_CNT)




typedef uint16_t TMR_handle_t;

typedef uint16_t (* TMR_pfnNotifier_t) ( TMR_handle_t handle, void* pvCtx );

typedef enum TMR_eState_t
{
    TMR_eStateInvalid = 0u,
    TMR_eStateActivated,
    TMR_eStatePaused,
    TMR_eStateExpired

}TMR_eState_t;


typedef enum TMR_ePerodicity_t
{
    TMR_eProdicitySingleShot = 0u,
    TMR_eProdicityMultiple,
}   TMR_ePerodicity_t;

#ifdef TMR_CFG_DEBUG_TIMER
//Test timer accuracy, just for testing
uint32_t TMR_ReturnErrorNum();
#endif
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
bool_t TMR_Init(void);


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
bool_t TMR_Shutdown ( void );


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
uint32_t TMR_CancleTimer( TMR_handle_t handle);


/*********************************************************************************************************
 * @brief     set a timer of custom
 *
 * @param   uIntervalMsec: time in msec
 * @param   bIsPeriodic:    if the timer is periodic.
 * @param   pfnCallback :  timer expires response function
 * @usage
 *
 * @return  timer handle if no error.
            NULL/0u otherwise.
 *
 * 
****************************************************************************************************/
TMR_handle_t TMR_SetTimer(uint32_t uIntervalMsec, bool_t bIsPeriodic, TMR_pfnNotifier_t pfnCallback, void *contxt);


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
bool_t TMR_PauseTimer( TMR_handle_t handle);

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
bool_t TMR_ResumeTimer( TMR_handle_t handle);


#endif /* _TIMER_H_ */
