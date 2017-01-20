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

#if (BLE_SPS_SERVER)||(BLE_SPS_CLIENT)
#include "arch_console.h"
#include "user_sps_scheduler.h"
#include "user_buffer.h"
#include "user_periph_setup.h"            // peripheral configuration
#include "uart_sps.h"
#include "ke_mem.h"
#if (BLE_SPS_SERVER)
#include "user_spss.h"
#endif //BLE_SPS_SERVER
#if (BLE_SPS_CLIENT)
#include "user_spsc.h"
#endif //BLE_SPS_CLIENT
///Needed for sleep
#include "rwip.h"
#include "ke_env.h"
#include "ke.h"
#include "lld_sleep.h"

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
* GLOBAL VARIABLE DEFINITIONS
****************************************************************************************
*/

/// Flag to prevent multiple calls to the tx callback function
bool callbackbusy __attribute__((section("retention_mem_area0"),zero_init));  //@RETENTION MEMORY

/// SPS application buffer pointers
RingBuffer ble_to_periph_buffer __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
RingBuffer periph_to_ble_buffer __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

bool user_sps_sleep_flow_off __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

 /*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/// Private callback functions prototypes
static void uart_rx_callback(uint8_t res, uint32_t read_size);
static void uart_tx_callback(uint8_t res);
static void uart_flow_control_callback(bool flow_en);

/// Private application functions prototypes
static uint16_t user_periph_pull(uint8_t** rddata, uint16_t read_amount);
static void user_periph_push(uint8_t** wrdata, uint16_t write_amount);

static void user_override_ble_xoff(void);
static void user_override_ble_xon(void);


/**
 ****************************************************************************************
 * @brief       Initialise buffers and peripheral callbacks
 *
 * @return      none
 ****************************************************************************************
 */
void user_scheduler_init(void)
{
    if(!user_buffer_initialized(&periph_to_ble_buffer) && !user_buffer_initialized(&ble_to_periph_buffer))
    {
        //initialize buffers
        user_buffer_create(&ble_to_periph_buffer, TX_BUFFER_ITEM_COUNT, TX_BUFFER_LWM, TX_BUFFER_HWM);
        user_buffer_create(&periph_to_ble_buffer, RX_BUFFER_ITEM_COUNT, RX_BUFFER_LWM, RX_BUFFER_HWM);
    }
    // register a callback function for the flow control
    uart_sps_register_flow_ctrl_cb(&uart_flow_control_callback);

    uart_sps_init(UART_SPS_BAUDRATE, 3);

    // call read function once to initialize uart driver environment
    uart_rx_callback(UART_STATUS_INIT, NULL);
		
    uart_sps_flow_on();
}

/**
 ****************************************************************************************
 * @brief       receive callback function will handle uart data reception
 *
 * @param[in]   res (status: UART_STATUS_OK, UART_STATUS_ERROR, UART_STATUS_INIT)
 * @param[in]   read_size Size of data received
 *
 * @return      none
 ****************************************************************************************
 */
static void uart_rx_callback(uint8_t res, uint32_t read_size)
{
    uint8_t *periph_rx_ptr = NULL;
    //function called from uart receive isr
    switch (res)
    {
        case UART_STATUS_OK:
            user_periph_push(&periph_rx_ptr, read_size);
            break;
        case UART_STATUS_INIT:
            //request space in buffer
            if (user_buffer_write_check(&periph_to_ble_buffer, &periph_rx_ptr, RX_CALLBACK_SIZE) != RX_CALLBACK_SIZE)
            {
                ASSERT_ERROR(0);     //Buffer is too small
            }
            break;
        default:
            ASSERT_ERROR(0); //error: callback called from unknown source
    }
    
    //reinitiate callback
    uart_sps_read(periph_rx_ptr, RX_CALLBACK_SIZE, &uart_rx_callback);
}

/**
 ****************************************************************************************
 * @brief       transmit callback function will handle uart data transmission
 *
 * @param[in]   res (status: UART_STATUS_OK, UART_STATUS_ERROR, UART_STATUS_INIT)
 *
 * @return      none
 ****************************************************************************************
 */
