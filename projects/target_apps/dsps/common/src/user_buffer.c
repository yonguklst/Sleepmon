/**
****************************************************************************************
*
* @file user_sps_scheduler.c
*
* @brief SPS project source code.
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
#include <stdlib.h>

#include "user_buffer.h"
#include "ke_mem.h"

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#define DISABLE_IRQ
#ifdef DISABLE_IRQ
#define disable_irqs() GLOBAL_INT_DISABLE()
#define enable_irqs() GLOBAL_INT_RESTORE()
#else
#define disable_irqs() {}
#define enable_irqs() {}
#endif
/*
* GLOBAL VARIABLE DEFINITIONS
****************************************************************************************
*/

 /*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief       function used to declare a new buffer
 *
 * @param[in]   buffer_size     (how many items should the buffer store)
 * @param[in]   item_size       (how big should each item be)
 * @param[in]   high_watermark  (amount of items in buffer where flow off should be send)
 * @param[in]   low_watermark   (amount of items in buffer where flow on should be send)
 *
 * @return      RingBuffer pointer
 ****************************************************************************************
 */
void user_buffer_create(RingBuffer* buffer, int buffer_size, int low_watermark, int high_watermark)
{
    //initialise circular buffer
    buffer->buffer_size = buffer_size + 1;
    buffer->readIdx     = 0;
    buffer->writeIdx    = 0;
    buffer->endIdx      = buffer_size;
    
    //memory allocation of circular buffer
    buffer->data_ptr = ke_malloc((long)(buffer->buffer_size)*sizeof(uint8_t), KE_MEM_NON_RETENTION); 
    
    //flow control
    buffer->hwm_reached = false;
    
    //check to make sure that watermarks are valid
    if(high_watermark < buffer_size && 
        low_watermark <= high_watermark && 
        low_watermark > 0)
    {
        buffer->high_watermark = high_watermark;
        buffer->low_watermark  = low_watermark;
    }
    else
    {
        ASSERT_ERROR(0); //error: watermarks invalid
    }
    return;
}

/**
 ****************************************************************************************
 * @brief       function used to check if buffer is initialized
 *
 * @param[in]   buffer      (buffer to check)
 *
 * @return      TRUE(1) or FALSE(0)
 ****************************************************************************************
 */
uint8_t user_buffer_initialized(RingBuffer *buffer)
{
    if(buffer != NULL &&
        buffer->data_ptr != NULL)
    {
        return true;
    }
    return false;
}

/**
 ****************************************************************************************
 * @brief       function used to count the amount of items in buffer
 *
 * @param[in]   buffer      (buffer to check)
 *
 * @return      amount of items
 ****************************************************************************************
 */
int user_buffer_item_count(RingBuffer* buffer)
{
    int wr = buffer->writeIdx;
    int rd = buffer->readIdx;
    
    //count items in buffer
    if (rd > wr)
    {
        return buffer->endIdx - rd + 1 + wr;
    }
    else
    {
        return wr - rd;
    }
}

/**
****************************************************************************************
* @brief        function used to push multiple items to buffer
*
* @param[in]    buffer                  (buffer to push data)
* @param[in]    wrdata                  (pointer with data which should be pushed)
* @param[in]    write_amount            (amount of bytes which should be pushed to buffer)
*
* @return       none
****************************************************************************************
*/
void user_buffer_write_items(RingBuffer* buffer, uint8_t* wrdata, uint16_t write_amount)
{
    int readIdx, writeIdx;
    int overflow = 0;
    int roomLeft;
    // Critical section
    disable_irqs();

    readIdx = buffer->readIdx;
    writeIdx = buffer->writeIdx;

    // End of critical section
    enable_irqs();

    if (readIdx > writeIdx)
    {
        //buffer does not cycle, it can only reach read pointer - 1 
        roomLeft = readIdx - writeIdx - 1;
        write_amount = (write_amount > roomLeft ? roomLeft : write_amount);
    }
    else
    {
        roomLeft = buffer->buffer_size - writeIdx + readIdx - 1;
        write_amount = (write_amount > roomLeft ? roomLeft : write_amount);
        if (writeIdx + write_amount > buffer->buffer_size)
        {
            overflow = writeIdx + write_amount - buffer->buffer_size;
        }
        //app_write_items can use the entire buffer
        buffer->endIdx = buffer->buffer_size - 1;
    }
    memcpy(buffer->data_ptr + writeIdx, wrdata, write_amount - overflow);
    memcpy(buffer->data_ptr, wrdata + write_amount - overflow, overflow);

    writeIdx += write_amount;
    if(writeIdx >= buffer->buffer_size)
        writeIdx -= buffer->buffer_size;
    buffer->writeIdx = writeIdx;
}

