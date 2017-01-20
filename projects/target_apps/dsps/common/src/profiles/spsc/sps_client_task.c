/**
 ****************************************************************************************
 *
 * @file sps_client_task.c
 *
 * @brief SPS client Task implementation.
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
 * @addtogroup SPS_CLIENT_TASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwble_config.h"

#if (BLE_SPS_CLIENT)

#include "sps_client_task.h"
#include "prf_utils.h"
#include "prf_utils_128.h"

/// State machine used to retrieve SPS characteristics information
const struct prf_char_def_128 spsc_sps_char[SPSC_CHAR_MAX] =
{
    [SPSC_SRV_TX_DATA_CHAR]    = {SPS_SERVER_TX_UUID, ATT_MANDATORY, ATT_CHAR_PROP_NTF},
    [SPSC_SRV_RX_DATA_CHAR]    = {SPS_SERVER_RX_UUID, ATT_MANDATORY, ATT_CHAR_PROP_WR_NO_RESP},
    [SPSC_FLOW_CTRL_CHAR]  = {SPS_FLOW_CTRL_UUID, ATT_MANDATORY, ATT_CHAR_PROP_WR_NO_RESP | ATT_CHAR_PROP_NTF}
};
/// State machine used to retrieve SPS service characteristic description information
const struct prf_char_desc_def spsc_sps_char_desc[SPSC_DESC_MAX] =
{
    ///SPS Client Config
    [SPSC_SRV_TX_DATA_CLI_CFG]      = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, SPSC_SRV_TX_DATA_CHAR},
    [SPSC_FLOW_CTRL_CLI_CFG]    = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, SPSC_FLOW_CTRL_CHAR},
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable the SPS client role, used after connection.
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_client_enable_req_handler(ke_msg_id_t const msgid,
                                    struct sps_client_enable_req const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status;
    
    // SPS Client Role Task Environment
    struct spsc_env_tag *spsc_env;
    // Connection Information
    struct prf_con_info con_info;

    // Fill the Connection Information structure
    con_info.conidx = gapc_get_conidx(param->conhdl);
    con_info.prf_id = dest_id;
    con_info.appid  = src_id;

    // Add an environment for the provided device
    status =  PRF_CLIENT_ENABLE(con_info, param, spsc);

    if (status == PRF_ERR_FEATURE_NOT_SUPPORTED)
    {
        // The message has been forwarded to another task id.
        return (KE_MSG_NO_FREE);
    }
    else if (status == PRF_ERR_OK)
    {
        spsc_env = PRF_CLIENT_GET_ENV(dest_id, spsc);

        // Discovery connection
        if (param->con_type == PRF_CON_DISCOVERY)
        { 
            // Start discovering LLS on peer
            uint8_t sps_data_service_uuid[]=SPS_SERVICE_UUID;
            spsc_env->last_uuid_req = ATT_DECL_PRIMARY_SERVICE;
            memcpy(spsc_env->last_svc_req, sps_data_service_uuid, ATT_UUID_128_LEN);
            prf_disc_svc_send_128(&spsc_env->con_info, spsc_env->last_svc_req);
            // Set state to discovering
            ke_state_set(dest_id, SPS_CLIENT_DISCOVERING);
        }
        else
        {
            //copy over data that has been stored
            spsc_env->sps     = param->sps;

            //send confirmation of enable request to application
            sps_client_enable_cfm_send(spsc_env, &con_info, PRF_ERR_OK);
        }
    }
    else
    {
        sps_client_enable_cfm_send(NULL, &con_info, status);
    }

    // message is consumed
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_DISC_SVC_IND message.
 * Get the start and ending handle of sps in the enviroment.
 * @param[in] msgid Id of the message received .
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance 
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */ 
static int gattc_disc_svc_ind_handler(ke_msg_id_t const msgid,
                                             struct gattc_disc_svc_ind const *param,
                                             ke_task_id_t const dest_id,
                                             ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct spsc_env_tag *spsc_env = PRF_CLIENT_GET_ENV(dest_id, spsc);

    // Even if we get multiple responses we only store 1 range
    spsc_env->sps.svc.shdl = param->start_hdl;
    spsc_env->sps.svc.ehdl = param->end_hdl;
    spsc_env->nb_svc++;

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_DISC_CHAR_IND message.
 * Characteristics for the currently desired service handle range are obtained and kept.
 * @param[in] msgid Id of the message received .
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance 
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_disc_char_ind_handler(ke_msg_id_t const msgid,
                                          struct gattc_disc_char_ind const *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct spsc_env_tag *spsc_env = PRF_CLIENT_GET_ENV(dest_id, spsc);

    
    prf_search_chars_128(spsc_env->sps.svc.ehdl, SPSC_CHAR_MAX,
            &spsc_env->sps.chars[0], &spsc_sps_char[0],
            param, &spsc_env->last_char_code);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_DISC_CHAR_DESC_IND message.
 * This event can be received 2-4 times, depending if measurement interval has seevral properties.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_disc_char_desc_ind_handler(ke_msg_id_t const msgid,
                                           struct gattc_disc_char_desc_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct spsc_env_tag *spsc_env = PRF_CLIENT_GET_ENV(dest_id, spsc);

    // Retrieve SPS descriptors
    prf_search_descs(SPSC_DESC_MAX, &spsc_env->sps.descs[0], &spsc_sps_char_desc[0],
                     param, spsc_env->last_char_code);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles @ref GATTC_CMP_EVT for GATTC_NOTIFY message meaning that
 * notification has been correctly sent to peer device (but not confirmed by peer device).
 * *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gattc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    // Get the address of the environment
    struct spsc_env_tag *spsc_env = PRF_CLIENT_GET_ENV(dest_id, spsc);
    uint8_t status;

    if(state == SPS_CLIENT_DISCOVERING)
    {
        if ((param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND)||(param->status == ATT_ERR_NO_ERROR))
        {
            //Currently discovering sps_server Service
            uint8_t sps_service_uuid[]=SPS_SERVICE_UUID;
            if (!memcmp(spsc_env->last_svc_req, sps_service_uuid, ATT_UUID_128_LEN))
            {
                if (spsc_env->last_uuid_req == ATT_DECL_PRIMARY_SERVICE)
                {
                    if (spsc_env->sps.svc.shdl == ATT_INVALID_HANDLE)
                    {
                        // stop discovery procedure.
                        sps_client_enable_cfm_send(spsc_env, &spsc_env->con_info, PRF_ERR_STOP_DISC_CHAR_MISSING);
                    }
                    // Too many services found only one such service should exist
                    else if(spsc_env->nb_svc > 1)
                    {
                        // stop discovery procedure.
                        sps_client_enable_cfm_send(spsc_env, &spsc_env->con_info, PRF_ERR_MULTIPLE_SVC);
                    }
                    else
                    {
                        // Discover SPS Device characteristics
                        prf_disc_char_all_send(&(spsc_env->con_info), &(spsc_env->sps.svc));

                        // Keep last UUID requested and for which service in env
                        spsc_env->last_uuid_req = ATT_DECL_CHARACTERISTIC;
                    }
                }
                else if (spsc_env->last_uuid_req == ATT_DECL_CHARACTERISTIC)
                {
                    status = prf_check_svc_char_validity_128(SPSC_CHAR_MAX, spsc_env->sps.chars, spsc_sps_char);
                    
                    // Check for characteristic properties.
                    if (status == PRF_ERR_OK)
                    {
                        spsc_env->last_uuid_req = ATT_INVALID_HANDLE;
                        spsc_env->last_char_code = spsc_sps_char_desc[SPSC_SRV_TX_DATA_CLI_CFG].char_code;

                        //Discover characteristic descriptor 
                        prf_disc_char_desc_send(&(spsc_env->con_info),
                                        &(spsc_env->sps.chars[spsc_env->last_char_code])); 
                    }
                    else
                    {
                        // Stop discovery procedure.
                        sps_client_enable_cfm_send(spsc_env, &spsc_env->con_info, status);
                    }
                }
                else //Descriptors
                {
                    //Get code of next char. having descriptors
                    spsc_env->last_char_code = spsc_get_next_desc_char_code(spsc_env, &spsc_sps_char_desc[0]);
                    if (spsc_env->last_char_code != SPSC_CHAR_MAX)
                    {
                        prf_disc_char_desc_send(&(spsc_env->con_info),
                                &(spsc_env->sps.chars[spsc_env->last_char_code]));
                    }
                    else
                    {
                         status = prf_check_svc_char_desc_validity(SPSC_DESC_MAX,
                                          spsc_env->sps.descs,
                                          spsc_sps_char_desc,
                                          spsc_env->sps.chars);

                        spsc_env->nb_svc = 0;
                        sps_client_enable_cfm_send(spsc_env, &spsc_env->con_info, status);
                    }
                }
            }
        }
        else
        {
            sps_client_enable_cfm_send(spsc_env, &spsc_env->con_info, param->status);
        }
    }
    else if (state == SPS_CLIENT_CONNECTED)
    {
        if(param->req_type == GATTC_WRITE_NO_RESPONSE)
        {
            ASSERT_WARNING(spsc_env->pending_wr_no_rsp_cmp > 0);
            spsc_env->pending_wr_no_rsp_cmp--;
            if (!spsc_env->pending_wr_no_rsp_cmp && spsc_env->pending_tx_ntf_cmp)
            {
                spsc_env->pending_tx_ntf_cmp = false;
                sps_client_confirm_data_tx(spsc_env, param->status);
            }
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Disconnection indication to SPS client profiel
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                        struct gapc_disconnect_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    PRF_CLIENT_DISABLE_IND_SEND(spsc_envs, dest_id, SPS_CLIENT,param->conhdl);
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_EVENT_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static int gattc_event_ind_handler(ke_msg_id_t const msgid,
                                        struct gattc_event_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct spsc_env_tag *spsc_env = PRF_CLIENT_GET_ENV(dest_id, spsc);
    
    if(spsc_env->sps.chars[SPSC_FLOW_CTRL_CHAR].val_hdl == param->handle)
    {
        //update ble flow control flag
        if(param->value[0] == FLOW_ON)
        {
            spsc_env->tx_flow_en = true;
            sps_client_indicate_tx_flow_ctrl(spsc_env, FLOW_ON);
            if (spsc_env->pending_tx_msg != NULL)
            {
                struct sps_client_data_tx_req const *pending_param = (struct sps_client_data_tx_req const *)spsc_env->pending_tx_msg->param;

                prf_gatt_write(&spsc_env->con_info, spsc_env->sps.chars[SPSC_SRV_RX_DATA_CHAR].val_hdl, (uint8_t *)pending_param->data,sizeof(uint8_t) * pending_param->length, GATTC_WRITE_NO_RESPONSE);
                spsc_env->pending_tx_ntf_cmp = true;
                spsc_env->pending_wr_no_rsp_cmp++;

                ke_msg_free(spsc_env->pending_tx_msg);
            }
            spsc_env->pending_tx_msg = NULL;

        }
        else if (param->value[0] == FLOW_OFF)
        {
            spsc_env->tx_flow_en = false;
            sps_client_indicate_tx_flow_ctrl(spsc_env, FLOW_OFF);
        }
    }
    else if (spsc_env->sps.chars[SPSC_SRV_TX_DATA_CHAR].val_hdl == param->handle)
    {
        sps_client_indicate_data_rx(spsc_env, param->value, param->length);
    }

    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief 
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_client_data_tx_req_handler(ke_msg_id_t const msgid,
                                        struct sps_client_data_tx_req const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct spsc_env_tag *spsc_env = PRF_CLIENT_GET_ENV(dest_id, spsc);
    
    if (ke_state_get(TASK_SPS_CLIENT)==SPS_CLIENT_CONNECTED)
    {
        if (spsc_env->tx_flow_en)
        {
            prf_gatt_write(&spsc_env->con_info, spsc_env->sps.chars[SPSC_SRV_RX_DATA_CHAR].val_hdl, (uint8_t *)param->data,sizeof(uint8_t) * param->length, GATTC_WRITE_NO_RESPONSE);
            spsc_env->pending_tx_ntf_cmp = true;
            spsc_env->pending_wr_no_rsp_cmp++;
        }
        else
        {
            spsc_env->pending_tx_msg = ke_param2msg(param);
            return (KE_MSG_NO_FREE);
        }
    }
    
    return (KE_MSG_CONSUMED);
}    
/**
 ****************************************************************************************
 * @brief Send flow control state @ref sps_client_send_notify_flow_control_state_req message.
 * @param[in] msgid Id of the message received 
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int sps_client_rx_flow_ctrl_req_handler(ke_msg_id_t const msgid,
                                   struct sps_client_rx_flow_ctrl_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    struct spsc_env_tag *spsc_env = PRF_CLIENT_GET_ENV(dest_id, spsc);
   
    // Send notification to the client
    prf_gatt_write(&spsc_env->con_info, spsc_env->sps.chars[SPSC_FLOW_CTRL_CHAR].val_hdl, (uint8_t*) &param->flow_control_state,sizeof (uint8_t),GATTC_WRITE_NO_RESPONSE);

    spsc_env->pending_wr_no_rsp_cmp++;

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

// CONNECTED State handlers definition.
const struct ke_msg_handler sps_client_connected[] =
{
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {SPS_CLIENT_DATA_TX_REQ,        (ke_msg_func_t)sps_client_data_tx_req_handler},
    {SPS_CLIENT_RX_FLOW_CTRL_REQ,   (ke_msg_func_t)sps_client_rx_flow_ctrl_req_handler},
};

// Specifies the Discovering state message handlers
const struct ke_msg_handler sps_client_discovering[] =
{
    {GATTC_DISC_CHAR_IND,           (ke_msg_func_t)gattc_disc_char_ind_handler},
    {GATTC_DISC_SVC_IND,            (ke_msg_func_t)gattc_disc_svc_ind_handler},
    {GATTC_DISC_CHAR_DESC_IND,      (ke_msg_func_t)gattc_disc_char_desc_ind_handler},
};

/// Default State handlers definition
const struct ke_msg_handler sps_client_default_state[] =
{
    {SPS_CLIENT_ENABLE_REQ,         (ke_msg_func_t)sps_client_enable_req_handler},
    {GAPC_DISCONNECT_IND,           (ke_msg_func_t)gapc_disconnect_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
};

// Specifies the message handler structure for every input state.
const struct ke_state_handler sps_client_state_handler[SPS_CLIENT_STATE_MAX] =
{
    [SPS_CLIENT_IDLE]        = KE_STATE_HANDLER_NONE,
    [SPS_CLIENT_CONNECTED]   = KE_STATE_HANDLER(sps_client_connected),
    [SPS_CLIENT_DISCOVERING] = KE_STATE_HANDLER(sps_client_discovering),
};

// Specifies the message handlers that are common to all states.
const struct ke_state_handler sps_client_default_handler = KE_STATE_HANDLER(sps_client_default_state);

// Defines the place holder for the states of all the task instances.
ke_state_t sps_client_state[SPS_CLIENT_IDX_MAX] __attribute__((section("exchange_mem_case1")));

#endif //BLE_SPS_CLIENT

/// @} SPS_CLIENTTASK
