/**
****************************************************************************************
*
* @file user_sps_scheduler.h
*
* @brief SPS Project application header file.
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

#ifndef APP_SPS_SCHEDULER_H_
#define APP_SPS_SCHEDULER_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief SPS Application entry point.
 *
 * @{
 ****************************************************************************************
 */
 
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 #include <stdint.h>
 #include <stdbool.h>
 #include "user_buffer.h"
 /*
 * DEFINES
 ****************************************************************************************
 */
//application defines 
#define TX_CALLBACK_SIZE        (uint8_t)   16  //16 bytes messages
#define RX_CALLBACK_SIZE        (uint8_t)   8

#define TX_BUFFER_ITEM_COUNT    (int)       1800
#define TX_BUFFER_HWM           (int)       850     //47%
#define TX_BUFFER_LWM           (int)       650     //36%

#define RX_BUFFER_ITEM_COUNT    (int)       500
#define RX_BUFFER_HWM           (int)       350     //70%
#define RX_BUFFER_LWM           (int)       150     //30%

#define TX_SIZE                     (128)
#define TX_WAIT_LEVEL               (74)    //3 packets (20 + 2*27)
#define TX_WAIT_ROUNDS              (10)
#define TX_START_FRAME_DATA_SIZE    (20)
#define TX_CONTINUE_FRAME_DATA_SIZE (27)

#define UART_STATUS_INIT (UART_STATUS_ERROR +1)
/*
 * STRUCTURES
 ****************************************************************************************
 */

 /*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize buffers
 ****************************************************************************************
 */
void user_scheduler_init(void);

/**
 ****************************************************************************************
 * @brief push ble data to uart transmit buffer
 ****************************************************************************************
 */
void user_ble_push(uint8_t* wrdata, uint16_t write_amount);

/**
 ****************************************************************************************
 * @brief Generate messages and provide to stream queue.
 ****************************************************************************************
*/
void user_ble_pull (bool init, bool success);

/**
 ****************************************************************************************
 * Sleep Functions
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Restores the flow control when device wakes up
 ****************************************************************************************
*/
void user_scheduler_reinit(void);

/**
 ****************************************************************************************
 * @brief Check if device can go to sleep
 ****************************************************************************************
 */
void user_sps_sleep_check(void);

/**
 ****************************************************************************************
 * @brief Check peripheral flow control state before sleep and restores it
 ****************************************************************************************
 */
void user_sps_sleep_restore(void);

/// @} APP

#endif //APP_SPS_SCHEDULER_H_
