/**
 ****************************************************************************************
 *
 * @file sps_client_task.h
 *
 * @brief Header file - SPS_CLIENTTASK.
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: 5767 $
 *
 ****************************************************************************************
 */

#ifndef SPS_CLIENT_TASK_H_
#define SPS_CLIENT_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup SPSCTASK Serial Port Service client Task
 * @ingroup SPSC
 * @brief Serial Port Service client Task
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "ke_task.h"

#if (BLE_SPS_CLIENT)
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Serial Port Service client task instances
#define SPS_CLIENT_IDX_MAX    	(BLE_CONNECTION_MAX)
/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the SPS_CLIENT task
enum
{
    /// IDLE state
    SPS_CLIENT_IDLE,
    
    ///Connected state
    SPS_CLIENT_CONNECTED,
    
    ///Discovering
    SPS_CLIENT_DISCOVERING,
    
    /// Number of defined states.
    SPS_CLIENT_STATE_MAX
};

///SPS Host API messages
enum
{
    ///SPS client role enable request from application.
    SPS_CLIENT_ENABLE_REQ = KE_FIRST_MSG(TASK_SPS_CLIENT),
    
    /// SPS Host role enable confirmation to application.
    SPS_CLIENT_ENABLE_CFM,

    /// Inform APP that the profile client role has been disabled after a disconnection
    SPS_CLIENT_DISABLE_IND,
    
    /// Request to transmit data
    SPS_CLIENT_DATA_TX_REQ,
    
    /// Confirm that data has been sent
    SPS_CLIENT_DATA_TX_CFM,
    
    /// Send data to app
    SPS_CLIENT_DATA_RX_IND,
    
    // Send flow control status
    SPS_CLIENT_RX_FLOW_CTRL_REQ,
    
    /// Indicate flow control state
    SPS_CLIENT_TX_FLOW_CTRL_IND,
};

/*
 * API Messages Structures
 ****************************************************************************************
 */

///Parameters of the @ref SPS_CLIENT_ENABLE_REQ message
struct sps_client_enable_req
{
    /// Connection handle
    uint16_t conhdl;
    
    /// Connection type
    uint8_t con_type;
    
    /// SPS Device details kept in APP
    struct spsc_sps_content sps;
};

///Parameters of the @ref SPS_CLIENT_ENABLE_CFM message
struct sps_client_enable_cfm
{
    /// Connection handle
    uint16_t conhdl;
   
    /// Status
    uint8_t status;
    
    /// SPS Device details to keep in APP
    struct spsc_sps_content sps;
};

///Parameters of the @ref SPS_CLIENT_DATA_TX_REQ message
struct sps_client_data_tx_req
{
    ///Connection handle
    uint16_t conhdl;
    ///Data length
    uint16_t length;
    ///Data
    uint8_t data[__ARRAY_EMPTY];
};

///Parameters of the @ref SPS_CLIENT_DATA_TX_CFM message
struct sps_client_data_tx_cfm
{
    ///Connection handle
    uint16_t conhdl;
    ///Status
    uint8_t status;
};

///Parameters of the @ref SPS_CLIENT_DATA_RX_IND message
struct sps_client_data_rx_ind
{
    ///Connection handle
    uint16_t conhdl;
    ///Data length
    uint16_t length;
    ///Data
    uint8_t data[__ARRAY_EMPTY];
};

///Parameters of the @ref SPS_CLIENT_RX_FLOW_CTRL_REQ message
struct sps_client_rx_flow_ctrl_req
{
    ///Connection handle
    uint16_t conhdl;
    // flow control state
    uint8_t flow_control_state;
};

///Parameters of the @ref SPS_CLIENT_TX_FLOW_CTRL_IND message
struct sps_client_tx_flow_ctrl_ind
{
    ///Connection handle
    uint16_t conhdl;
    // flow control state
    uint8_t flow_control_state;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern const struct ke_state_handler sps_client_state_handler[SPS_CLIENT_STATE_MAX];
extern const struct ke_state_handler sps_client_default_handler;
extern ke_state_t sps_client_state[SPS_CLIENT_IDX_MAX];

/*
 * Functions
 ****************************************************************************************
 */

#endif //BLE_SPS_CLIENT

/// @} SPS_CLIENTTASK

#endif // SPS_CLIENT_TASK_H_
