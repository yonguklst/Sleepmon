/**
 ****************************************************************************************
 *
 * @file sps_server_task.c
 *
 * @brief Serial Port Service Device profile task.
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
 * @addtogroup SPS_SERVER_TASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwble_config.h"

#if (BLE_SPS_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm_util.h"
#include "atts_util.h"

#include "sps_server.h"
#include "sps_server_task.h"

#include "prf_utils.h"
#include "gpio.h"
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
extern bool connected;
void system_on(void);
void system_off(void);
/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SPS_SERVER_CREATE_DB_REQ message.
 * The handler adds SPS_SERVER Service into the database using the database
 * configuration value given in param.
 * @param[in] msgid Id of the message received 
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static int sps_server_create_db_req_handler(ke_msg_id_t const msgid,
                                      struct sps_server_create_db_req const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    //DB Creation Statis
    uint8_t status = ATT_ERR_NO_ERROR;

    //Save profile id
    spss_env.con_info.prf_id = TASK_SPS_SERVER;

    status = attm_svc_create_db_128(&spss_env.shdl, NULL, SPSS_IDX_NB, &spss_env.att_tbl[0],
                                dest_id, &sps_att_db[0]);

    //Go to Idle State
    if (status == ATT_ERR_NO_ERROR)
    {
        //Disable SPS
        attmdb_svc_set_permission(spss_env.shdl, PERM(SVC, DISABLE));
        
        //If we are here, database has been fulfilled with success, go to idle state
        ke_state_set(TASK_SPS_SERVER, SPS_SERVER_IDLE);
    }

    //Send response to application
    struct sps_server_create_db_cfm * cfm = KE_MSG_ALLOC(SPS_SERVER_CREATE_DB_CFM, src_id,
                                                    TASK_SPS_SERVER, sps_server_create_db_cfm);
    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SPS_SERVER_ENABLE_REQ message.
 * The handler enables the Serial Port Service Device profile.
 * @param[in] msgid Id of the message received .
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance 
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_server_enable_req_handler(ke_msg_id_t const msgid,
                                    struct sps_server_enable_req const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    //Save the connection handle associated to the profile
    spss_env.con_info.conidx = gapc_get_conidx(param->conhdl);
    //Save the application id
    spss_env.con_info.appid = src_id;

    // Check if the provided connection exist
    if (spss_env.con_info.conidx == GAP_INVALID_CONIDX)
    {
        // The connection doesn't exist, request disallowed
        prf_server_error_ind_send((prf_env_struct *)&spss_env, PRF_ERR_REQ_DISALLOWED,
                                  SPS_SERVER_ERROR_IND, SPS_SERVER_ENABLE_REQ);
    }
    else
    {
        //Enable Attributes + Set Security Level
        attmdb_svc_set_permission(spss_env.shdl, param->sec_lvl);
        
        // Go to connected state
        ke_state_set(TASK_SPS_SERVER, SPS_SERVER_ACTIVE);
        
        //Initialise flow flags
        spss_env.tx_flow_en = true;
        spss_env.rx_flow_en = true;
        
        // Reset counter
        spss_env.pending_ntf_cmp = 0;
        spss_env.pending_tx_ntf_cmp = false;
        
        //Send response to application
        struct sps_server_enable_cfm * cfm = KE_MSG_ALLOC(SPS_SERVER_ENABLE_CFM, spss_env.con_info.appid,
            TASK_SPS_SERVER, sps_server_enable_cfm);
        
        ke_msg_send(cfm);
    }
    
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SPS_SERVER_ENABLE_REQ message.
 * The handler enables the Serial Port Service Device profile.
 * @param[in] msgid Id of the message received .
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance 
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                    struct gapc_disconnect_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    //Disable SPS in database
    attmdb_svc_set_permission(spss_env.shdl, PERM(SVC, DISABLE));

    //Go to idle state
    ke_state_set(TASK_SPS_SERVER, SPS_SERVER_IDLE);
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref gattc_cmp_evt message.
 * The handler enables the Serial Port Service Device profile.
 * @param[in] msgid Id of the message received .
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance 
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gattc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    if(param->req_type == GATTC_NOTIFY)
    {
        ASSERT_WARNING(spss_env.pending_ntf_cmp > 0);
        spss_env.pending_ntf_cmp--;
        if (!spss_env.pending_ntf_cmp && spss_env.pending_tx_ntf_cmp)
        {
            spss_env.pending_tx_ntf_cmp = false;
            sps_server_confirm_data_tx(param->status);
        }
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Send first part of data in buffer @ref sps_server_data_tx_req message.
 * @param[in] msgid Id of the message received 
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_server_data_tx_req_handler(ke_msg_id_t const msgid,
                                struct sps_server_data_tx_req const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
//    if(param->conhdl == gapc_get_conhdl(spss_env.con_info.conidx))
    {
        attmdb_att_set_value(spss_env.shdl + SPSS_IDX_TX_DATA_VAL, sizeof(uint8_t) * param->length, (uint8_t *)param->data);
        if (spss_env.tx_flow_en)
        {
            prf_server_send_event((prf_env_struct *)&spss_env, false, spss_env.shdl + SPSS_IDX_TX_DATA_VAL);
            spss_env.pending_tx_ntf_cmp = true;
            spss_env.pending_ntf_cmp++;
        }
        else
        {
            spss_env.pending_tx = true;
        }
    }

    return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Send flow control state @ref sps_server_rx_flow_ctrl_req message.
 * @param[in] msgid Id of the message received 
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_server_rx_flow_ctrl_req_handler(ke_msg_id_t const msgid,
                                   struct sps_server_data_tx_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
 
{
    // Set value in data base
    attmdb_att_set_value(spss_env.shdl + SPSS_IDX_FLOW_CTRL_VAL, sizeof(uint8_t) * param->length, (uint8_t *)param->data);//sizeof(param->flow_control_state), (uint8_t*) &(param->flow_control_state ));

    // Send notification to the client
    prf_server_send_event((prf_env_struct *)&(spss_env), false, spss_env.shdl + SPSS_IDX_FLOW_CTRL_VAL);
    spss_env.pending_ntf_cmp++;

    //Set flag
  //  spss_env.rx_flow_en = (param->flow_control_state == FLOW_ON ? true : false);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATT_WRITE_CMD_IND message.
 * Receive and proces incoming data 
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_write_cmd_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_write_cmd_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if (param->handle == spss_env.shdl + SPSS_IDX_TX_DATA_CFG) 
    {
        //send write response if required
        if (param->response==0x01)
        {
            atts_write_rsp_send(spss_env.con_info.conidx, param->handle, PRF_ERR_OK);
					//	connected=true;
						//system_on();
        }
    }
    else if (param->handle == (spss_env.shdl + SPSS_IDX_RX_DATA_VAL))
    {
        // send data to the application
        sps_server_indicate_data_rx(param->value, param->length);
    }
    else if (param->handle == (spss_env.shdl + SPSS_IDX_RX_DATA_CFG))
    {
        //send write response if required
        if (param->response==0x01)
        {
            atts_write_rsp_send(spss_env.con_info.conidx, param->handle, PRF_ERR_OK);
        }
    }
    else if (param->handle == spss_env.shdl + SPSS_IDX_FLOW_CTRL_VAL)
    {
        //update ble flow control flag
        if(param->value[0] == FLOW_ON)
        {
            spss_env.tx_flow_en = true;
            sps_server_indicate_tx_flow_ctrl(FLOW_ON);
            if (spss_env.pending_tx)
            {
                prf_server_send_event((prf_env_struct *)&spss_env, false, spss_env.shdl + SPSS_IDX_TX_DATA_VAL);
                spss_env.pending_tx_ntf_cmp = true;
                spss_env.pending_ntf_cmp++;
            }
            spss_env.pending_tx = false;
        }
        else if (param->value[0] == FLOW_OFF)
        {
            spss_env.tx_flow_en = false;
            sps_server_indicate_tx_flow_ctrl(FLOW_OFF);
        }
    }
    else if (param->handle == spss_env.shdl + SPSS_IDX_FLOW_CTRL_CFG)
    {
        // Extract value before check
        uint16_t ntf_cfg = co_read16p(&param->value[0]);

        if ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF))
        {
            //provide ble flow control state when flow control att is configured to 0x0100
            if (ntf_cfg == PRF_CLI_START_NTF)
            {
                uint8_t flow = (spss_env.rx_flow_en ? FLOW_ON : FLOW_OFF);
                // Set value in data base
                attmdb_att_set_value(spss_env.shdl + SPSS_IDX_FLOW_CTRL_VAL, sizeof(flow), &flow);

                // Send notification to the client
                prf_server_send_event((prf_env_struct *)&(spss_env), false, spss_env.shdl + SPSS_IDX_FLOW_CTRL_VAL);
                spss_env.pending_ntf_cmp++;
            }
        }
        //send write response if required
        if (param->response==0x01)
        {
            atts_write_rsp_send(spss_env.con_info.conidx, param->handle, PRF_ERR_OK);
        }
    }
    return (KE_MSG_CONSUMED);
}


/*
 * TASK DESCRIPTOR DEFINITIONS
 ****************************************************************************************
 */
