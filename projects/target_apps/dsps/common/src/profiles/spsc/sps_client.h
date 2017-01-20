/**
 ****************************************************************************************
 *
 * @file sps_client.h
 *
 * @brief Header file - SPS_CLIENT.
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: 7794 $
 *
 ****************************************************************************************
 */

#ifndef SPS_CLIENT_H_
#define SPS_CLIENT_H_

/**
 ****************************************************************************************
 * @addtogroup SPS Serial Port Service
 * @ingroup PROFILE
 * @brief Serial Port Service
 *****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup SPS Serial Port Service Client
 * @ingroup SPS
 * @brief SPS Profile role: Client
 * @{
 ****************************************************************************************
 */

/// Serial Port Service Client Role
#define BLE_SPS_CLIENT          1
#if !defined (BLE_CLIENT_PRF)
    #define BLE_CLIENT_PRF      1
#endif 

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_SPS_CLIENT)

#include "ke_task.h"
#include "prf_types.h"

/*
 * Defines
 ****************************************************************************************
 */

#define SPS_SERVICE_UUID    {0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SPS_SERVER_TX_UUID  {0xb8, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SPS_SERVER_RX_UUID  {0xba, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SPS_FLOW_CTRL_UUID  {0xb9, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

#define SPS_CLIENT_CLEANUP_FNCT        (NULL)

/// SPS Service Characteristics
enum
{
    SPSC_SRV_TX_DATA_CHAR = 0,
    SPSC_SRV_RX_DATA_CHAR,
    SPSC_FLOW_CTRL_CHAR,

    SPSC_CHAR_MAX
};

/// SPS Service Characteristic Descriptors
enum
{
    /// SPS client config
    SPSC_SRV_TX_DATA_CLI_CFG,
    SPSC_FLOW_CTRL_CLI_CFG,
    
    SPSC_DESC_MAX,
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

/// Service information
struct spsc_sps_content
{
    /// Service info
    struct prf_svc svc;

    /// Characteristic info:
    struct prf_char_inf chars[SPSC_CHAR_MAX];

    /// Characteristic cfg
    struct prf_char_desc_inf descs[SPSC_DESC_MAX];
};

/// SPS Host environment variable
struct spsc_env_tag
{
    /// Profile Connection Info
    struct prf_con_info con_info;

    /// Last requested UUID(to keep track of the two services and char)
    uint16_t last_uuid_req;
    /// Last service for which something was discovered
    uint8_t last_svc_req[ATT_UUID_128_LEN];

    /// Last char. code requested to read.
    uint8_t last_char_code;

    /// counter used to check service uniqueness
    uint8_t nb_svc;

    /// SPS Device Service information recovered from discovery
    struct spsc_sps_content sps;
    
    /// Flow control states
    bool tx_flow_en;
    bool rx_flow_en;
    /// Pending TX
    struct ke_msg *pending_tx_msg;
    /// Write no rsp completion remaining
    uint8_t pending_wr_no_rsp_cmp;
    /// Pending notification of data tx
    bool pending_tx_ntf_cmp;
};


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

extern struct spsc_env_tag **spsc_envs;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the SPS_CLIENT module.
 * This function performs all the initializations of the SPS_CLIENT module.
 ****************************************************************************************
 */
void sps_client_init(void);

/**
 ****************************************************************************************
 * @brief When all handles received correctly it can send and receive data or flow control
 ****************************************************************************************
 */
void sps_client_enable_cfm_send(struct spsc_env_tag *spsc_env, struct prf_con_info *con_info, uint8_t status);

/**
 ****************************************************************************************
 * @brief Confirm that data has been sent
 ****************************************************************************************
 */
void sps_client_confirm_data_tx(const struct spsc_env_tag *spsc_env, uint8_t status);

/**
 ****************************************************************************************
 * @brief  Send data to app
 ****************************************************************************************
 */
void sps_client_indicate_data_rx(const struct spsc_env_tag *spsc_env, const uint8_t *data, uint16_t length);

/**
 ****************************************************************************************
 * @brief  Send flow control state to app
 ****************************************************************************************
 */
void sps_client_indicate_tx_flow_ctrl(const struct spsc_env_tag *spsc_env, uint8_t flow_ctrl);

/**
 ****************************************************************************************
 * @brief  Find next empty characteristic description
 ****************************************************************************************
 */
uint8_t spsc_get_next_desc_char_code(struct spsc_env_tag *spsc_env,
                                     const struct prf_char_desc_def *desc_def);

#endif //BLE_SPS_CLIENT

/// @} SPS_CLIENT

#endif // SPS_CLIENT_H_
