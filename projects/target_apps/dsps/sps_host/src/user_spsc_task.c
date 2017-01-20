/**
****************************************************************************************
*
* @file  user_spsc_task.c
*
* @brief SPS application Message Handlers.
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
#include "rwip_config.h"        // SW configuration

#if (BLE_APP_PRESENT)

#if (BLE_SPS_CLIENT)
#include "user_spsc_task.h"
#include "user_spsc.h"
#include "app_task.h"                // application definitions
#include "sps_client.h"    // SPS functions
#include "sps_client_task.h"
#include "user_sps_scheduler.h"

#include "arch_console.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
#include "app_entry_point.h"

const struct ke_msg_handler user_spss_process_handlers[]=
{
    {SPS_CLIENT_ENABLE_CFM,         (ke_msg_func_t)user_sps_client_enable_cfm_handler},
    {SPS_CLIENT_DATA_TX_CFM,        (ke_msg_func_t)user_sps_client_data_tx_cfm_handler},
    {SPS_CLIENT_DATA_RX_IND,        (ke_msg_func_t)user_sps_client_data_rx_ind_handler},
    {SPS_CLIENT_TX_FLOW_CTRL_IND,   (ke_msg_func_t)user_sps_client_tx_flow_ctrl_ind_handler},
};

enum process_event_response user_spsc_process_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id, 
                                         enum ke_msg_status_tag *msg_ret)
{
    return (app_std_process_event(msgid, param,src_id,dest_id,msg_ret, user_spss_process_handlers,
                                         sizeof(user_spss_process_handlers)/sizeof(struct ke_msg_handler)));
}
/**
 ****************************************************************************************
 * @brief Handles client enable confirmation
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int user_sps_client_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_client_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    user_ble_pull(false, false);
    arch_printf("Profile Enabled\r\n");
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles confirmation that data has been sent
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int user_sps_client_data_tx_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_client_data_tx_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if(param->status == GATT_ERR_NO_ERROR)
        user_ble_pull(false, true);
    else
        user_ble_pull(false, false);
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles indication that data has been received
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int user_sps_client_data_rx_ind_handler(ke_msg_id_t const msgid,
                                      struct sps_client_data_rx_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    user_ble_push((uint8_t *)param->data, param->length);
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles client flow control state request indication
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int user_sps_client_tx_flow_ctrl_ind_handler(ke_msg_id_t const msgid,
                                      struct sps_client_tx_flow_ctrl_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if(param->flow_control_state == FLOW_ON)
    {
        arch_printf("TX FLOW ON\r\n");
    }
    else
    {
        arch_printf("TX FLOW OFF\r\n");
    }

    return (KE_MSG_CONSUMED);
}
#endif //(BLE_SPS_CLIENT)

#endif //(BLE_APP_PRESENT)

/// @} APPTASK

