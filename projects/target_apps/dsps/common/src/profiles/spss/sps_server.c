 /**
 ****************************************************************************************
 *
 * @file sps_server.c
 *
 * @brief Serial Port Service Server Implementation.
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
 
/**
 ****************************************************************************************
 * @addtogroup SPS_SERVER
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_SPS_SERVER)

#include "attm_db_128.h"
#include "sps_server_task.h"
#include "sps_server.h"
#include "gapc.h"

/*
 * SPSS PROFILE ATTRIBUTES DEFINITION
 ****************************************************************************************
 */
#define SPS_SERVICE_UUID     {0x2F, 0x2A, 0x93, 0xA6, 0xBD, 0xD8, 0x41, 0x52, 0xAC, 0x0B, 0x10, 0x99, 0x2E, 0xC6, 0xFE, 0xED}
#define SPS_SERVER_TX_UUID  {0x8C, 0x09, 0xE0, 0xD1, 0x81, 0x54, 0x42, 0x40, 0x8E, 0x4F, 0xD2, 0xB3, 0x77, 0xE3, 0x2A, 0x77}
#define SPS_SERVER_RX_UUID  {0x8C, 0x09, 0xE0, 0xD1, 0x81, 0x54, 0x42, 0x40, 0x8E, 0x4F, 0xD2, 0xB3, 0x77, 0xE3, 0x2A, 0x78}
#define SPS_FLOW_CTRL_UUID  {0x8C, 0x09, 0xE0, 0xD1, 0x81, 0x54, 0x42, 0x40, 0x8E, 0x4F, 0xD2, 0xB3, 0x77, 0xE3, 0x2A, 0x79}

#define SPS_SERVER_TX_CHAR_LEN      160
#define SPS_SERVER_RX_CHAR_LEN      160
#define SPS_FLOW_CTRL_CHAR_LEN      1

#define SPS_SERVER_ENABLE_DESC  "Enable SPS Device"
#define SPS_SERVER_TX_DESC      "Server TX Data"
#define SPS_SERVER_RX_DESC      "Server RX Data"
#define SPS_FLOW_CTRL_DESC      "Flow Control"

/*
 *  SPSS PROFILE ATTRIBUTES VALUES DEFINTION
 ****************************************************************************************
 */

/// SPS Service
static const att_svc_desc128_t sps_service = SPS_SERVICE_UUID;

/// SPS Service - TX Data Characteristic
static struct att_char128_desc spss_tx_data_char = {(ATT_CHAR_PROP_NTF),
                                            {0,0},
                                            SPS_SERVER_TX_UUID};
static const struct att_uuid_128 spss_tx_data_val = {SPS_SERVER_TX_UUID};

/// SPS Service - RX Data Characteristic
static struct att_char128_desc spss_rx_data_char = {(ATT_CHAR_PROP_WR_NO_RESP),
                                            {0,0},
                                            SPS_SERVER_RX_UUID};
static const struct att_uuid_128 spss_rx_data_val = {SPS_SERVER_RX_UUID};

/// SPOTA Service - Flow Control Characteristic
static struct att_char128_desc spss_flow_ctrl_char = {(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR_NO_RESP| ATT_CHAR_PROP_NTF),
                                            {0,0},
                                            SPS_FLOW_CTRL_UUID};
static const struct att_uuid_128 spss_flow_ctrl_val = {SPS_FLOW_CTRL_UUID};

/*
 * CUSTS1 ATTRIBUTES
 ****************************************************************************************
 */

static const uint16_t att_decl_svc       = ATT_DECL_PRIMARY_SERVICE;
static const uint16_t att_decl_char      = ATT_DECL_CHARACTERISTIC;
static const uint16_t att_decl_cfg       = ATT_DESC_CLIENT_CHAR_CFG;
static const uint16_t att_decl_user_desc = ATT_DESC_CHAR_USER_DESCRIPTION;

