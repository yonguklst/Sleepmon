/**
 ****************************************************************************************
 *
 * @file app_security.c
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

#include "app_api.h"            // Application task Definition
#include "gapc_task.h"          // GAP Controller Task API Definition
#include "app_security_task.h"  // Application Security Task API Definition
#include "app_security.h"       // Application Security API Definition
#include <stdlib.h>

#include "user_callback_config.h"
#include "app_default_handlers.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Security Environment Structure
struct app_sec_env_tag app_sec_env[APP_EASY_MAX_ACTIVE_CONNECTION] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY


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

/**
 ****************************************************************************************
 * @brief Generates pin code
 *
 * @return pin code
 ****************************************************************************************
 */
uint32_t app_sec_gen_tk(void)
{
    // Generate a PIN Code (Between 100000 and 999999)
    return (100000 + (rand()%900000));
}

/**
 ****************************************************************************************
 * @brief Generates Long Term Key in app_sec_env
 *
 * @param[in] key_size     Generated key's size
 *
 * @return void
 ****************************************************************************************
 */
void app_sec_gen_ltk(uint8_t connection_id, uint8_t key_size)
{
    // Counter
    uint8_t i;
    app_sec_env[connection_id].key_size = key_size;

    // Randomly generate the LTK and the Random Number
    for (i = 0; i < RAND_NB_LEN; i++)
    {
        app_sec_env[connection_id].rand_nb.nb[i] = rand()%256;
    }

    // Randomly generate the end of the LTK
    for (i = 0; i < KEY_LEN; i++)
    {
        app_sec_env[connection_id].ltk.key[i] = (((key_size) < (16 - i)) ? 0 : rand()%256);
    }

    // Randomly generate the EDIV
    app_sec_env[connection_id].ediv = rand()%65536;
}



/// @} APP_SECURITY
