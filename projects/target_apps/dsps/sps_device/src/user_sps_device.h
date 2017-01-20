/**
****************************************************************************************
*
* @file user_sps_device.h
*
* @brief Empty peripheral template project header file.
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

#ifndef USER_SPS_DEVICE_H_
#define USER_SPS_DEVICE_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief 
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                 // gap functions and messages
#include "gapm_task.h"                 // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions
#include "smpc_task.h"                 // error code definitions
 

/****************************************************************************
Add here supported profiles' application header files.
i.e.
#if (BLE_DIS_SERVER)
#include "app_dis.h"
#include "app_dis_task.h"
#endif
*****************************************************************************/
#include "user_spss.h"
#include "user_spss_task.h"
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles application events
 ****************************************************************************************
 */
void user_process_catch(ke_msg_id_t const msgid, void const *param,
                                         ke_task_id_t const dest_id, ke_task_id_t const src_id);

/*
 * APP CALLBACKS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Handles connection event
 ****************************************************************************************
 */
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param);

/**
 ****************************************************************************************
 * @brief Handles disconnection event
 ****************************************************************************************
 */
void user_on_disconnect( struct gapc_disconnect_ind const *param );

/**
 ****************************************************************************************
 * @brief Handles connection failure
 ****************************************************************************************
 */
void user_on_connect_failed (void);

/*
 * MAIN LOOP CALLBACKS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Called on device initialization
 ****************************************************************************************
 */
void user_on_init(void);

/**
 ****************************************************************************************
 * @brief Called on system power up
 ****************************************************************************************
 */
enum arch_main_loop_callback_ret user_on_system_powered(void);

/**
 ****************************************************************************************
 * @brief Called before entering sleep
 ****************************************************************************************
 */
void user_before_sleep(void);

/// @} APP

#endif //USER_SPS_DEVICE_H_