static void uart_tx_callback(uint8_t res)
{
    static uint8_t size=0;
    uint8_t *periph_tx_ptr = NULL;
    //function gets called from uart transmit isr or application when its not running
    switch(res)
    {
        case UART_STATUS_OK:
            //get data and pointer
            size = user_periph_pull(&periph_tx_ptr, size);
            break;
        case UART_STATUS_INIT:
            size = user_buffer_read_address(&ble_to_periph_buffer, &periph_tx_ptr, TX_CALLBACK_SIZE);
            break;
        default:
            ASSERT_ERROR(0); //error: callback called from unknown source
    }
    //if there is data available, send data over periph
    if(size > 0)
    {
        uart_sps_write(periph_tx_ptr, size, &uart_tx_callback);
        return;
    }
    
    //there is no data in the buffer so the callback is done
    callbackbusy = false; 
}

/**
 ****************************************************************************************
 * @brief       callback function will handle uart flow control change of state
 *
 * @param[in]   flow_en true if data flow is enabled, false otherwise
 *
 * @return      none
 ****************************************************************************************
 */
static void uart_flow_control_callback(bool flow_en)
{
    if (flow_en)
        user_override_ble_xon();
    else
        user_override_ble_xoff();
}

/**
 ****************************************************************************************
 * @brief       pull data to transmit over UART
 *
 * @param[out]  rddata Pointer to data that will be transmitted by callback
 * @param[in]   read_amount How many bytes should be pulled
 *
 * @return      how many items are actually pulled
 ****************************************************************************************
 */
static uint16_t user_periph_pull(uint8_t** rddata, uint16_t read_amount)
{
    bool send_flow_on = false;
    uint8_t readcount = 0;


    //pull data
    user_buffer_release_items(&ble_to_periph_buffer, read_amount);
    readcount = user_buffer_read_address(&ble_to_periph_buffer, rddata, TX_CALLBACK_SIZE);
    //check if buffer is almost empty and send XON if neccesary
    send_flow_on = user_check_buffer_almost_empty(&ble_to_periph_buffer);
    
    //if XON should be send, make sure it's send as soon as possible
    if(send_flow_on)
    {
        user_send_ble_flow_ctrl(FLOW_ON);
    }

    return readcount;
}

/**
 ****************************************************************************************
 * @brief       push uart data to buffer and update flow control 
 *
 * @param[in]   wrdata          (data)
 * @param[in]   write_amount    (how much bytes should be stored)
 *
 * @return      none
 ****************************************************************************************
 */
static void user_periph_push(uint8_t** wrdata, uint16_t write_amount)
{
    bool send_flow_off = false;


    //write items to buffer
    user_buffer_cfm_write(&periph_to_ble_buffer, write_amount);
    if (user_buffer_write_check(&periph_to_ble_buffer, wrdata, RX_CALLBACK_SIZE) != RX_CALLBACK_SIZE)
    {
        ASSERT_WARNING(0);
    }
    
    //check if buffer is almost full and issue to send a XOFF if so
    send_flow_off = user_check_buffer_almost_full(&periph_to_ble_buffer);

    //make sure that XOFF is send as fast as possible
    if(send_flow_off)
    {
        uart_sps_flow_off(true);
    }
}

/**
 ****************************************************************************************
 * @brief       Push Bluetooth data to buffer or update flow control depending on packet
 *              type.
 *
 * @param[in]   wrdata          (data)
 * @param[in]   write_amount    (how much bytes should be stored)
 *
 * @return      none
 ****************************************************************************************
 */
void user_ble_push(uint8_t* wrdata, uint16_t write_amount)
{
    bool send_flow_off = false;
    
    //write items to buffer;
    user_buffer_write_items(&ble_to_periph_buffer, wrdata, write_amount);
    
    //check if buffer is almost full and issue to send a XOFF if so
    send_flow_off = user_check_buffer_almost_full(&ble_to_periph_buffer);
    
    //if XOFF must be send, send asap
    if(send_flow_off)
    {
        user_send_ble_flow_ctrl(FLOW_OFF);
    }
    //start transmitting
    __disable_irq();
    if(!callbackbusy)
    {
        callbackbusy = true;
        uart_tx_callback(UART_STATUS_INIT);
    }
    __enable_irq();
}

/**
 ****************************************************************************************
 * @brief       Checks if data available in buffer and initiates a transmittion.
 *
 * @return      none
 ****************************************************************************************
 */