/**
****************************************************************************************
* @brief        function used to pull multiple items from buffer
*
* @param[in]    buffer                  (buffer to pull from)
* @param[in]    rddata                  (pulled data)
* @param[in]    read_amount             (maximum items to pull)
*
* @return       items pulled
****************************************************************************************
*/
uint16_t user_buffer_read_items(RingBuffer* buffer, uint8_t* rddata, uint16_t read_amount)
{
    int readIdx, writeIdx, endIdx;
    int overflow = 0;
    int itemsAvail;
    // Critical section
    disable_irqs();

    readIdx = buffer->readIdx;
    writeIdx = buffer->writeIdx;
    endIdx = buffer->endIdx;

    // End of critical section
    enable_irqs();

    if (writeIdx >= readIdx)
    {
        itemsAvail = writeIdx - readIdx;
        read_amount = (read_amount > itemsAvail ? itemsAvail : read_amount);
    }
    else
    {
        itemsAvail = endIdx - readIdx + 1 + writeIdx;
        read_amount = (read_amount > itemsAvail ? itemsAvail : read_amount);
        if (readIdx + read_amount > endIdx + 1)
        {
            overflow = read_amount + readIdx - endIdx - 1;
        }
    }
    memcpy(rddata, buffer->data_ptr + readIdx, read_amount - overflow);
    memcpy(rddata + read_amount - overflow, buffer->data_ptr, overflow);

    readIdx += read_amount;
    if(readIdx >= endIdx + 1 && readIdx > writeIdx) //endIdx has a proper value only when writeIdx is lower than readIdx
        readIdx -= (endIdx + 1);
    
    buffer->readIdx = readIdx;
    return read_amount; //return the amount of items pulled from buffer
}

/**
****************************************************************************************
* @brief        function used to calculate available items and return address of first item
*
* @param[in]    buffer              (buffer to pull from)
* @param[out]   rddata              (pulled data)
* @param[in]    read_amount         (maximum items to pull)
*
* @return       amount of available items
****************************************************************************************
*/
uint16_t user_buffer_read_address(RingBuffer* buffer, uint8_t** rddata, uint16_t read_amount)
{
    int itemsAvail;
    // Critical section
    disable_irqs();

    if (buffer->writeIdx >= buffer->readIdx)
    {
        //Buffer limit is set by the write pointer
        itemsAvail = buffer->writeIdx - buffer->readIdx;
    }
    else
    {
        //Buffer limit is set by the end pointer
        itemsAvail = buffer->endIdx + 1 - buffer->readIdx;
    }
    //Limit requested bytes to available
    read_amount = (read_amount > itemsAvail ? itemsAvail : read_amount);
    //Return read pointer
    *rddata = buffer->data_ptr + buffer->readIdx;
    //Return the amount of items pulled from buffer
    // End of critical section
    enable_irqs();
    return read_amount;
}

