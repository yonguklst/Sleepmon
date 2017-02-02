/**
****************************************************************************************
*
* @file app_easy_timer.c
*
* @brief The easy timer api implementation.
*
* Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
*
* <bluetooth.support@diasemi.com> and contributors.
*
****************************************************************************************
*/

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)
#include "app_api.h"                // Application task Definition
#include "arch_api.h"
#include "rwip.h"
#include "app_msg_utils.h"
#include "app_entry_point.h"
#include "app_easy_timer.h" 

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
#define APP_TIMER_MAX_NUM (APP_TIMER_API_LAST_MES-APP_TIMER_API_MES0+1)
//HND: Timer handler values = 1...APP_TIMER_MAX_NUM
//IDX: The index to the table = 0...APP_TIMER_MAX_NUM-1
//The reason of HND and IDX are different has to do with the fact that the retention
//should be populated with zero init data. This makes difficult to have the invalid hnd in a 
//value other than zero.
#define APP_EASY_TIMER_HND_TO_MSG_ID(timer_id)    (timer_id-1+APP_TIMER_API_MES0)
#define APP_EASY_TIMER_MSG_ID_TO_HND(timer_msg)   (timer_msg-APP_TIMER_API_MES0+1)
#define APP_EASY_TIMER_HND_TO_IDX(timer_id) (timer_id-1)
#define APP_EASY_TIMER_IDX_TO_HND(timer_id) (timer_id+1)
#define APP_EASY_TIMER_HND_IS_VALID(timer_id) ((timer_id>0)&&(timer_id<=APP_TIMER_MAX_NUM))

struct cancel_timer_struct {
  timer_hnd timer_id;
};
struct create_new_timer_struct {
  timer_hnd timer_id;
  uint16_t delay;
};



typedef void (*void_tim_void) (void);

static void_tim_void timer_callbacks[APP_TIMER_MAX_NUM] __attribute__((section("retention_mem_area0"),zero_init));

static int app_easy_timer_new_handler(ke_msg_id_t const msgid,
                                struct create_new_timer_struct const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);
static int app_easy_timer_cancel_handler(ke_msg_id_t const msgid,
                                struct cancel_timer_struct const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);
static int app_easy_timer_handler(ke_msg_id_t const msgid,
                                        void const *param,
										ke_task_id_t const dest_id,
										ke_task_id_t const src_id);
                                   

/**
 ****************************************************************************************
 * @brief Process Handle for the Easy Timer Messages.
 * @param[in] msgid Id of the message received.
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @param[in] msg_ret Result of the message handler.
 * @return Returns if the message is handled by the process handler.
 ****************************************************************************************
 */
enum process_event_response app_timer_api_process_handler (ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id, 
                                         enum ke_msg_status_tag *msg_ret)
{
    
    switch (msgid)
    {
        case APP_CANCEL_TIMER:
                              (*msg_ret)=(enum ke_msg_status_tag)app_easy_timer_cancel_handler(msgid, param, dest_id, src_id);
                              return (PR_EVENT_HANDLED);
        case APP_CREATE_NEW_TIMER:
                              (*msg_ret)=(enum ke_msg_status_tag)app_easy_timer_new_handler(msgid, param, dest_id, src_id);
                              return (PR_EVENT_HANDLED);

        default: if ((msgid<APP_TIMER_API_MES0)||(msgid>APP_TIMER_API_LAST_MES))
                        return (PR_EVENT_UNHANDLED);
                 else
                        (*msg_ret)=(enum ke_msg_status_tag)app_easy_timer_handler (msgid, param, dest_id, src_id);
                        return (PR_EVENT_HANDLED);
    }
}




/**
 ****************************************************************************************
 * @brief Place a callback in the first available position in the timer callback array.
 * @param[in] fn The callback to add.
 * @return The handler of the timer for future reference. EASY_TIMER_INVALID_TIMER if
 * there is no timer available.
 ****************************************************************************************
 */