/// Full SPS Database Description - Used to add attributes into the database
const struct attm_desc_128 sps_att_db[SPSS_IDX_NB] =
{
    // SPS Service Declaration
    [SPSS_IDX_SVC]                      = {(uint8_t*)&att_decl_svc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            sizeof(sps_service), sizeof(sps_service), (uint8_t*)&sps_service},

    // TX Data Characteristic Declaration
    [SPSS_IDX_TX_DATA_CHAR]             = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            sizeof(spss_tx_data_char), sizeof(spss_tx_data_char), (uint8_t*)&spss_tx_data_char},
    // TX Data Characteristic Value
    [SPSS_IDX_TX_DATA_VAL]              = {(uint8_t*)spss_tx_data_val.uuid, ATT_UUID_128_LEN, PERM(NTF, ENABLE),
                                            SPS_SERVER_TX_CHAR_LEN, 0, NULL},
    // TX Data  Characteristic Configuration Descriptor
    [SPSS_IDX_TX_DATA_CFG]              = {(uint8_t*)&att_decl_cfg, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE),
                                            sizeof(uint16_t), 0, NULL},
    // TX Data Characteristic User Description
    [SPSS_IDX_TX_DATA_DESC]             = {(uint8_t*)&att_decl_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            sizeof(SPS_SERVER_TX_DESC) - 1, sizeof(SPS_SERVER_TX_DESC) - 1, SPS_SERVER_TX_DESC},

    // RX Data Characteristic Declaration
    [SPSS_IDX_RX_DATA_CHAR]             = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            sizeof(spss_rx_data_char), sizeof(spss_rx_data_char), (uint8_t*)&spss_rx_data_char},
    // RX Data Characteristic Value
    [SPSS_IDX_RX_DATA_VAL]              = {(uint8_t*)spss_rx_data_val.uuid, ATT_UUID_128_LEN, PERM(WR, ENABLE),
                                            SPS_SERVER_RX_CHAR_LEN, 0, NULL},
    // RX Data  Characteristic Configuration Descriptor
    [SPSS_IDX_RX_DATA_CFG]              = {(uint8_t*)&att_decl_cfg, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE),
                                            sizeof(uint16_t), 0, NULL},
    // RX Data Characteristic User Description
    [SPSS_IDX_RX_DATA_DESC]             = {(uint8_t*)&att_decl_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            sizeof(SPS_SERVER_RX_DESC) - 1, sizeof(SPS_SERVER_RX_DESC) - 1, SPS_SERVER_RX_DESC},

    // Flow Control Characteristic Declaration
    [SPSS_IDX_FLOW_CTRL_CHAR]           = {(uint8_t*)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            sizeof(spss_flow_ctrl_char), sizeof(spss_flow_ctrl_char), (uint8_t*)&spss_flow_ctrl_char},
    // Flow Control Characteristic Value
    [SPSS_IDX_FLOW_CTRL_VAL]            = {(uint8_t*)spss_flow_ctrl_val.uuid, ATT_UUID_128_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(NTF, ENABLE),
                                            SPS_FLOW_CTRL_CHAR_LEN, 0, NULL},
    // Flow Control  Characteristic Configuration Descriptor
    [SPSS_IDX_FLOW_CTRL_CFG]            = {(uint8_t*)&att_decl_cfg, ATT_UUID_16_LEN, PERM(RD, ENABLE) | PERM(WR, ENABLE),
                                            sizeof(uint16_t), 0, NULL},
    // Flow Control Characteristic User Description
    [SPSS_IDX_FLOW_CTRL_DESC]           = {(uint8_t*)&att_decl_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                            sizeof(SPS_FLOW_CTRL_DESC) - 1, sizeof(SPS_FLOW_CTRL_DESC) - 1, SPS_FLOW_CTRL_DESC},
};


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
struct spss_env_tag spss_env __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

/// SPS_SERVER task descriptor
static const struct ke_task_desc TASK_DESC_SPS_SERVER = {sps_server_state_handler, &sps_server_default_handler, sps_server_state, SPS_SERVER_STATE_MAX, SPS_SERVER_IDX_MAX};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Init of the SPS device
 ****************************************************************************************
 */
void sps_server_init(void)
{
    // Reset the sps_receivererometer environment
    memset(&spss_env, 0, sizeof(spss_env));
    
    // Create SPS task
    ke_task_create(TASK_SPS_SERVER, &TASK_DESC_SPS_SERVER);

    // Go to IDLE state
    ke_state_set(TASK_SPS_SERVER, SPS_SERVER_DISABLED);
}

/**
 ****************************************************************************************
 * @brief Confirm that data has been sent
 ****************************************************************************************
 */
void sps_server_confirm_data_tx(uint8_t status)
{
    struct sps_server_data_tx_cfm * cfm = KE_MSG_ALLOC(SPS_SERVER_DATA_TX_CFM, spss_env.con_info.appid,
                                                        TASK_SPS_SERVER, sps_server_data_tx_cfm);

    cfm->conhdl = gapc_get_conhdl(spss_env.con_info.conidx);
    cfm->status = status;

    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief  Send data to app
 ****************************************************************************************
 */
void sps_server_indicate_data_rx(const uint8_t *data, uint16_t length)
{
    struct sps_server_data_rx_ind * ind = KE_MSG_ALLOC_DYN(SPS_SERVER_DATA_RX_IND, spss_env.con_info.appid,
                                                        TASK_SPS_SERVER, sps_server_data_rx_ind, length);

    ind->conhdl = gapc_get_conhdl(spss_env.con_info.conidx);
    ind->length = length;
    memcpy(ind->data, data, length);

    ke_msg_send(ind);
}

/**
 ****************************************************************************************
 * @brief  Send flow control state to app
 ****************************************************************************************
 */
void sps_server_indicate_tx_flow_ctrl(uint8_t flow_ctrl)
{
    struct sps_server_tx_flow_ctrl_ind * ind = KE_MSG_ALLOC(SPS_SERVER_TX_FLOW_CTRL_IND, spss_env.con_info.appid,
                                                        TASK_SPS_SERVER, sps_server_tx_flow_ctrl_ind);

    ind->conhdl = gapc_get_conhdl(spss_env.con_info.conidx);
    ind->flow_control_state = flow_ctrl;

    ke_msg_send(ind);
}

#endif // BLE_SPS_SERVER

/// @} SPS_SERVER

