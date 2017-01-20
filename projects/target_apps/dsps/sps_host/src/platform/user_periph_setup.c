/**
 ****************************************************************************************
 *
 * @file user_periph_setup.c
 *
 * @brief Peripherals setup and initialization. 
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
#include "rwip_config.h"             // SW configuration
#include "user_periph_setup.h"            // peripheral configuration
#include "global_io.h"
#include "gpio.h"
#include "uart.h"                    // UART initialization
#include "uart_sps.h"
#include "user_sps_scheduler.h"

/**
 ****************************************************************************************
 * @brief Each application reserves its own GPIOs here.
 *
 * @return void
 ****************************************************************************************
 */

#if DEVELOPMENT_DEBUG

void GPIO_reservations(void)
{
/*
* Globally reserved GPIOs reservation
*/

/*
* Application specific GPIOs reservation. Used only in Development mode (#if DEVELOPMENT_DEBUG)
    
i.e.  
    RESERVE_GPIO(DESCRIPTIVE_NAME, GPIO_PORT_0, GPIO_PIN_1, PID_GPIO);    //Reserve P_01 as Generic Purpose I/O
*/
    RESERVE_GPIO( UART1_TX, GPIO_UART1_TX_PORT, GPIO_UART1_TX_PIN, PID_UART1_TX);  
    RESERVE_GPIO( UART1_RX, GPIO_UART1_RX_PORT, GPIO_UART1_RX_PIN, PID_UART1_RX);
#if (UART_HW_FLOW_ENABLED) 
    RESERVE_GPIO( UART1_RTS, GPIO_UART1_RTS_PORT, GPIO_UART1_RTS_PIN, PID_UART1_RTSN);  
    RESERVE_GPIO( UART1_CTS, GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN, PID_UART1_CTSN);
#endif /*UART_HW_FLOW_ENABLED*/

#ifdef CFG_PRINTF_UART2
	RESERVE_GPIO( UART2_TX, GPIO_UART2_TX_PORT,  GPIO_UART2_TX_PIN, PID_UART2_TX);
	RESERVE_GPIO( UART2_RX, GPIO_UART2_RX_PORT,  GPIO_UART2_RX_PIN, PID_UART2_RX);
#endif
    
}
#endif //DEVELOPMENT_DEBUG

/**
 ****************************************************************************************
 * @brief Map port pins
 *
 * The Uart and SPI port pins and GPIO ports are mapped
 ****************************************************************************************
 */
void set_pad_functions(void)        // set gpio port function mode
{

/*
* Configure application ports.
i.e.    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false ); // Set P_01 as Generic purpose Output
*/    
    GPIO_ConfigurePin( GPIO_UART1_TX_PORT, GPIO_UART1_TX_PIN, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( GPIO_UART1_RX_PORT, GPIO_UART1_RX_PIN, INPUT_PULLUP, PID_UART1_RX, false );
#if (UART_HW_FLOW_ENABLED) 
    GPIO_ConfigurePin( GPIO_UART1_RTS_PORT, GPIO_UART1_RTS_PIN, OUTPUT, PID_UART1_RTSN, false );
    GPIO_ConfigurePin( GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN, INPUT_PULLUP, PID_UART1_CTSN, false );
#endif //UART_HW_FLOW_ENABLED

#ifdef CFG_PRINTF_UART2
    GPIO_ConfigurePin( GPIO_UART2_TX_PORT, GPIO_UART2_TX_PIN, OUTPUT, PID_UART2_TX, false );
    GPIO_ConfigurePin( GPIO_UART2_RX_PORT, GPIO_UART2_RX_PIN, INPUT, PID_UART2_RX, false );
#endif

}


/**
 ****************************************************************************************
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain is down. The Uart and SPi clocks are set.
 *
 * @return void
 ****************************************************************************************
 */
void periph_init(void) 
{
    // Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP)) ; 

    SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_ENABLE, 1);

    //rom patch
    patch_func();

    //Init pads
    set_pad_functions();

    if (app_default_sleep_mode == ARCH_EXT_SLEEP_ON)
        user_scheduler_reinit();

#ifdef CFG_PRINTF_UART2
    SetBits16(CLK_PER_REG, UART2_ENABLE, 1);
    uart2_init(UART_BAUDRATE_115K2, 3);
#endif
    
    // Enable the pads
    SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}