/// Disabled State handler definition.
const struct ke_msg_handler sps_server_disabled[] =
{
    {SPS_SERVER_CREATE_DB_REQ,      (ke_msg_func_t) sps_server_create_db_req_handler}
};

/// IDLE State handlers definition
const struct ke_msg_handler sps_server_idle[] =
{
    {SPS_SERVER_ENABLE_REQ,         (ke_msg_func_t)sps_server_enable_req_handler}
};

/// ACTIVE State handlers definition
const struct ke_msg_handler sps_server_active[] =
{
    {GAPC_DISCONNECT_IND,           (ke_msg_func_t)gapc_disconnect_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
    {GATTC_WRITE_CMD_IND,           (ke_msg_func_t)gattc_write_cmd_ind_handler},
    {SPS_SERVER_DATA_TX_REQ,        (ke_msg_func_t)sps_server_data_tx_req_handler},
    {SPS_SERVER_RX_FLOW_CTRL_REQ,   (ke_msg_func_t)sps_server_rx_flow_ctrl_req_handler},
}; 

/// Specifies the message handler structure for every input state
const struct ke_state_handler sps_server_state_handler[SPS_SERVER_STATE_MAX] =
{
    /// DISABLE State message handlers.
    [SPS_SERVER_DISABLED]  = KE_STATE_HANDLER(sps_server_disabled),
    /// IDLE State message handlers.
    [SPS_SERVER_IDLE]      = KE_STATE_HANDLER(sps_server_idle),
    /// ACTIVE State message handlers.
    [SPS_SERVER_ACTIVE]    = KE_STATE_HANDLER(sps_server_active),
};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler sps_server_default_handler = KE_STATE_HANDLER_NONE;

/// Defines the placeholder for the states of all the task instances.
ke_state_t sps_server_state[SPS_SERVER_IDX_MAX];

#endif /*#if BLE_SPS_SERVER*/

/// @} SPS_SERVER_TASK