void user_ble_pull (bool init, bool success)
{
    bool send_flow_on = false;
    static bool tx_busy_flag = false;
    static uint8_t rounds_waiting=0;
    static uint16_t length;

    if (init)
    {
        if(!tx_busy_flag)
        {
            if((length = user_buffer_item_count(&periph_to_ble_buffer)) > 0)
                if (length > TX_WAIT_LEVEL || (rounds_waiting++) >= TX_WAIT_ROUNDS)
                {
                    uint8_t *p_data = NULL;
                    
                    rounds_waiting = 0;
                    tx_busy_flag = true;

                    length = (length > TX_SIZE ? TX_SIZE : length);
                    length = (length < TX_START_FRAME_DATA_SIZE ? length : (((length - TX_START_FRAME_DATA_SIZE) / TX_CONTINUE_FRAME_DATA_SIZE) * TX_CONTINUE_FRAME_DATA_SIZE + TX_START_FRAME_DATA_SIZE));
                    length = user_buffer_read_address(&periph_to_ble_buffer, &p_data, TX_SIZE);
                    user_send_ble_data(p_data, length);
                }
        }
    }
    else
    {
        tx_busy_flag = false;
        if (success)
            user_buffer_release_items(&periph_to_ble_buffer, length);
        
        if((length = user_buffer_item_count(&periph_to_ble_buffer)) > TX_WAIT_LEVEL)
        {
            uint8_t *p_data = NULL;
            
            rounds_waiting = 0;
            tx_busy_flag = true;

            length = (length > TX_SIZE ? TX_SIZE : length);
            length = (((length - TX_START_FRAME_DATA_SIZE) / TX_CONTINUE_FRAME_DATA_SIZE) * TX_CONTINUE_FRAME_DATA_SIZE + TX_START_FRAME_DATA_SIZE);
            length = user_buffer_read_address(&periph_to_ble_buffer, &p_data, TX_SIZE);
            user_send_ble_data(p_data, length);
        }
    }
    //check if buffer is almost empty and send XON if neccesary
    send_flow_on = user_check_buffer_almost_empty(&periph_to_ble_buffer);
    
    //if XON should be send, make sure it's send as soon as possible
    if(send_flow_on)
    {
        uart_sps_flow_on();
    }
}

/**
 ****************************************************************************************
 * @brief Sets directly the uart flow control to xoff and sends state to connected device
 *
 * @return void.
 ****************************************************************************************
*/
static void user_override_ble_xoff(void)
{
    if(!ble_to_periph_buffer.hwm_reached)
    {
        user_send_ble_flow_ctrl(FLOW_OFF);
        ble_to_periph_buffer.hwm_reached = true;
    }
}

/**
 ****************************************************************************************
 * @brief Checks buffer level and sends xon to connected device
 *
 * @return void.
 ****************************************************************************************
*/
static void user_override_ble_xon(void)
{
    bool send_flow_on = false;
    
    send_flow_on = user_check_buffer_almost_empty(&ble_to_periph_buffer);
    if(send_flow_on)
    {
        user_send_ble_flow_ctrl(FLOW_ON);
    }
}

/**
 ****************************************************************************************
 * Sleep Functions
 ****************************************************************************************
 */

void user_scheduler_reinit(void)
{
    // register a callback function for the flow control
    uart_sps_register_flow_ctrl_cb(&uart_flow_control_callback);
    // call read function once to initialize uart driver environment
    if(user_buffer_initialized(&periph_to_ble_buffer))
    {
        uart_sps_init(UART_SPS_BAUDRATE, 3);

        uart_rx_callback(UART_STATUS_INIT, NULL);
    }
}

/**
 ****************************************************************************************
 * @brief  Performs a pre-check of the kernel an ble events in order to flow off serial interface. 
 *
 * @return sleep_mode_t Sleep mode
 ****************************************************************************************
 */
