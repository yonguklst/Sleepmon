/**
****************************************************************************************
*
* @file app_sps_scheduler.h
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

#ifndef USER_BUFFER_H_
#define USER_BUFFER_H_

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
 /*
 * DEFINES
 ****************************************************************************************
 */

/*
 * STRUCTURES
 ****************************************************************************************
 */
typedef struct {
    int     buffer_size;
    int     readIdx;        //points to first full
    int     writeIdx;       //points to first empty
    int     endIdx;         //points to element (full or empty) with maximal address
    uint8_t *data_ptr;
    int     low_watermark;
    int     high_watermark;
    bool    hwm_reached;
}RingBuffer;

 /*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief       function used to declare a new buffer
 ****************************************************************************************
 */
void user_buffer_create(RingBuffer* buffer, int buffer_size, int low_watermark, int high_watermark);

/**
 ****************************************************************************************
 * @brief       function used to check if buffer is initialized
 ****************************************************************************************
 */
uint8_t user_buffer_initialized(RingBuffer *buffer);

/**
 ****************************************************************************************
 * @brief function used to count the amount of items in buffer
 ****************************************************************************************
 */
int user_buffer_item_count(RingBuffer* buffer);

/**
****************************************************************************************
* @brief        function used to push multiple items to buffer
****************************************************************************************
*/
void user_buffer_write_items(RingBuffer* buffer, uint8_t* wrdata, uint16_t write_amount);

/**
****************************************************************************************
* @brief        function used to pull multiple items from buffer
****************************************************************************************
*/
uint16_t user_buffer_read_items(RingBuffer* buffer, uint8_t* rddata, uint16_t read_amount);

/**
 ****************************************************************************************
 * @brief Function used to calculate available items and return address of first item
 ****************************************************************************************
 */
uint16_t user_buffer_read_address(RingBuffer* buffer, uint8_t** rddata, uint16_t readAmount);

/**
 ****************************************************************************************
 * @brief Function used release items that were previously pulled with 
 ****************************************************************************************
 */
void user_buffer_release_items(RingBuffer* buffer, uint16_t readAmount);

/**
 ****************************************************************************************
 * @brief           Check if required space is available and return the starting address
 ****************************************************************************************
 */
uint16_t user_buffer_write_check (RingBuffer* buffer, uint8_t** wrdata, uint16_t max_write_amount);

/**
 ****************************************************************************************
 * @brief       Confirm bytes written in the buffer. Always used with the user_buffer_write_check
 ****************************************************************************************
 */
void user_buffer_cfm_write (RingBuffer* buffer, uint16_t actual_write_amount);

/**
 ****************************************************************************************
 * @brief       This function checks if the buffer is almost empty.
 ****************************************************************************************
 */
bool user_check_buffer_almost_empty(RingBuffer* buffer);

/**
 ****************************************************************************************
 * @brief       This function checks if the buffer is almost full.
 ****************************************************************************************
 */
bool user_check_buffer_almost_full(RingBuffer* buffer);

/// @} APP

#endif //USER_BUFFER_H_
