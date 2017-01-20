/**
****************************************************************************************
*
* @file user_spsc_task.h
*
* @brief SPSS task header. SPS client handlers declaration.
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

#include "rwip_config.h"

#ifndef USER_SPSC_TASK_H_
#define USER_SPSC_TASK_H_

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
#if (BLE_SPS_CLIENT)

#include "sps_client_task.h"

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
 * @brief Handles client enable confirmation
 ****************************************************************************************
 */
int user_sps_client_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_client_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles confirmation that data has been sent
 ****************************************************************************************
 */
int user_sps_client_data_tx_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_client_data_tx_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles indication that data has been received
 ****************************************************************************************
 */
int user_sps_client_data_rx_ind_handler(ke_msg_id_t const msgid,
                                      struct sps_client_data_rx_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles client flow control state request indication
 ****************************************************************************************
 */
int user_sps_client_tx_flow_ctrl_ind_handler(ke_msg_id_t const msgid,
                                      struct sps_client_tx_flow_ctrl_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

#endif //BLE_SPS_CLIENT

/// @} APP

#endif // USER_SPSC_TASK_H_