sleep_mode_t user_rwip_sleep_check(void)
{
    sleep_mode_t proc_sleep = mode_active;
   
    uint32_t sleep_duration = jump_table_struct[max_sleep_duration_external_wakeup_pos];//MAX_SLEEP_DURATION_EXTERNAL_WAKEUP;

#if (BLE_APP_PRESENT)
    if ( arch_ble_ext_wakeup_get() || (rwip_env.ext_wakeup_enable == 2) )  // sleep forever!
        sleep_duration = 0;
#else
#if (!EXTERNAL_WAKEUP) // sleep_duration will remain as it was set above....
    if (rwip_env.ext_wakeup_enable == 2)
        sleep_duration = 0;
#endif
#endif
    
    do
    {
        
        // Do not allow sleep if system is in startup period
        if (check_sys_startup_period())
            break;
        
        /************************************************************************
         **************            CHECK KERNEL EVENTS             **************
         ************************************************************************/
        // Check if some kernel processing is ongoing
        if (!ke_sleep_check())
            break;
        
        // Processor sleep can be enabled
        proc_sleep = mode_idle;
        
        /************************************************************************
         **************           CHECK RADIO POWER DOWN           **************
         ************************************************************************/
        // Check if BLE + Radio are still sleeping
        if(GetBits16(SYS_STAT_REG, RAD_IS_DOWN)) {
            // If BLE + Radio are in sleep return the appropriate mode for ARM
            proc_sleep = mode_sleeping;
            break;
        }

        /************************************************************************
         **************              CHECK RW FLAGS                **************
         ************************************************************************/
        // First check if no pending procedure prevents us from going to sleep
        if (rwip_prevent_sleep_get() != 0)
            break;

        /************************************************************************
         **************           CHECK EXT WAKEUP FLAG            **************
         ************************************************************************/
        /* If external wakeup is enabled, sleep duration can be set to maximum, otherwise
         * the system must be woken-up periodically to poll incoming packets from HCI */
        if((BLE_APP_PRESENT == 0) || (BLE_INTEGRATED_HOST_GTL == 1 )) // No need for periodic wakeup if we have full-hosted system
        {
            if(!rwip_env.ext_wakeup_enable)
                sleep_duration = jump_table_struct[max_sleep_duration_periodic_wakeup_pos]; // MAX_SLEEP_DURATION_PERIODIC_WAKEUP;
        }

        
        
        /************************************************************************
         *                                                                      *
         *                   CHECK DURATION UNTIL NEXT EVENT                    *
         *                                                                      *
         ************************************************************************/
        // If there's any timer pending, compute the time to wake-up to serve it
        if (ke_env.queue_timer.first != NULL)
            sleep_duration = jump_table_struct[max_sleep_duration_external_wakeup_pos];

        /************************************************************************
         **************            CHECK KERNEL TIMERS             **************
         ************************************************************************/
        // Compute the duration up to the next software timer expires
        if (!ke_timer_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;
        
        #if (BLE_EMB_PRESENT)
        /************************************************************************
         **************                 CHECK BLE                  **************
         ************************************************************************/
        // Compute the duration up to the next BLE event
        if (!lld_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;
        #endif // BLE_EMB_PRESENT
        
        proc_sleep = mode_sleeping;
     } while(0);
     
     return proc_sleep;
}

/**
 ****************************************************************************************
 * @brief Check if device can go to sleep
 *
 * @return void
 ****************************************************************************************
 */
void user_sps_sleep_check(void)
{
    if (app_default_sleep_mode == ARCH_EXT_SLEEP_ON)
        do
        {
            if (user_buffer_item_count(&ble_to_periph_buffer))
                break;
            if (user_buffer_item_count(&periph_to_ble_buffer))
                break;
            if (uart_sps_get_tx_buffer_size())
                break;
            if (uart_sps_fifo_check())
                break;
            if (user_rwip_sleep_check() != mode_sleeping)
                break;
            if (!uart_sps_flow_off(false))
                break;
            user_sps_sleep_flow_off = true;
            arch_set_extended_sleep();
            return;
        }while(0);
        
    arch_disable_sleep();
}

/**
 ****************************************************************************************
 * @brief Check peripheral flow control state before sleep and restores it
 *
 * @return void
 ****************************************************************************************
 */
void user_sps_sleep_restore(void)
{
    if (app_default_sleep_mode == ARCH_EXT_SLEEP_ON)
    {
        if (user_sps_sleep_flow_off)
        {
            if(periph_to_ble_buffer.hwm_reached == false)
            {
                uart_sps_flow_on();
            }
#if (UART_HW_FLOW_ENABLED)
            else
            {
                uart_sps_flow_off(true);
            }
#endif //UART_HW_FLOW_ENABLED
            
            user_sps_sleep_flow_off = false;
        }
    }
}

#endif //(BLE_SPS_SERVER)||(BLE_SPS_CLIENT)
/// @} APP
