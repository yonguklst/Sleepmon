/**
****************************************************************************************
*
* @file user_sps_device.c
*
* @brief Empty peripheral template project source code.
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
#include "rwip_config.h"             // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_api.h"
#include "user_config.h"
#include "arch_console.h"
#include "gattc.h"
#include "user_sps_host.h"
#include "user_sps_scheduler.h"

enum process_event_response user_spsc_process_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id, 
                                         enum ke_msg_status_tag *msg_ret);

const struct ke_msg_handler user_main_process_handlers[]=
{
    {GAPC_PARAM_UPDATE_REQ_IND,         (ke_msg_func_t)gapc_param_update_req_ind_handler},
};

timer_hnd connection_timer;
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
/**
 ****************************************************************************************
 * @brief Sends gapm_start_scan_cmd message
 *
 * @return void
 ****************************************************************************************
 */
static void user_scan_start(void)
{
    struct gapm_start_scan_cmd* cmd = KE_MSG_ALLOC(GAPM_START_SCAN_CMD,
                            TASK_GAPM, TASK_APP,
                            gapm_start_scan_cmd);

    cmd->op.code = user_scan_conf.code;
    cmd->op.addr_src = user_scan_conf.addr_src;
    cmd->interval = user_scan_conf.interval;
    cmd->window = user_scan_conf.window;
    cmd->mode = user_scan_conf.mode;
    cmd->filt_policy = user_scan_conf.filt_policy;
    cmd->filter_duplic = user_scan_conf.filter_duplic;

    // Send the message
    ke_msg_send(cmd);

    // We are now connectable
    ke_state_set(TASK_APP, APP_CONNECTABLE);
}

/**
 ****************************************************************************************
 * @brief Sends app_gapm_cancel_msg message
 *
 * @return void
 ****************************************************************************************
 */
static void user_gapm_cancel(void)
{
    // Disable Advertising
    struct gapm_cancel_cmd *cmd = app_gapm_cancel_msg_create();
    // Send the message
    app_gapm_cancel_msg_send(cmd);
}

/**
 ****************************************************************************************
 * @brief Sets BLE Rx irq threshold. It should be called at app_init and in external wakeup irq callabck if deep sleep is used.
 * @param[in] Rx buffer irq threshold
 * @return void 
 ****************************************************************************************
 */
//static void app_set_rxirq_threshold(uint32_t rx_threshold)
//{
//    jump_table_struct[lld_rx_irq_thres] = (uint32_t) rx_threshold;
//}

/**
 ****************************************************************************************
 * @brief Sends a exchange MTU command
 * @param[in]   conidx Connection index
 * @return void
 ****************************************************************************************
 */
static void user_gattc_exc_mtu_cmd(uint8_t conidx)
{
  struct gattc_exc_mtu_cmd *cmd =  KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
            KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
            gattc_exc_mtu_cmd);
    
    cmd->req_type = GATTC_MTU_EXCH;
    
    ke_msg_send(cmd);
}

/*
 * APP TASK HANDLERS
 ****************************************************************************************
 */
enum process_event_response user_main_process_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id, 
                                         enum ke_msg_status_tag *msg_ret)
{
    return (app_std_process_event(msgid, param,src_id,dest_id,msg_ret, user_main_process_handlers,
                                         sizeof(user_main_process_handlers)/sizeof(struct ke_msg_handler)));
}

/**
 ****************************************************************************************
 * @brief Handles application events
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return void
 ****************************************************************************************
 */
void user_process_catch(ke_msg_id_t const msgid, void const *param,
                                         ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    int i=0;
    enum ke_msg_status_tag msg_ret;
    const process_event_func_t user_process_handlers[] = {
        (process_event_func_t) user_main_process_handler,
        (process_event_func_t) user_spsc_process_handler,
    };

    while (i<sizeof(user_process_handlers)/sizeof(process_event_func_t))
    {
        ASSERT_ERR(app_process_handlers[i]);
         if (user_process_handlers[i](msgid,param,dest_id,src_id, &msg_ret)==PR_EVENT_HANDLED)
               return;
         i++;
    }
}