static timer_hnd app_easy_timer_set_callback (void (*fn)(void))
{
    int i;
    for (i=0;i<APP_TIMER_MAX_NUM;i++)
    {
        if (timer_callbacks[i]==NULL)
        {
            timer_callbacks[i]=fn;
            return(APP_EASY_TIMER_IDX_TO_HND(i));
        }
    }
    return(EASY_TIMER_INVALID_TIMER);
}

/**
 ****************************************************************************************
 * @brief Empty callback used when canceling a timer.
 * @return None.
 ****************************************************************************************
 */
static void app_easy_timer_canceled_handler (void)
{
    //this functions puts an empty handler in the 
    //when a timer is removed. This handler replaces the 
    //original handler. We have to do to compensate 
    //the case when a timer is canceled but issued in the same 
    //tick.
     
}

/**
 ****************************************************************************************
 * @brief Call the callback of a specific timer handler if it exists.
 * @param[in] timerid The handler to call.
 * @return None.
 ****************************************************************************************
 */
static void app_easy_timer_call_callback ( const timer_hnd timerid )
{
    if APP_EASY_TIMER_HND_IS_VALID(timerid)
    {
        if (timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timerid)]!=NULL)
        {
            void (*callback)(void) = timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timerid)];
            timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timerid)] = NULL;
            callback();
        }
    }    else
        ASSERT_WARNING(0);
}
 
/**
 ****************************************************************************************
 * @brief Erase the callback of a specific timer handler.
 * @param[in] timerid Erase the callback of this timer handler.
 * @return If the operation was succesful return the value of the handler else return EASY_TIMER_INVALID_TIMER.
 ****************************************************************************************
 */
timer_hnd app_easy_timer_free_callback( const timer_hnd timerid)
{
    if APP_EASY_TIMER_HND_IS_VALID(timerid)
    {
        timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timerid)]=NULL;
        return(timerid);
    }    else
        ASSERT_WARNING(0);
    return(EASY_TIMER_INVALID_TIMER);
}



/**
 ****************************************************************************************
 * @brief Cancel all the active timers.
 * @return None.
 ****************************************************************************************
 */
void app_easy_timer_cancel_all( void )
{
    int i;
    for (i=0;i<APP_TIMER_MAX_NUM;i++)
    if ((timer_callbacks[i]!=NULL)&&\
           (timer_callbacks[i]!=app_easy_timer_canceled_handler)) 
        app_easy_timer_cancel(i);
}

/**
 ****************************************************************************************
 * @brief Create a new timer. Activate the ble if required.
 * @param[in] delay The amount of timer slots (10 ms) to wait.
 * @param[in] fn The callback to be called when timer expires.
 * @return The handler of the timer for future reference. If there are not timers available
 * EASY_TIMER_INVALID_TIMER will be returned.
 ****************************************************************************************
 */
timer_hnd app_easy_timer(const uint16_t delay, void (*fn) (void))
{
   timer_hnd timer_id=app_easy_timer_set_callback(fn);
   if (timer_id==EASY_TIMER_INVALID_TIMER) 
       return (EASY_TIMER_INVALID_TIMER); //No timers available

    if (app_check_BLE_active())
        ke_timer_set(APP_EASY_TIMER_HND_TO_MSG_ID(timer_id), TASK_APP, delay);
    else
    {
        arch_ble_force_wakeup(); //wake_up BLE
        //send a message to wait for BLE to be woken up before executing the 
        struct create_new_timer_struct *req = KE_MSG_ALLOC(APP_CREATE_NEW_TIMER, TASK_APP, TASK_APP,
                                                 create_new_timer_struct);

        req->delay=delay;
        req->timer_id=timer_id;
        ke_msg_send(req);        
    }
     return (timer_id);
}

/**
 ****************************************************************************************
 * @brief Cancel an active timer.
 * @param[in] timerid The timer handler to cancel.
 * @return None.
 ****************************************************************************************
 */
