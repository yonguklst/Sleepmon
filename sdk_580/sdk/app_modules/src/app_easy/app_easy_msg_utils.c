/**
****************************************************************************************
*
* @file app_easy_msg_utils.c
*
* @brief Message related helper function.
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

#if (BLE_APP_PRESENT)
#include "app_task.h"                // Application task Definition
#include "app.h"                     // Application Definition
#include "ke_msg.h"
#include "app_msg_utils.h"
#include "arch_api.h"
/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 
#include "rwip.h"


#include "app_entry_point.h"

#define APP_MSG_UTIL_MAX_NUM (APP_MSG_UTIL_API_LAST_MES-APP_MSG_UTIL_API_MES0+1)

typedef void (*void_msg_void) (void);

/**
 ****************************************************************************************
 * @brief Array holding the callbacks for the pull of messages.
 ****************************************************************************************
 */
static void_msg_void msg_util_callbacks[APP_MSG_UTIL_MAX_NUM] __attribute__((section("retention_mem_area0"),zero_init));


/**
 ****************************************************************************************
 * @brief Find the array index for a given message id.
 * @param[in] msg_util_id The message id.
 * @return Returns the index in the callback array.
 ****************************************************************************************
 */
static int8_t app_easy_msg_find_idx ( ke_msg_id_t msg_util_id)
{
     int i;
      i=msg_util_id-APP_MSG_UTIL_API_MES0;
     if ( (i>=0) && (i<APP_MSG_UTIL_MAX_NUM))
         return (i);
    return(-1);   
}

/**
 ****************************************************************************************
 * @brief Find the first free entry in the array of callbacks and populate it
 * with a callback.
 * @param[in] fn The pointer to the callback function.
 * @return Returns the message id hooked with the callback and zero if no free entry exists.
 ****************************************************************************************
 */
static ke_msg_id_t app_easy_msg_set_callback (void (*fn)(void))
{
    int i;
    for (i=0;i<APP_MSG_UTIL_MAX_NUM;i++)
    {
        if (msg_util_callbacks[i]==NULL)
        {
            msg_util_callbacks[i]=fn;
            return(APP_MSG_UTIL_API_MES0+i);
        }
    }
    return(0);
}

 /**
 ****************************************************************************************
 * @brief Call the callback hooked to a given message.
 * @param[in] msg_util_id The message id.
 * @return None.
 ****************************************************************************************
 */
static void app_easy_msg_call_callback ( ke_msg_id_t msg_util_id)
{
    int i;
    i=app_easy_msg_find_idx (msg_util_id);
    if (i>=0)
    {
        if (msg_util_callbacks[i]!=NULL)
        {
            void (*callback)(void)=msg_util_callbacks[i];
            msg_util_callbacks[i]=NULL;
            callback();
        }
    }    else
        ASSERT_WARNING(0);
}

/**
 ****************************************************************************************
 * @brief Clear the callback function for a given message.
 * @param[in] msg_util_id The Id of the message.
 * @return Returns the message id if everything was ok.
 ****************************************************************************************
 */
ke_msg_id_t app_easy_msg_free_callback ( ke_msg_id_t msg_util_id)
{
    int i;
    i=app_easy_msg_find_idx (msg_util_id);
    if (i>=0)
    {
        msg_util_callbacks[i]=NULL;
        return(msg_util_id);
    }    else
        ASSERT_WARNING(0);
    return(0);
}

/**
 ****************************************************************************************
 * @brief Hook a callback to a free message from a pool of messages.
 * @param[in] fn Pointer to the callback function.
 * @return Returns the message id, zero if no message is available.
 ****************************************************************************************
 */
ke_msg_id_t app_easy_msg_set(void (*fn)(void))
{

   ke_msg_id_t msg_id=app_easy_msg_set_callback(fn);
   if (msg_id==0) 
       return (0xFFFF); //No timers available
   return (msg_id);
}

/**
 ****************************************************************************************
 * @brief Change the callback for a given message.
 * @param[in] msg_util_id The Id of the message.
 * @param[in] fn The new pointer to the callback function.
 * @return Returns the message id, zero if no message is available.
 ****************************************************************************************
 */
ke_msg_id_t app_easy_msg_modify(ke_msg_id_t msg_util_id, void (*fn)(void))
{
   int i;
   i=app_easy_msg_find_idx (msg_util_id);
   if (i>=0)
   { 
        msg_util_callbacks[i]=fn;
        return(msg_util_id);
   }
   else
        ASSERT_WARNING(0);
   return (0xFFFF);
}


/**
 ****************************************************************************************
 * @brief The message id used by the easy wakeup functions.
 ****************************************************************************************
 */
static ke_msg_id_t wakeup_id __attribute__((section("retention_mem_area0"),zero_init));

/**
 ****************************************************************************************
 * @brief Set the callback to be invoked when the wake-up message issued
 * from the app_easy_wakeup function is scheduled.
 * @param[in] fn Pointer to the callback function.
 * @return None.
 ****************************************************************************************
 */
void app_easy_wakeup_set (void (*fn)(void))
{
    if (wakeup_id==0)
        wakeup_id=app_easy_msg_set(fn);
    else
        app_easy_msg_modify(wakeup_id,fn);
}

/**
 ****************************************************************************************
 * @brief Remove the callback from the wake-up message handler.
 * @return None.
 ****************************************************************************************
 */
void app_easy_wakeup_free (void)
{
    app_easy_msg_free_callback(wakeup_id);
    wakeup_id=0;
    
}

/**
 ****************************************************************************************
 * @brief This function should be called from within the wake-up handler.
 * It will put a message in the scheduler queue which will be called when the BLE stack is 
 * fully awake and operable.
 * @return None.
 ****************************************************************************************
 */
void app_easy_wakeup(void)
{
    if (wakeup_id!=NULL)
        ke_msg_send_basic(wakeup_id, TASK_APP, 0);  
}

/**
 ****************************************************************************************
 * @brief Process Handle for the Easy MSG Messages.
 * @param[in] msgid Id of the message received.
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @param[in] msg_ret Result of the message handler.
 * @return Returns if the message is handled by the process handler.
 ****************************************************************************************
 */
enum process_event_response app_msg_utils_api_process_handler (ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id, 
                                         enum ke_msg_status_tag *msg_ret)
{
    if ((msgid<APP_MSG_UTIL_API_MES0)||(msgid>APP_MSG_UTIL_API_LAST_MES))
        return (PR_EVENT_UNHANDLED);
    else
    {
        app_easy_msg_call_callback (msgid);
        (*msg_ret)=KE_MSG_CONSUMED;
        return (PR_EVENT_HANDLED);
    }
}




    
    
    
    
    


 
 
 #endif //(BLE_APP_PRESENT)

/// @} APP