/**
 ****************************************************************************************
 * @brief Handles update parameter event from the GAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_param_update_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_param_update_req_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    struct gapc_param_update_cfm* cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM, KE_BUILD_ID(TASK_GAPC, KE_IDX_GET(src_id)), TASK_APP,
                                                    gapc_param_update_cfm);
    
    cmd->accept = false;
    
    ke_msg_send(cmd);
    
    return (KE_MSG_CONSUMED);
}

/*
 * APP CALLBACKS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Handles connection event
 * @param[in]   connection_idx Connection index
 * @param[in]   param Parameters of connection
 * @return void
 ****************************************************************************************
 */
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    if (app_env[connection_idx].conidx != GAP_INVALID_CONIDX)
    {
        app_easy_timer_cancel(connection_timer);
        app_prf_enable (param->conhdl);
        user_gattc_exc_mtu_cmd(connection_idx);
        if ((user_default_hnd_conf.security_request_scenario==DEF_SEC_REQ_ON_CONNECT) && (BLE_APP_SEC))
        {
             app_easy_security_request(connection_idx);
        }
    }
    else
    {
        // No connection has been established, restart scanning
        user_scan_start();
    }
    arch_printf("Device connected\r\n");
}

/**
 ****************************************************************************************
 * @brief Handles disconnection event
 * @param[in]   param Parameters of disconnect message
 * @return void
 ****************************************************************************************
 */
void user_on_disconnect( struct gapc_disconnect_ind const *param )
{
    // Restart Scanning
    user_scan_start();
    arch_printf("Device disconnected\r\n");
}

/**
 ****************************************************************************************
 * @brief Called upon device cofiguration complete message
 * @return void
 ****************************************************************************************
 */
void user_on_set_dev_config_complete( void )
{
    // Add the first required service in the database
    if (app_db_init_start())
    {
        user_scan_start();
    }
}

/**
 ****************************************************************************************
 * @brief Called upon scan completion
 * @return void
 ****************************************************************************************
 */
void user_on_scanning_completed (uint8_t status)
{
    if(status == GAP_ERR_CANCELED)
    {
        app_easy_gap_start_connection_to();
        connection_timer = app_easy_timer(USER_CON_TIMEOUT, user_gapm_cancel);
    }
    else
    {
        user_scan_start();
    }
    return;
}

/**
 ****************************************************************************************
 * @brief Handles advertise reports
 * @param[in]   param Parameters of disconnect message
 * @return void
 ****************************************************************************************
 */
void user_on_adv_report_ind(struct gapm_adv_report_ind const * param)
{
    if(!memcmp(&param->report.data[3], USER_ADVERTISE_DATA, USER_ADVERTISE_DATA_LEN))
    {
        arch_printf("Connect with %02x %02x %02x %02x %02x %02x",
            param->report.adv_addr.addr[5],
            param->report.adv_addr.addr[4],
            param->report.adv_addr.addr[3],
            param->report.adv_addr.addr[2],
            param->report.adv_addr.addr[1],
            param->report.adv_addr.addr[0]);
    app_easy_gap_start_connection_to_set(param->report.adv_addr_type, (uint8_t *)&param->report.adv_addr.addr, MS_TO_DOUBLESLOTS(USER_CON_INTV));
    user_gapm_cancel();
    }
}

/**
 ****************************************************************************************
 * @brief Handles connection failure
 * @return void
 ****************************************************************************************
 */
void user_on_connect_failed (void)
{
    app_easy_timer_cancel(connection_timer);
    ASSERT_WARNING(0);
}

/*
 * MAIN LOOP CALLBACKS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Called on device initialization
 * @return void
 ****************************************************************************************
 */
void user_on_init(void)
{
    // SPS application initialisation
    user_scheduler_init();

    default_app_on_init();
}

/**
 ****************************************************************************************
 * @brief Called on system power up
 * @return true to force calling of schedule(), else false
 ****************************************************************************************
 */
enum arch_main_loop_callback_ret user_on_system_powered(void)
{
    user_sps_sleep_restore();
    user_ble_pull(true, NULL);

    return GOTO_SLEEP;
}

/**
 ****************************************************************************************
 * @brief Called before entering sleep
 * @return void
 ****************************************************************************************
 */
void user_before_sleep(void)
{
    user_sps_sleep_check();
}

/// @} APP
