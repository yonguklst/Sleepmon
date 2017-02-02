/**
 ****************************************************************************************
 *
 * @file app_security.h
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

#ifndef _APP_SECURITY_H_
    #define _APP_SECURITY_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

    #include "rwip_config.h"
    #include "co_bt.h"
    #include "gap.h"
    #include "gapm.h"
    #include "gapc_task.h"
    //#include "app_easy_security.h"
    #include "app.h"


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

struct app_sec_env_tag
{
    // LTK
    struct gap_sec_key ltk;
    
    // Random Number
    struct rand_nb rand_nb;
    
    // EDIV
    uint16_t ediv;
    
    // LTK key size
    uint8_t key_size;

    // Last paired peer address type
    uint8_t peer_addr_type;
    
    // Last paired peer address
    struct bd_addr peer_addr;

    // authentication level
    uint8_t auth;

    // Current Peer Device NVDS Tag
    uint8_t nvds_tag;
};


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// Application Security Environment
extern struct app_sec_env_tag app_sec_env[APP_EASY_MAX_ACTIVE_CONNECTION];


/*
 * FUNCTIONS DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Generate pin code.
 * @return uint32_t
 ****************************************************************************************
 */
uint32_t app_sec_gen_tk(void);

/**
 ****************************************************************************************
 * @brief Generates Long Term Key in app_sec_env
 * @param[in] connection_id Connection Id
 * @param[in] key_size      Key size
 * @return void
 ****************************************************************************************
 */
void app_sec_gen_ltk(uint8_t connection_id, uint8_t key_size);


/// @} APP_SECURITY

#endif // _APP_SECURITY_H_
