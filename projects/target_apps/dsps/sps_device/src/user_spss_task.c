/**
****************************************************************************************
*
* @file  user_spss_task.c
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

#if (BLE_SPS_SERVER)

#include "user_spss_task.h"
#include "user_spss.h" 
#include "app_task.h"           // Application Task API
#include "sps_server.h"         // SPS functions
#include "sps_server_task.h"
//#include "user_sps_scheduler.h"

#include "arch_console.h"
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
#include "app_entry_point.h"
void system_on(void);
void system_off(void);
extern uint16_t resetcnt;
const struct ke_msg_handler user_spss_process_handlers[]=
{
    {SPS_SERVER_CREATE_DB_CFM,      (ke_msg_func_t)user_sps_create_db_cfm_handler},
    {SPS_SERVER_ENABLE_CFM,         (ke_msg_func_t)user_sps_server_enable_cfm_handler},
    {SPS_SERVER_DATA_TX_CFM,        (ke_msg_func_t)user_sps_server_data_tx_cfm_handler},
    {SPS_SERVER_DATA_RX_IND,        (ke_msg_func_t)user_sps_server_data_rx_ind_handler},
    {SPS_SERVER_TX_FLOW_CTRL_IND,   (ke_msg_func_t)user_sps_server_tx_flow_ctrl_ind_handler},
    {SPS_SERVER_ERROR_IND,          (ke_msg_func_t)user_sps_server_error_ind_handler},
};

enum process_event_response user_spss_process_handler(ke_msg_id_t const msgid,
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
 * @brief Handles start indication if the database is created
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int user_sps_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_server_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    // If state is not idle, ignore the message
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
        // Inform the Application Manager
        struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
            TASK_APP, TASK_APP, app_module_init_cmp_evt);
        
        cfm->status = param->status;
        
        ke_msg_send(cfm);
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles enable indication of the database
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int user_sps_server_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_server_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
//    user_ble_pull(false, false);
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
int user_sps_server_data_tx_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_server_data_tx_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    //if(param->status == GATT_ERR_NO_ERROR)
       // user_ble_pull(false, true);
    //else
//        user_ble_pull(false, false);
    
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
int user_sps_server_data_rx_ind_handler(ke_msg_id_t const msgid,
                                      struct sps_server_data_rx_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
		uint8_t val;
    memcpy( &val,  &param->data, 1 );

			if(val==0x01)
			{
					system_on();
			}
			else if(val==0x02)
			{
					system_off();
			}				
			else if(val==0x03)
			{
					resetcnt=60;
			}
			
	
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles server flow control state request indication
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int user_sps_server_tx_flow_ctrl_ind_handler(ke_msg_id_t const msgid,
                                      struct sps_server_tx_flow_ctrl_ind const *param,
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

/**
 ****************************************************************************************
 * @brief Handles server error indication
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int user_sps_server_error_ind_handler(ke_msg_id_t const msgid,
                                      struct prf_server_error_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    //if(param->status == PRF_ERR_DISCONNECTED)
    //ASSERT_WARNING(0);
		platform_reset(RESET_AFTER_SPOTA_UPDATE);
    return (KE_MSG_CONSUMED);
}
#endif //(BLE_SPS_SERVER)

#endif //(BLE_APP_PRESENT)

/// @} APP

