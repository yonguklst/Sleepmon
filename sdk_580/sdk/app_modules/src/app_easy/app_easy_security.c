/**
 ****************************************************************************************
 *
 * @file app_easy_security.c
 *
 * @brief Application Security Entry Point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_SECURITY
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#include "nvds.h"               // NVDS API Definition

#include "app_api.h"            // Application task Definition
#include "gapc_task.h"          // GAP Controller Task API Definition
#include <stdlib.h>

#include "user_callback_config.h"
#include "app_default_handlers.h"
#include "app_easy_security.h"
#include "app_mid.h"


/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_EASY_SECURITY_INLINE static inline
#define APP_EASY_SECURITY_MAX_CONNECTION APP_EASY_MAX_ACTIVE_CONNECTION


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

//--------------------------------------------------------------------------------   
static struct gapc_bond_cfm *gapc_bond_cfm_pairing_rsp[APP_EASY_SECURITY_MAX_CONNECTION] __attribute__((section("retention_mem_area0"),zero_init));
static struct gapc_bond_cfm *gapc_bond_cfm_tk_exch[APP_EASY_SECURITY_MAX_CONNECTION] __attribute__((section("retention_mem_area0"),zero_init));
static struct gapc_bond_cfm *gapc_bond_cfm_csrk_exch[APP_EASY_SECURITY_MAX_CONNECTION] __attribute__((section("retention_mem_area0"),zero_init));
static struct gapc_bond_cfm *gapc_bond_cfm_ltk_exch[APP_EASY_SECURITY_MAX_CONNECTION] __attribute__((section("retention_mem_area0"),zero_init));
static struct gapc_encrypt_cfm *gapc_encrypt_cfm[APP_EASY_SECURITY_MAX_CONNECTION] __attribute__((section("retention_mem_area0"),zero_init));
static struct gapc_security_cmd *gapc_security_req[APP_EASY_SECURITY_MAX_CONNECTION] __attribute__((section("retention_mem_area0"),zero_init));

static const struct gapc_pairing default_gapc_bond_cfm_pairing_rsp = {
			.oob            = GAP_OOB_AUTH_DATA_NOT_PRESENT,
			.key_size       = KEY_LEN,
			.iocap          = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
			.auth           = GAP_AUTH_REQ_NO_MITM_BOND,
			.sec_req        = GAP_NO_SEC,
			.ikey_dist      = GAP_KDIST_SIGNKEY,
			.rkey_dist      = GAP_KDIST_ENCKEY,
};

APP_EASY_SECURITY_INLINE struct gapc_bond_cfm* app_easy_security_pairing_rsp_create_msg(uint8_t connection_idx)
{
    // Allocate a message for GAP
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    if (gapc_bond_cfm_pairing_rsp[connection_idx]==NULL)
    {
        struct gapc_bond_cfm* cfm = app_gapc_bond_cfm_pairing_rsp_msg_create(connection_idx);
        gapc_bond_cfm_pairing_rsp[connection_idx] = cfm;
        if (USER_CONFIG)
        {
            cfm->data.pairing_feat.oob=user_security_configuration.oob;
            cfm->data.pairing_feat.key_size=user_security_configuration.key_size;
            cfm->data.pairing_feat.iocap=user_security_configuration.iocap;
            cfm->data.pairing_feat.auth=user_security_configuration.auth;
            cfm->data.pairing_feat.sec_req=user_security_configuration.sec_req;
            cfm->data.pairing_feat.ikey_dist=user_security_configuration.ikey_dist;
            cfm->data.pairing_feat.rkey_dist=user_security_configuration.rkey_dist;
        }
        else
            memcpy((void*)&(cfm->data.pairing_feat), (void*)&default_gapc_bond_cfm_pairing_rsp, sizeof(struct gapc_pairing));    
    }
    return (gapc_bond_cfm_pairing_rsp[connection_idx]);
}

struct gapc_bond_cfm* app_easy_security_pairing_rsp_get_active(uint8_t connection_idx)
{
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    return (app_easy_security_pairing_rsp_create_msg(connection_idx));
}
    
void app_easy_security_send_pairing_rsp( uint8_t connection_idx )
{
    struct gapc_bond_cfm* cmd;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    cmd = app_easy_security_pairing_rsp_create_msg(connection_idx);
    ke_msg_send(cmd);
    gapc_bond_cfm_pairing_rsp[connection_idx]=NULL;
    return;       
}

//-----------------------------------------------------------------
static const struct gap_sec_key default_gapc_bond_cfm_tk_exch = {
       .key={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
};

APP_EASY_SECURITY_INLINE struct gapc_bond_cfm* app_easy_security_tk_exch_create_msg(uint8_t connection_idx)
{
    // Allocate a message for GAP
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    if (gapc_bond_cfm_tk_exch[connection_idx]==NULL)
    {
        gapc_bond_cfm_tk_exch[connection_idx] = app_gapc_bond_cfm_tk_exch_msg_create(connection_idx);
        if (USER_CONFIG)
        {
            memcpy((void*)gapc_bond_cfm_tk_exch[connection_idx]->data.tk.key,user_security_configuration.tk.key,KEY_LEN);
        }
        else
            memcpy((void*)gapc_bond_cfm_tk_exch[connection_idx]->data.tk.key, (void*)default_gapc_bond_cfm_tk_exch.key, KEY_LEN);
    }
     return (gapc_bond_cfm_tk_exch[connection_idx]);
}

void app_easy_security_set_tk(uint8_t connection_idx, uint8_t *key, uint8_t keylen )
{
    struct gapc_bond_cfm* cfm;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    cfm = app_gapc_bond_cfm_tk_exch_msg_create(connection_idx);
    if (keylen<KEY_LEN)
        memcpy(cfm->data.tk.key,(void*)key, keylen);
}

struct gapc_bond_cfm* app_easy_security_tk_get_active(uint8_t connection_idx)
{
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    return (app_easy_security_tk_exch_create_msg(connection_idx));
}

void app_easy_security_tk_exch(uint8_t connection_idx )
{
    struct gapc_bond_cfm* cmd;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    cmd = app_easy_security_tk_exch_create_msg(connection_idx);
    ke_msg_send(cmd);
    gapc_bond_cfm_tk_exch[connection_idx]=NULL;
    return;       
    
}

//--------------------------------------------------------------------------------
static const struct gap_sec_key default_gapc_bond_cfm_csrk_exch = {
            .key={0xAB,0xAB,0x45,0x55,0x23,0x01,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
};

APP_EASY_SECURITY_INLINE struct gapc_bond_cfm* app_easy_security_csrk_exch_create_msg(uint8_t connection_idx)
{
    // Allocate a message for GAP
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    if (gapc_bond_cfm_csrk_exch[connection_idx]==NULL)
    {
        gapc_bond_cfm_csrk_exch[connection_idx] = app_gapc_bond_cfm_csrk_exch_msg_create(connection_idx );
        if (USER_CONFIG)
        {
            memcpy((void*)gapc_bond_cfm_csrk_exch[connection_idx]->data.csrk.key,user_security_configuration.csrk.key,KEY_LEN);
        }
        else
          memcpy((void*)gapc_bond_cfm_csrk_exch[connection_idx]->data.csrk.key, (void*)default_gapc_bond_cfm_csrk_exch.key, KEY_LEN);
    }
     return (gapc_bond_cfm_csrk_exch[connection_idx]);
}

struct gapc_bond_cfm* app_easy_security_csrk_get_active(uint8_t connection_idx)
{
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    return (app_easy_security_csrk_exch_create_msg(connection_idx));
}

void app_easy_security_csrk_exch(uint8_t connection_idx)
{
    struct gapc_bond_cfm* cfm;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    cfm=app_easy_security_csrk_exch_create_msg(connection_idx);
    ke_msg_send(cfm);
    gapc_bond_cfm_csrk_exch[connection_idx]=NULL;
    return; 
}
//-----------------------------------------------------------------

APP_EASY_SECURITY_INLINE struct gapc_bond_cfm* app_easy_security_bond_cfm_ltk_exch_create_msg(uint8_t connection_idx)
{
    // Allocate a message for GAP
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    if (gapc_bond_cfm_ltk_exch[connection_idx]==NULL)
    {
        gapc_bond_cfm_ltk_exch[connection_idx] = app_gapc_bond_cfm_ltk_exch_msg_create(connection_idx);
    }
     return (gapc_bond_cfm_ltk_exch[connection_idx]);
}

struct gapc_bond_cfm* app_easy_security_ltk_exch_get_active(uint8_t connection_idx)
{
   ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
   return (app_easy_security_bond_cfm_ltk_exch_create_msg(connection_idx));
};

void app_easy_security_ltk_exch(uint8_t connection_idx)
{
    struct gapc_bond_cfm* cfm;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    cfm=app_easy_security_bond_cfm_ltk_exch_create_msg(connection_idx);
    ke_msg_send(cfm);
    gapc_bond_cfm_ltk_exch[connection_idx]=NULL;
    return; 
}

void app_easy_security_set_ltk_exch_from_sec_env(uint8_t connection_idx)
{
    struct gapc_bond_cfm* cfm;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    cfm=app_easy_security_bond_cfm_ltk_exch_create_msg(connection_idx);    
    cfm->data.ltk.key_size = app_sec_env[connection_idx].key_size;
    cfm->data.ltk.ediv = app_sec_env[connection_idx].ediv;

    memcpy(&(cfm->data.ltk.randnb), &(app_sec_env[connection_idx].rand_nb) , RAND_NB_LEN);
    memcpy(&(cfm->data.ltk.ltk), &(app_sec_env[connection_idx].ltk) , KEY_LEN);
    return;
}

void app_easy_security_set_ltk_exch(uint8_t connection_idx, uint8_t* long_term_key, uint8_t encryption_key_size, \
                                            uint8_t* random_number, uint16_t encryption_diversifier)
{
    struct gapc_bond_cfm* cfm;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    cfm=app_easy_security_bond_cfm_ltk_exch_create_msg(connection_idx);    
    cfm->data.ltk.key_size = encryption_key_size;
    cfm->data.ltk.ediv = encryption_diversifier;

    memcpy(&(cfm->data.ltk.randnb), random_number , RAND_NB_LEN);
    memcpy(&(cfm->data.ltk.ltk), long_term_key , KEY_LEN);
    return;
}

//-----------------------------------------------------------------
APP_EASY_SECURITY_INLINE struct gapc_encrypt_cfm* app_easy_security_encrypt_cfm_create_msg(uint8_t connection_idx)
{
    // Allocate a message for GAP
    if (gapc_encrypt_cfm[connection_idx]==NULL)
    {
        gapc_encrypt_cfm[connection_idx] = app_gapc_encrypt_cfm_msg_create(connection_idx);
    }
     return (gapc_encrypt_cfm[connection_idx]);
}

void app_easy_security_encrypt_cfm( uint8_t connection_idx )
{
	struct gapc_encrypt_cfm* cfm;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    cfm = app_easy_security_encrypt_cfm_create_msg(connection_idx);
	ke_msg_send(cfm);
	gapc_encrypt_cfm[connection_idx] = NULL;
}

struct gapc_encrypt_cfm* app_easy_security_encrypt_cfm_get_active( uint8_t connection_idx )
{
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    return(app_easy_security_encrypt_cfm_create_msg(connection_idx));
}

bool app_easy_security_validate_encrypt_req_against_env (uint8_t connection_idx, struct gapc_encrypt_req_ind const *param)
{
    if(((app_sec_env[connection_idx].auth & GAP_AUTH_BOND) != 0)
            && (memcmp(&(app_sec_env[connection_idx].rand_nb), &(param->rand_nb), RAND_NB_LEN) == 0)
            && (app_sec_env[connection_idx].ediv == param->ediv))
		return (true);
	else
		return (false);
}
    
void app_easy_security_set_encrypt_req_valid ( uint8_t connection_idx )
{
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
	struct gapc_encrypt_cfm* cfm = app_easy_security_encrypt_cfm_create_msg(connection_idx);
        cfm->found = true;
        cfm->key_size = app_sec_env[connection_idx].key_size;
        memcpy(&(cfm->ltk), &(app_sec_env[connection_idx].ltk), KEY_LEN);
}

void app_easy_security_set_encrypt_req_invalid ( uint8_t connection_idx )
{
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
	struct gapc_encrypt_cfm* cfm = app_easy_security_encrypt_cfm_create_msg(connection_idx);
        cfm->found = false;
}


//--------------------------------------------------------------------------------------------------------------


APP_EASY_SECURITY_INLINE struct gapc_security_cmd* app_easy_security_request_create_msg(uint8_t connection_idx)
{
    // Allocate a message for GAP
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    if (gapc_security_req[connection_idx]==NULL)
    {
        if (USER_CONFIG)
            gapc_security_req[connection_idx] = app_gapc_security_request_msg_create(connection_idx,user_security_configuration.auth);
        else
            gapc_security_req[connection_idx] = app_gapc_security_request_msg_create(connection_idx,GAP_AUTH_REQ_NO_MITM_NO_BOND);
    }
     return (gapc_security_req[connection_idx]);
}

struct gapc_security_cmd* app_easy_security_request_get_active(uint8_t connection_idx)
{
     ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
     return(app_easy_security_request_create_msg(connection_idx));   
}

void app_easy_security_request(uint8_t connection_idx)
{
    struct gapc_security_cmd* req;
    ASSERT_WARNING(connection_idx<APP_EASY_SECURITY_MAX_CONNECTION);
    req=app_easy_security_request_get_active(connection_idx);
    ke_msg_send(req);
    gapc_security_req[connection_idx]=NULL;
    return; 
  
    
}


//-----------------------------------------------------------------

/// @} APP_SECURITY
