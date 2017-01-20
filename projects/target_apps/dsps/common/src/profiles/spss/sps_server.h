/**
 ****************************************************************************************
 *
 * @file sps_server.h
 *
 * @brief Header file - SPSS
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef SPS_SERVER_H_
#define SPS_SERVER_H_

/**
 ****************************************************************************************
 * @addtogroup SPS_SERVER
 * @ingroup SPSS
 * @brief SPS Server Profile
 *
 * The SPS_SERVER module is the responsible block for receiving data, process it and send
 * it to the application.
 * @{
 *****************************************************************************************
 */

/// Software Patching Over the Air Receiver Role
#define BLE_SPS_SERVER          1
#if !defined (BLE_SERVER_PRF)
    #define BLE_SERVER_PRF      1
#endif

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_SPS_SERVER)

#include "prf_types.h"
#include "attm_db_128.h"
/*
 * DEFINES
 ****************************************************************************************
 */


/* Serial Port Server Data Base Characteristic enum */
enum
{
    SPSS_IDX_SVC = 0,

    SPSS_IDX_TX_DATA_CHAR,
    SPSS_IDX_TX_DATA_VAL,
    SPSS_IDX_TX_DATA_CFG,
    SPSS_IDX_TX_DATA_DESC,

    SPSS_IDX_RX_DATA_CHAR,
    SPSS_IDX_RX_DATA_VAL,
    SPSS_IDX_RX_DATA_CFG,
    SPSS_IDX_RX_DATA_DESC,

    SPSS_IDX_FLOW_CTRL_CHAR,
    SPSS_IDX_FLOW_CTRL_VAL,
    SPSS_IDX_FLOW_CTRL_CFG,
    SPSS_IDX_FLOW_CTRL_DESC,

    SPSS_IDX_NB
};

///Attribute Table Indexes
enum
{
    SPSS_TX_DATA_CHAR,
    SPSS_RX_DATA_CHAR,
    SPSS_FLOW_CTRL_CHAR,

    SPSS_CHAR_MAX,
};

typedef enum flow_ctrl_states
{
    FLOW_ON     = (uint8_t)0x01,
    FLOW_OFF    = (uint8_t)0x02,
}flow_ctrl_state_t;


/*
 * STRUCTURES
 ****************************************************************************************
 */

///SPS Server environment variable
struct spss_env_tag
{
    ///connection information
    struct prf_con_info con_info;
    
    /// DB Handle
    uint16_t shdl;
    /// Attribute Table
    uint8_t att_tbl[SPSS_CHAR_MAX];
    
    /// Flow control states
    bool tx_flow_en;
    bool rx_flow_en;
    /// Pending TX
    bool pending_tx;
    /// Notification completion remaining
    uint8_t pending_ntf_cmp;
    /// Pending notification of data tx
    bool pending_tx_ntf_cmp;

};

/*
 *  SPS PROFILE ATTRIBUTES VALUES DECLARATION
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern const struct attm_desc_128 sps_att_db[SPSS_IDX_NB];

extern struct spss_env_tag spss_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Init of the SPS device
 ****************************************************************************************
 */
void sps_server_init(void);

/**
 ****************************************************************************************
 * @brief Confirm that data has been sent
 ****************************************************************************************
 */
void sps_server_confirm_data_tx(uint8_t status);

/**
 ****************************************************************************************
 * @brief  Send data to app
 ****************************************************************************************
 */
void sps_server_indicate_data_rx(const uint8_t *data, uint16_t length);

/**
 ****************************************************************************************
 * @brief  Send flow control state to app
 ****************************************************************************************
 */
void sps_server_indicate_tx_flow_ctrl(uint8_t flow_ctrl);

#endif /* BLE_SPS_SERVER */

/// @} SPS_SERVER_DEV

#endif // SPS_SERVER_H_
