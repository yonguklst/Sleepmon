/**
 ****************************************************************************************
 *
 * @file user_apss.h
 *
 * @brief SPS Application header file
 *
 * Copyright (C) 2013. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef USER_SPSC_H_
#define USER_SPSC_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief SPS Application header file.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwble_config.h"

#if (BLE_SPS_CLIENT)

#include <stdint.h>          // standard integer definition

/*
 * STRUCTURES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable the the notifcation of the SPS or enable the host to set it in discovery mode
 ****************************************************************************************
 */
void user_sps_enable(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Send data over Bluetooth
 ****************************************************************************************
 */
void user_send_ble_data(const uint8_t *data, uint16_t length);

/**
 ****************************************************************************************
 * @brief Send flow control state over Bluetooth
 ****************************************************************************************
*/
void user_send_ble_flow_ctrl(uint8_t flow_ctrl);

#endif //BLE_SPS_CLIENT

/// @} APP

#endif // USER_SPSC_H_
