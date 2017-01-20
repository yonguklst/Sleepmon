/**
 ****************************************************************************************
 *
 * @file app_proxr_task.c
 *
 * @brief Proximity Reporter application Task Implemantation.
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
 * @addtogroup APPTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"              // SW configuration

#if (BLE_APP_PRESENT)

#if (BLE_CUSTOM_SERVER)
#include "app_customs_task.h"
#include "custs1_task.h"
#include "custs2_task.h"
#include "app_task.h"           // Application Task API
#include "app_entry_point.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
#if (BLE_CUSTOM1_SERVER)
const struct ke_msg_handler app_custs1_process_handlers[]=
{
    {CUSTS1_CREATE_DB_CFM,                 (ke_msg_func_t)app_custs1_create_db_cfm_handler},
    {CUSTS1_DISABLE_IND,                   (ke_msg_func_t)app_custs1_disable_ind_handler},
};
#endif //BLE_CUSTOM1_SERVER

#if (BLE_CUSTOM2_SERVER)
const struct ke_msg_handler app_custs2_process_handlers[]=
{
    {CUSTS2_CREATE_DB_CFM,                 (ke_msg_func_t)app_custs2_create_db_cfm_handler},
    {CUSTS2_DISABLE_IND,                   (ke_msg_func_t)app_custs2_disable_ind_handler},
};
#endif //BLE_CUSTOM2_SERVER
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
#if (BLE_CUSTOM1_SERVER)
enum process_event_response app_custs1_process_handler(ke_msg_id_t const msgid,
                                                       void const *param,
                                                       ke_task_id_t const dest_id,
                                                       ke_task_id_t const src_id, 
                                                       enum ke_msg_status_tag *msg_ret)
{
    return (app_std_process_event(msgid, param, src_id, dest_id, msg_ret, app_custs1_process_handlers,
                                         sizeof(app_custs1_process_handlers)/sizeof(struct ke_msg_handler)));
}

int app_custs1_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct custs1_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
        // Inform the Application Manager
        struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                           TASK_APP, TASK_APP,
                                                           app_module_init_cmp_evt);

        cfm->status = param->status;

        ke_msg_send(cfm);
    }

    return (KE_MSG_CONSUMED);
}

int app_custs1_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct custs1_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}
#endif //BLE_CUSTOM1_SERVER

#if (BLE_CUSTOM2_SERVER)
enum process_event_response app_custs2_process_handler(ke_msg_id_t const msgid,
                                                       void const *param,
                                                       ke_task_id_t const dest_id,
                                                       ke_task_id_t const src_id, 
                                                       enum ke_msg_status_tag *msg_ret)
{
    return (app_std_process_event(msgid, param, src_id, dest_id, msg_ret, app_custs2_process_handlers,
                                         sizeof(app_custs2_process_handlers)/sizeof(struct ke_msg_handler)));
}

int app_custs2_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct custs2_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
        // Inform the Application Manager
        struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                           TASK_APP, TASK_APP,
                                                           app_module_init_cmp_evt);

        cfm->status = param->status;

        ke_msg_send(cfm);
    }

    return (KE_MSG_CONSUMED);
}

int app_custs2_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct custs2_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}

#endif // (BLE_CUSTOM2_SERVER)
#endif // (BLE_CUSTOM_SERVER)

#endif // (BLE_APP_PRESENT)

/// @} APPTASK
