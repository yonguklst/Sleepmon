/**
****************************************************************************************
*
* @file app_default_handlers.c
*
* @brief Default helper handlers implementing a primitive peripheral.
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


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration
#include "user_periph_setup.h"             // SW configuration
#include "wkupct_quadec.h"             // SW configuration

#include "user_profiles_config.h"
#include "arch_api.h"
#include "user_callback_config.h"
#include "co_math.h"                 // Common Maths Definition
#include "app_prf_types.h"
#include "app_prf_perm_types.h"

#include "nvds.h"                    // NVDS Definitions

#include "app_easy_security.h"
#include "spi_flash.h"
#include "arch_api.h"
#include "app.h"

#include "app_default_handlers.h"
#include "user_config.h"

#if BLE_CUSTOM_SERVER
#include "user_custs_config.h"
#endif
void system_off(void);
extern bool connected;
/**
 ****************************************************************************************
 * @brief The advertise operation used by the rest of default handlers.
 * @return void
 ****************************************************************************************
 */
void default_advertise_operation(void)
{

    if (user_default_hnd_conf.adv_scenario==DEF_ADV_FOREVER) app_easy_gap_undirected_advertise_start();
  //  else if (user_default_hnd_conf.adv_scenario==DEF_ADV_WITH_TIMEOUT(user_default_hnd_conf.advertise_period,NULL));
		connected=false;
		system_off();
		
}

void default_app_on_init(void)
{	
#if BLE_PROX_REPORTER
	app_proxr_init(GPIO_ALERT_LED_PORT, GPIO_ALERT_LED_PIN);
   
    alert_state.ll_alert_lvl = PROXR_ALERT_HIGH; //Link Loss default Alert Level 
    alert_state.txp_lvl = 0x00;                  // TX power level indicator  
#elif BLE_FINDME_LOCATOR
    app_findl_init();
#endif

#if BLE_BAS_SERVER
    app_batt_init();
#endif
    
#if BLE_DIS_SERVER        
    app_dis_init();
#endif

#if BLE_SPOTA_RECEIVER
    app_spotar_init();
#endif

    // Initialise service access write permitions for all the included profiles
    prf_init_srv_perm();
    
    arch_set_sleep_mode(app_default_sleep_mode);
}

void default_app_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    if (app_env[connection_idx].conidx != GAP_INVALID_CONIDX)
    {
        if (user_default_hnd_conf.adv_scenario == DEF_ADV_WITH_TIMEOUT)
        {
            app_easy_gap_advertise_with_timeout_stop();
        }

        app_prf_enable(param->conhdl);
        
        if ((user_default_hnd_conf.security_request_scenario == DEF_SEC_REQ_ON_CONNECT) && (BLE_APP_SEC))
        {
            app_easy_security_request(connection_idx);
        }
    }
    else
    {
        // No connection has been established, restart advertising
        EXECUTE_DEFAULT_OPERATION_VOID(default_operation_adv);
    }
    return;
}

void default_app_on_disconnect( struct gapc_disconnect_ind const *param ){
    // Restart Advertising
    EXECUTE_DEFAULT_OPERATION_VOID(default_operation_adv);
}

void default_app_on_set_dev_config_complete( void )
{
    // Add the first required service in the database
    if (app_db_init_start())
    {
        // No service to add in the DB -> Start Advertising
        EXECUTE_DEFAULT_OPERATION_VOID(default_operation_adv);
    }
 
    return;
}

void default_app_on_adv_undirect_complete(const uint8_t status)
{
    //ke_timer_clear(APP_ADV_TIMER, TASK_APP); 
    return; 
}

void default_app_on_db_init_complete( void )
{
    EXECUTE_DEFAULT_OPERATION_VOID(default_operation_adv);
    return;  
}

void default_app_on_pairing_request(uint8_t connection_idx, struct gapc_bond_req_ind const * param)
{
    app_easy_security_send_pairing_rsp(connection_idx);
    return;
}

void default_app_on_tk_exch_nomitm(uint8_t connection_idx, struct gapc_bond_req_ind const * param)
{
    uint32_t pin_code = app_sec_gen_tk();
    app_easy_security_set_tk( connection_idx, (uint8_t*) &pin_code, 4 );
    app_easy_security_tk_exch( connection_idx);
}

void default_app_on_csrk_exch(uint8_t connection_idx, struct gapc_bond_req_ind const *param)
{
    //provide the csrk to the host
    app_easy_security_csrk_exch( connection_idx );
}

void default_app_on_ltk_exch(uint8_t connection_idx, struct gapc_bond_req_ind const *param)
{
    // generate ltk and store it to sec_env
    app_sec_gen_ltk(connection_idx, param->data.key_size);
    //copy the parameters in the message
    app_easy_security_set_ltk_exch_from_sec_env(connection_idx);
    //send the message
    app_easy_security_ltk_exch(connection_idx);
    
}

void default_app_on_encrypt_req_ind(uint8_t connection_idx, struct gapc_encrypt_req_ind const *param)
{
	if (app_easy_security_validate_encrypt_req_against_env( connection_idx, param))
	{
        // update connection auth
        app_easy_gap_confirm(connection_idx, (enum gap_auth) app_sec_env[connection_idx].auth, GAP_AUTHZ_NOT_SET);
    
        app_easy_security_set_encrypt_req_valid ( connection_idx );
        
        app_easy_security_encrypt_cfm( connection_idx );
	}
	else
	{
        app_easy_security_set_encrypt_req_invalid  ( connection_idx );
    
        app_easy_security_encrypt_cfm( connection_idx );
    
        app_easy_gap_disconnect(connection_idx);
	}
}