void app_easy_timer_cancel(const timer_hnd timer_id)
{
   if APP_EASY_TIMER_HND_IS_VALID(timer_id)
   { 
       if ((timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timer_id)]!=NULL)&&\
           (timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timer_id)]!=app_easy_timer_canceled_handler))
       {
            ke_timer_clear(APP_EASY_TIMER_HND_TO_MSG_ID(timer_id), TASK_APP);
            timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timer_id)]=app_easy_timer_canceled_handler;
            struct cancel_timer_struct *req = KE_MSG_ALLOC(APP_CANCEL_TIMER, TASK_APP, TASK_APP,
                                                 cancel_timer_struct);

            req->timer_id=timer_id;
            ke_msg_send(req);        
        }
       else
        ASSERT_WARNING(0);
   }
    else
        ASSERT_WARNING(0);
}

/**
 ****************************************************************************************
 * @brief Modify the delay of an existing timer.
 * @param[in] timerid The timer handler to modify.
 * @param[in] delay The new delay value.
 * @return The timer handler if everything is ok.
 ****************************************************************************************
 */
timer_hnd app_easy_timer_modify(const timer_hnd timer_id, uint16_t delay)
{
   if APP_EASY_TIMER_HND_IS_VALID(timer_id)
   { 
       if ((timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timer_id)]!=NULL)&&\
           (timer_callbacks[APP_EASY_TIMER_HND_TO_IDX(timer_id)]!=app_easy_timer_canceled_handler))
       {
            ke_timer_clear(APP_EASY_TIMER_HND_TO_MSG_ID(timer_id) , TASK_APP);
            ke_timer_set(APP_EASY_TIMER_HND_TO_MSG_ID(timer_id) , TASK_APP, delay);
            return (timer_id);
        }
       else
        ASSERT_WARNING(0);
   }
   else
        ASSERT_WARNING(0);
   return (EASY_TIMER_INVALID_TIMER);
}

/**
 ****************************************************************************************
 * @brief Helper function that sets a new timer. Called after the ble wakes up in the case 
 * where the ble is sleeping when trying to set a new timer.
 * @param[in] msgid Id of the message received.
 * @param[in] param The timer details to be set.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return Returns KE_MSG_CONSUMED.
 ****************************************************************************************
 */
static int app_easy_timer_new_handler(ke_msg_id_t const msgid,
                                struct create_new_timer_struct const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
   ke_timer_set(APP_EASY_TIMER_HND_TO_MSG_ID(param->timer_id), TASK_APP, param->delay);
   return (KE_MSG_CONSUMED);    
}

/**
 ****************************************************************************************
 * @brief Helper function that is called when a timer is canceled. 
 * @param[in] msgid Id of the message received.
 * @param[in] param The timer details to be canceled.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return Returns KE_MSG_CONSUMED.
 ****************************************************************************************
 */
static int app_easy_timer_cancel_handler(ke_msg_id_t const msgid,
                                struct cancel_timer_struct const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
   if APP_EASY_TIMER_HND_IS_VALID(param->timer_id)
   {  
     int i=APP_EASY_TIMER_HND_TO_IDX(param->timer_id);  
     if (timer_callbacks[i]==app_easy_timer_canceled_handler)  //the timer may be cleared (the event came)
                                                          //and then it was granted before the cancel
                                                          //message occured
          timer_callbacks[i]=NULL;
   }    else
        ASSERT_ERROR(0);
    return (KE_MSG_CONSUMED);    
}

/**
 ****************************************************************************************
 * @brief The actual timer handler that calls the user callback. 
 * @param[in] msgid Id of the message received.
 * @param[in] param No parameters are required.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return Returns KE_MSG_CONSUMED.
 ****************************************************************************************
 */
static int app_easy_timer_handler(ke_msg_id_t const msgid,
                                        void const *param,
										ke_task_id_t const dest_id,
										ke_task_id_t const src_id)
{
  app_easy_timer_call_callback(APP_EASY_TIMER_MSG_ID_TO_HND(msgid)); 
  return (KE_MSG_CONSUMED);   
}

 
 
 #endif //(BLE_APP_PRESENT)

/// @} APP
