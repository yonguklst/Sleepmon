/**
 ****************************************************************************************
 *
 * @file sps_client.c
 *
 * @brief SPS transmit implementation.
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
 * @addtogroup SPS_CLIENT
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (BLE_SPS_CLIENT)
#include "sps_client.h"
#include "sps_client_task.h"
#include "prf_utils.h"
#include "prf_utils_128.h"


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

struct spsc_env_tag **spsc_envs __attribute__((section("exchange_mem_case1")));

static const struct ke_task_desc TASK_DESC_SPS_CLIENT = {sps_client_state_handler, &sps_client_default_handler, sps_client_state, SPS_CLIENT_STATE_MAX, SPS_CLIENT_IDX_MAX};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief sps_client_init  init sps_client
 * @param[in] void
 * @return void
 ****************************************************************************************
 */
void sps_client_init(void)
{
    // Reset all the profile role tasks
    PRF_CLIENT_RESET(spsc_envs, SPS_CLIENT);
}

/**
 ****************************************************************************************
 * @brief     When all handels received correctly it can send and receive data or flow control
 * @param[in] env_tag enviroment 
 * @param[in] con_inf Connection information
 * @param[in] status
 * @return    void
 ****************************************************************************************
 */
void sps_client_enable_cfm_send(struct spsc_env_tag *spsc_env, struct prf_con_info *con_info, uint8_t status)
{
    //format response to app
    struct sps_client_enable_cfm * cfm = KE_MSG_ALLOC(SPS_CLIENT_ENABLE_CFM,
                                                 con_info->appid, con_info->prf_id,
                                                 sps_client_enable_cfm);

    cfm->conhdl = gapc_get_conhdl(con_info->conidx);
    cfm->status = status;

    if (status == PRF_ERR_OK)
    {
        uint8_t flow;
        
        cfm->sps  = spsc_env->sps;
        
        // Register SPS_CLIENT task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(spsc_env->con_info), &(spsc_env->sps.svc));
        
        //Set value 0x0001 to CFG
        prf_gatt_write_ntf_ind(con_info, spsc_env->sps.descs[SPSC_SRV_TX_DATA_CLI_CFG].desc_hdl, PRF_CLI_START_NTF);
        prf_gatt_write_ntf_ind(con_info, spsc_env->sps.descs[SPSC_FLOW_CTRL_CLI_CFG].desc_hdl, PRF_CLI_START_NTF);
        
        //Initialise flow flags
        spsc_env->tx_flow_en = false;
        spsc_env->rx_flow_en = true;
        
        // Reset counter
        spsc_env->pending_wr_no_rsp_cmp = 0;
        spsc_env->pending_tx_ntf_cmp = false;

        flow = (spsc_env->rx_flow_en ? FLOW_ON : FLOW_OFF);
        prf_gatt_write(con_info, spsc_env->sps.chars[SPSC_FLOW_CTRL_CHAR].val_hdl, &flow, sizeof (uint8_t),GATTC_WRITE_NO_RESPONSE);
        spsc_env->pending_wr_no_rsp_cmp++;
        
        // Go to connected state
        ke_state_set(con_info->prf_id, SPS_CLIENT_CONNECTED);
    }
    else
    {
        PRF_CLIENT_ENABLE_ERROR(spsc_envs, con_info->prf_id, SPS_CLIENT);
    }
    
    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief Confirm that data has been sent
 * @param[in] env_tag enviroment 
 * @param[in] status
 * @return    void
 ****************************************************************************************
 */
void sps_client_confirm_data_tx(const struct spsc_env_tag *spsc_env, uint8_t status)
{
    struct sps_client_data_tx_cfm * cfm = KE_MSG_ALLOC(SPS_CLIENT_DATA_TX_CFM, spsc_env->con_info.appid,
                                                        TASK_SPS_CLIENT, sps_client_data_tx_cfm);

    cfm->conhdl = gapc_get_conhdl(spsc_env->con_info.conidx);
    cfm->status = status;

    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief  Send data to app
 * @param[in] env_tag   enviroment 
 * @param[in] data      pointer to data
 * @param[in] length    size of data
 * @return    void
 ****************************************************************************************
 */
void sps_client_indicate_data_rx(const struct spsc_env_tag *spsc_env, const uint8_t *data, uint16_t length)
{
    struct sps_client_data_rx_ind * ind = KE_MSG_ALLOC_DYN(SPS_CLIENT_DATA_RX_IND, spsc_env->con_info.appid,
                                                        TASK_SPS_CLIENT, sps_client_data_rx_ind, length);

    ind->conhdl = gapc_get_conhdl(spsc_env->con_info.conidx);
    ind->length = length;
    memcpy(ind->data, data, length);

    ke_msg_send(ind);
}

/**
 ****************************************************************************************
 * @brief  Send flow control state to app
 * @param[in] env_tag   enviroment 
 * @param[in] flow_ctrl 
 * @return    void
 ****************************************************************************************
 */
void sps_client_indicate_tx_flow_ctrl(const struct spsc_env_tag *spsc_env, uint8_t flow_ctrl)
{
    struct sps_client_tx_flow_ctrl_ind * ind = KE_MSG_ALLOC(SPS_CLIENT_TX_FLOW_CTRL_IND, spsc_env->con_info.appid,
                                                        TASK_SPS_CLIENT, sps_client_tx_flow_ctrl_ind);

    ind->conhdl = gapc_get_conhdl(spsc_env->con_info.conidx);
    ind->flow_control_state = flow_ctrl;

    ke_msg_send(ind);
}

/**
 ****************************************************************************************
 * @brief  Find next empty characteristic description
 * @param[in] env_tag   enviroment 
 * @param[in] desc_def  service characteristic description information table
 * @return    position of next characteristic description
 ****************************************************************************************
 */
uint8_t spsc_get_next_desc_char_code(struct spsc_env_tag *spsc_env,
                                     const struct prf_char_desc_def *desc_def)
{
    // Counter
    uint8_t i;
    uint8_t next_char_code;

    for (i=0; i<SPSC_DESC_MAX; i++)
    {
        next_char_code = desc_def[i].char_code;

        if (next_char_code > spsc_env->last_char_code)
        {
            //If Char. has been found and number of attributes is upper than 2
            if ((spsc_env->sps.chars[next_char_code].char_hdl != 0)
                    & (spsc_env->sps.chars[next_char_code].char_ehdl_off > 2))
            {
                return next_char_code;
            }
        }
    }

    return SPSC_CHAR_MAX;
}

#endif //BLE_SPS_CLIENT

/// @} SPS_CLIENT
