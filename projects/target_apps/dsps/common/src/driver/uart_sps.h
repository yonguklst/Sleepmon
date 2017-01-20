/**
 ****************************************************************************************
 *
 * @file uart_spp.h
 *
 * @brief UART driver for SPP
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

#ifndef _UART_SPS_H_
#define _UART_SPS_H_

/**
 ****************************************************************************************
 * @defgroup UART UART
 * @ingroup DRIVERS
 *
 * @brief UART driver
 *
 * @{
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>       // integer definition
#include <stdbool.h>      // boolean definition
#include "uart.h"
#include "user_periph_setup.h"
/*
 * DEFINES
 *****************************************************************************************
 */
 
 /// Baud rate 115200, 57600, 38400, 19200, 9600
#define UART_SPS_BAUDRATE           CFG_UART_SPS_BAUDRATE
 
 /// Flow control bytes
#define UART_XON_BYTE               (uint8_t)0x11
#define UART_XOFF_BYTE              (uint8_t)0x13

/// HW/SW flow control on(1) or off(0)
#ifdef CFG_UART_SW_FLOW_CTRL
#define UART_SW_FLOW_ENABLED        1
#else
#define UART_SW_FLOW_ENABLED        0
#endif

#ifdef CFG_UART_HW_FLOW_CTRL
#define UART_HW_FLOW_ENABLED        1
#else
#define UART_HW_FLOW_ENABLED        0
#endif

#if (UART_SW_FLOW_ENABLED) && (UART_HW_FLOW_ENABLED)
    #error "Only one mode of data flow control should be used!"
#endif

#define UART_WAIT_BYTE_TIME             (UART_SPS_BAUDRATE * 10)            // time in uSec
#if UART_SW_FLOW_ENABLED
    #define UART_WAIT_BYTE_COUNTER          (UART_WAIT_BYTE_TIME * 8) + 30      //convert in for loop counter 
#else
#define UART_WAIT_BYTE_COUNTER          (UART_WAIT_BYTE_TIME * 4) + 30      //convert in for loop counter 
#endif

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initializes the UART to default values.
 *****************************************************************************************
 */
void uart_sps_init( uint8_t, uint8_t);

/**
 ****************************************************************************************
 * @brief Enable UART flow (hw).
 *****************************************************************************************
 */
void uart_sps_flow_on(void);

/**
 ****************************************************************************************
 * @brief Disable UART flow (hw).
 *****************************************************************************************
 */
bool uart_sps_flow_off(bool force);

/**
 ****************************************************************************************
 * @brief Returns size of TX buffer contents.
 *****************************************************************************************
 */
uint32_t uart_sps_get_tx_buffer_size(void);
/**
 ****************************************************************************************
 * @brief Finish current UART transfers
 *****************************************************************************************
 */
void uart_sps_finish_transfers(void);

/**
 ****************************************************************************************
 * @brief Starts a data reception.
 *
 * As soon as the end of the data transfer or a buffer overflow is detected,
 * the hci_uart_rx_done function is executed.
 *
 * @param[in,out]  bufptr Pointer to the RX buffer
 * @param[in]      size   Size of the expected reception
 *****************************************************************************************
 */
void uart_sps_read(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t, uint32_t));

/**
 ****************************************************************************************
 * @brief Starts a data transmission.
 *
 * As soon as the end of the data transfer is detected, the hci_uart_tx_done function is
 * executed.
 *
 * @param[in]  bufptr Pointer to the TX buffer
 * @param[in]  size   Size of the transmission
 *****************************************************************************************
 */
void uart_sps_write(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t));

void uart_sps_register_flow_ctrl_cb(void (*callback) (bool));

/**
 ****************************************************************************************
 * @brief Check if RX FIFO is empty.
 *****************************************************************************************
 */
bool uart_sps_is_rx_fifo_empty(void);

/**
 ****************************************************************************************
 * @brief Check rx or tx is ongoing.
 *****************************************************************************************
 */
bool uart_sps_fifo_check(void);

/// @} UART
#endif /* _UART_SPS_H_ */