/**
****************************************************************************************
* @brief        Function used release items that were previously pulled with user_buffer_read_address
*
* @param[in]    buffer              (buffer to pull from)
* @param[in]    read_amount         (items to pull as returned by user_buffer_read_address)
*
* @return       none
****************************************************************************************
*/
void user_buffer_release_items(RingBuffer* buffer, uint16_t read_amount)
{
    // Critical section
    disable_irqs();
    buffer->readIdx += read_amount;
    if(buffer->readIdx >= buffer->endIdx + 1 
            && buffer->readIdx > buffer->writeIdx) //endIdx has a proper value only when writeIdx is lower than readIdx
        buffer->readIdx -= (buffer->endIdx + 1);

    // End of critical section
    enable_irqs();
}

/**
 ****************************************************************************************
 * @brief           Check if required space is available and return the starting address
 *
 * @param[in]       buffer              (buffer to write to)
 * @param[out]      wrdata              (pointer of initial address)
 * @param[in]       max_write_amount    (required number of items to be written)
 *
 * @return          number of available items that can be written
 ****************************************************************************************
 */
uint16_t user_buffer_write_check (RingBuffer* buffer, uint8_t** wrdata, uint16_t max_write_amount)
{
    uint8_t write_amount;
    int roomLeft;
    // Critical section
    disable_irqs();

    if (max_write_amount == 0)
    {
        *wrdata = NULL;
        return 0;
    }
    if (buffer->readIdx > buffer->writeIdx)
    {
        //buffer does not cycle, it can only reach read pointer - 1
        roomLeft = buffer->readIdx - buffer->writeIdx - 1;
        *wrdata = buffer->data_ptr + buffer->writeIdx;
    }
    else
    {
        roomLeft = buffer->buffer_size - buffer->writeIdx - 1;

        if(max_write_amount > roomLeft)
        {
            //cycle to beggining of buffer and check if space available
            if(buffer->readIdx == 0)
            {
                roomLeft = 0;
                *wrdata = NULL;
            }
            else
            {
                roomLeft = buffer->readIdx - 1;
                *wrdata = buffer->data_ptr;
                buffer->endIdx = buffer->writeIdx - 1;
            }
        }
        else
        {
            *wrdata = buffer->data_ptr + buffer->writeIdx;
            buffer->endIdx = buffer->buffer_size - 1;
        }
    }
    write_amount = (max_write_amount > roomLeft ? roomLeft : max_write_amount);
    // End of critical section
    enable_irqs();
    return write_amount;
}

/**
 ****************************************************************************************
 * @brief       Confirm bytes written in the buffer. Always used with the user_buffer_write_check
 *
 * @param[in]   buffer                  (buffer to write to)
 * @param[in]   actual_write_amount     (actual number of items written)
 *
 * @return      none
 ****************************************************************************************
 */
void user_buffer_cfm_write (RingBuffer* buffer, uint16_t actual_write_amount)
{
    // Critical section
    disable_irqs();

    buffer->writeIdx += actual_write_amount;
    if(buffer->writeIdx >= buffer->endIdx + 1)
        buffer->writeIdx -= (buffer->endIdx + 1);

    // End of critical section
    enable_irqs();
}

/**
 ****************************************************************************************
 * @brief       This function checks if the buffer is almost empty.
 *
 * @param[in]   buffer          (buffer to check)
 *
 * @return      none
 ****************************************************************************************
 */
bool user_check_buffer_almost_empty(RingBuffer* buffer)
{
    // if less than low watermark
    if(user_buffer_item_count(buffer) <= buffer->low_watermark && buffer->hwm_reached == true)
    {
        buffer->hwm_reached = false;
        return true;
    }
    return false;
}

/**
 ****************************************************************************************
 * @brief       This function checks if the buffer is almost full.
 *
 * @param[in]   buffer          (buffer to check)
 *
 * @return      none
 ****************************************************************************************
 */
bool user_check_buffer_almost_full(RingBuffer* buffer)
{
    //if high watermark exceeded
    if(user_buffer_item_count(buffer) > buffer->high_watermark && buffer->hwm_reached == false)
    {
        buffer->hwm_reached = true;
        return true;
    }
    return false;
}

/// @} APP
