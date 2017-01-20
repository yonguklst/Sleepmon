 /**
 ****************************************************************************************
 *
 * @file  user_spss.c
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

#if (BLE_SPS_SERVER)

#include "app_task.h"                // application task definitions
#include "sps_server_task.h"        // SPS functions
#include "user_spss.h"
#include "app_prf_perm_types.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
 /**
 ****************************************************************************************
 * @brief       user_sps_create_db creates a database for the sps server service
 * @param[in]   void
 * @return      none
 ****************************************************************************************
 */
void user_sps_db_create(void)
{
    // Add sps server service to the database
    struct sps_server_create_db_req * req = KE_MSG_ALLOC(SPS_SERVER_CREATE_DB_REQ, TASK_SPS_SERVER,
        TASK_APP, sps_server_create_db_req);
    req->start_hdl = 0;
    
    // Send the message
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief       Function used to enable server role
 * @param[in]   Connection handle
 * @return      none
 ****************************************************************************************
 */
void user_sps_enable(uint16_t conhdl)
{
    struct sps_server_enable_req * req = KE_MSG_ALLOC(SPS_SERVER_ENABLE_REQ, TASK_SPS_SERVER,
        TASK_APP, sps_server_enable_req);
    
    // Fill in the parameter structure
    req->conhdl             = conhdl;
    req->sec_lvl            = SRV_PERM_ENABLE;
    
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
    struct sps_server_data_tx_req * req = KE_MSG_ALLOC_DYN(SPS_SERVER_DATA_TX_REQ,
        TASK_SPS_SERVER, TASK_APP, sps_server_data_tx_req, length);
    
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
    struct sps_server_rx_flow_ctrl_req * req = KE_MSG_ALLOC(SPS_SERVER_RX_FLOW_CTRL_REQ,
        TASK_SPS_SERVER, TASK_APP, sps_server_rx_flow_ctrl_req);
    
    req->flow_control_state = (uint8_t) flow_ctrl;
    
    ke_msg_send(req);
}

#endif // (BLE_SPS_SERVER)

#endif // (BLE_APP_PRESENT)

/// @} APP
