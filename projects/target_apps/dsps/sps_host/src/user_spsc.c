 /**
 ****************************************************************************************
 *
 * @file  user_spsc.c
 *
 * @brief SPS application.
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
 
#include "rwip_config.h"            // SW configuration

#if (BLE_APP_PRESENT)

#if (BLE_SPS_CLIENT)
#include "sps_client.h"
#include "sps_client_task.h"
#include "user_sps_scheduler.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

/**
 ****************************************************************************************
 * @brief       Function used to enable client role
 * @param[in]   void
 * @return      none
 ****************************************************************************************
 */
void user_sps_enable(uint16_t conhdl)
{
    struct sps_client_enable_req * req = KE_MSG_ALLOC(SPS_CLIENT_ENABLE_REQ, TASK_SPS_CLIENT, TASK_APP,
                                                                        sps_client_enable_req);
    
    // Fill in the parameter structure
    req->conhdl = conhdl;
    req->con_type = PRF_CON_DISCOVERY; 
    
    // Send the message
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief       Send data over Bluetooth
 * @param[in]   flow control (FLOW_ON or FLOW_OFF)
 * @return      none
 ****************************************************************************************
 */
void user_send_ble_data(const uint8_t *data, uint16_t length)
{
    struct sps_client_data_tx_req * req = KE_MSG_ALLOC_DYN(SPS_CLIENT_DATA_TX_REQ,
        TASK_SPS_CLIENT, TASK_APP, sps_client_data_tx_req, length);
    
    req->length = length;
    memcpy(&req->data[0], data, length);
    
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief       Send flow control over Bluetooth
 * @param[in]   flow control (FLOW_ON or FLOW_OFF)
 * @return      none
 ****************************************************************************************
 */
void user_send_ble_flow_ctrl(uint8_t flow_ctrl)
{
    struct sps_client_rx_flow_ctrl_req * req = KE_MSG_ALLOC(SPS_CLIENT_RX_FLOW_CTRL_REQ,
        TASK_SPS_CLIENT, TASK_APP, sps_client_rx_flow_ctrl_req);

    req->flow_control_state = (uint8_t) flow_ctrl;

    ke_msg_send(req);
}

#endif // (BLE_SPS_CLIENT)

#endif // (BLE_APP_PRESENT)

/// @} APP
