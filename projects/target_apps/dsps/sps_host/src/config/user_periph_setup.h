/**
 ****************************************************************************************
 *
 * @file user_periph_setup.h
 *
 * @brief Peripherals setup header file. 
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
 
#ifndef _USER_PERIPH_SETUP_H_
    #define _USER_PERIPH_SETUP_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

    #include "rwip_config.h"
    #include "global_io.h"
    #include "arch.h"
    #include "da1458x_periph_setup.h"
    #include "i2c_eeprom.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define CFG_UART_HW_FLOW_CTRL
#undef CFG_UART_SW_FLOW_CTRL

/// Available baud rates 115200, 57600, 38400, 19200, 9600
#define CFG_UART_SPS_BAUDRATE       UART_BAUDRATE_115K2

/* Enable WKUPCT. Required by wkupct_quadec driver. */
    #define WKUP_ENABLED

/****************************************************************************************/ 
/* UART configuration                                                                   */
/****************************************************************************************/  

    #define GPIO_UART1_TX_PORT   GPIO_PORT_0
    #define GPIO_UART1_TX_PIN    GPIO_PIN_4
    #define GPIO_UART1_RX_PORT   GPIO_PORT_0
    #define GPIO_UART1_RX_PIN    GPIO_PIN_5
    #define GPIO_UART1_RTS_PORT  GPIO_PORT_0
    #define GPIO_UART1_RTS_PIN   GPIO_PIN_3
    #define GPIO_UART1_CTS_PORT  GPIO_PORT_0
    #define GPIO_UART1_CTS_PIN   GPIO_PIN_2

/****************************************************************************************/ 
/* UART2 GPIO configuration                                                             */
/****************************************************************************************/ 

    #ifdef CFG_PRINTF_UART2
        #define  GPIO_UART2_TX_PORT     GPIO_PORT_1
        #define  GPIO_UART2_TX_PIN      GPIO_PIN_2
        
        #define  GPIO_UART2_RX_PORT     GPIO_PORT_1
        #define  GPIO_UART2_RX_PIN      GPIO_PIN_3
    #endif
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
void periph_init(void);

void GPIO_reservations(void);



#endif // _USER_PERIPH_SETUP_H_
