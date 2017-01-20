/**
 ****************************************************************************************
 *
 * @file app_security_task.h
 *
 * @brief Application Security Task Header file
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef _APP_SECURITY_TASK_H_
    #define _APP_SECURITY_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup APPSECTASK
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

    #include "rwip_config.h"

    #include "ke_task.h"        // Kernel Task
    #include "ke_msg.h"         // Kernel Message
    #include "gapc_task.h"      // GAP Controller Task API
    #include <stdint.h>         // Standard Integer


/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


int gapc_bond_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_bond_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int gapc_bond_ind_handler(ke_msg_id_t const msgid,
        struct gapc_bond_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

int gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_encrypt_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);
        
int gapc_encrypt_ind_handler(ke_msg_id_t const msgid,
        struct gapc_encrypt_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);

/// @} APPSECTASK

#endif // _APP_SECURITY_TASK_H_
