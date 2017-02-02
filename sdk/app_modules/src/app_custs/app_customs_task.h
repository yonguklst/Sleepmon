#ifndef __APP_CUSTOMS_TASK_H
#define __APP_CUSTOMS_TASK_H

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_CUSTOM_SERVER)

#include "ke_task.h"
#include "custs1_task.h"
#include "custs2_task.h"
/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

#if (BLE_CUSTOM1_SERVER)
/**
 ****************************************************************************************
 * @brief Handles CUSTOM Server profile database creation confirmation.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_custs1_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct custs1_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles disable indication from the CUSTOM Server profile.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_custs1_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct custs1_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);  

#endif //BLE_CUSTOM1_SERVER
#if (BLE_CUSTOM2_SERVER) 
 /**
 ****************************************************************************************
 * @brief Handles CUSTOM Server profile database creation confirmation.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_custs2_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct custs2_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles disable indication from the CUSTOM Server profile.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_custs2_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct custs2_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);  
#endif //BLE_CUSTOM2_SERVER
#endif // BLE_CUSTOM_SERVER
#endif // __APP_CUSTOMS_TASK_H
