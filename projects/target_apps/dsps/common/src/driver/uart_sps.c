/**
 ****************************************************************************************
 *
 * @file uart_spp.c
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

/**
 ****************************************************************************************
 * @addtogroup UART
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stddef.h>     // standard definition
#include "user_periph_setup.h"
#include "uart_sps.h"       // uart definition
#include "reg_uart.h"   // uart register
#include "hcic_eif.h"   // hci uart definition
#include "global_io.h"
#include "rwble_config.h"

/*newly added*/
#include "user_sps_scheduler.h"
#include "gpio.h"
/*
 * DEFINES
 *****************************************************************************************
 */
 
/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */
enum UART_ID
{
    MODEM_STAT         = 0,
    NO_INT_PEND        = 1,
    THR_EMPTY          = 2,
    RECEIVED_AVAILABLE = 4,    
    UART_TIMEOUT       = 12
};

/// RX_LVL values
enum UART_RXLVL
{
    UART_RXLVL_1,
    UART_RXLVL_4,
    UART_RXLVL_8,
    UART_RXLVL_14
};

/// WORD_LEN values
enum UART_WORDLEN
{
    UART_WORDLEN_5,
    UART_WORDLEN_6,
    UART_WORDLEN_7,
    UART_WORDLEN_8
};

/// FIFO_SZ values
enum UART_FIFOSIZE
{
    UART_FIFOSIZE_16,
    UART_FIFOSIZE_32,
    UART_FIFOSIZE_64,
    UART_FIFOSIZE_128
};

/*
 * STRUCT DEFINITIONS
 *****************************************************************************************
 */
/* TX and RX channel class holding data used for asynchronous read and write data
 * transactions
 */
/// UART TX RX Channel
struct uart_rxchannel
{
    uint32_t  size;                         // size
    uint8_t  *bufptr;                       // buffer pointer
    void (*callback) (uint8_t, uint32_t);   // data callback fcn ptr
    void (*flow_ctrl_cb) (bool);         // flow ctrlcall back fcn ptr
};
struct uart_txchannel
{
    uint32_t size;                          // size
    uint8_t *bufptr;                        // buffer pointer
    bool state;                             // flow control state
    bool flow_on_pending;                   // send flow on
    bool flow_off_pending;                  // send flow off
    void (*callback) (uint8_t);             // data callback fcn ptr
};

/// UART environment structure
struct uart_env_tag
{
    struct uart_txchannel tx;   // tx channel
    struct uart_rxchannel rx;   // rx channel
    uint8_t errordetect;        // error detect
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// uart environment structure
static struct uart_env_tag uart_sps_env __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void gpio0_callback(void);

/**
 ****************************************************************************************
 * @brief Serves the receive data available interrupt requests. It clears the requests and
 *        executes the callback function.
 *
 * @return void
 ****************************************************************************************
 */
static void uart_sps_rcv_data_avail_isr(void)
{
    uint8_t readData = 0;        //uart receive data will be read into this variable
    uint32_t rcv_bytes = 0;
    
    while (uart_data_rdy_getf() && uart_sps_env.rx.size)
    {
        // Read the received in the FIFO
        readData = uart_rxdata_getf();
#if (UART_SW_FLOW_ENABLED)
        if(readData == UART_XON_BYTE)
        {
            if(!uart_sps_env.tx.state)
            {
                void (*tx_callback) (uint8_t);
                
                //callback function will tell application that XON is received
                if(uart_sps_env.rx.flow_ctrl_cb != NULL)
                    uart_sps_env.rx.flow_ctrl_cb(true);
                
                uart_sps_env.tx.state = true;
                
                ///Reinitiate tx by calling tx callback
                // Retrieve callback pointer
                tx_callback = uart_sps_env.tx.callback;
                
                if(tx_callback != NULL)
                {
                    // Clear callback pointer
                    uart_sps_env.tx.callback = NULL;
                    
                    // Call handler
                    tx_callback(UART_STATUS_OK);
                }
                else
                {
                    ASSERT_ERR(0);
                }
            }
        }
        else if(readData == UART_XOFF_BYTE)
        {
            if(uart_sps_env.tx.state)
            {
                //callback function will tell application that XOFF is received
                if(uart_sps_env.rx.flow_ctrl_cb != NULL)
                    uart_sps_env.rx.flow_ctrl_cb(false);

                uart_sps_env.tx.state = false;
            }
        }
        else
#endif /*UART_SW_FLOW_ENABLED*/
        {
            //put data in buffer
            *uart_sps_env.rx.bufptr = readData;
            
            //update RX parameters
            uart_sps_env.rx.size--;
            rcv_bytes++;
            uart_sps_env.rx.bufptr++;
        }
    }
    if(rcv_bytes)
    {
        void (*rx_callback) (uint8_t, uint32_t);
        
        // Reset RX parameters
        uart_sps_env.rx.bufptr = NULL;
        
        // Retrieve callback pointer
        rx_callback = uart_sps_env.rx.callback;
        
        if(rx_callback != NULL)
        {
            // Clear callback pointer
            uart_sps_env.rx.callback = NULL;
            
            // Call handler
            rx_callback(UART_STATUS_OK, rcv_bytes);
        }
        else
        {
            ASSERT_ERR(0);
        }
    }
    else
    {
        uart_rec_data_avail_setf(1);
    }
}

/**
 ****************************************************************************************
 * @brief Serves the receive data error interrupt requests. It clears the requests and
 *        executes the callback function.
 *
 * @return void
 ****************************************************************************************
 */

static void uart_sps_rec_error_isr(void)
{
    void (*callback) (uint8_t, uint32_t) = NULL;
    // Reset RX parameters
    uart_sps_env.rx.size = 0;
    uart_sps_env.rx.bufptr = NULL;
    
    // Disable RX interrupt
    SetBits16(UART_IER_DLH_REG, ERBFI_dlh0, 0);
    
    // Retrieve callback pointer
    callback = uart_sps_env.rx.callback;

    if(callback != NULL)
    {
        // Clear callback pointer
        uart_sps_env.rx.callback = NULL;

        // Call handler
        callback(UART_STATUS_ERROR, NULL);
    }
    else
    {
        ASSERT_ERR(0);
    }
}

/**
 ****************************************************************************************
 * @brief Serves the transmit data fill interrupt requests. It clears the requests and
 *        executes the callback function. The callback function is called as soon as the
 *        last byte of the provided data is put into the FIFO. The interrupt is disabled
 *        at the same time.
 *
 * @return void
 ****************************************************************************************
 */
static void uart_sps_thr_empty_isr(void)
{
    void (*callback) (uint8_t);

    // Fill TX FIFO until there is no more room inside it
    while (uart_txfifo_full_getf())
    {
#if (UART_SW_FLOW_ENABLED)
        if(uart_sps_env.tx.flow_off_pending)
        {
            uart_txdata_setf(UART_XOFF_BYTE); //push UART XOFF byte
            uart_sps_env.tx.flow_off_pending = false;
        }
        else if (uart_sps_env.tx.flow_on_pending)
        {
            uart_txdata_setf(UART_XON_BYTE); //push UART XON byte
            uart_sps_env.tx.flow_on_pending = false;
        }
        else
#endif
        if (uart_sps_env.tx.size)
        {
            // Put a byte in the FIFO
            uart_txdata_setf(*uart_sps_env.tx.bufptr); 
            
            // Update TX parameters
            uart_sps_env.tx.size--;
            uart_sps_env.tx.bufptr++;
        }
        else
        {
            // Reset TX parameters
            uart_sps_env.tx.bufptr = NULL;
            
            // Disable TX interrupt
            uart_thr_empty_setf(0);
            
            if (uart_sps_env.tx.state)
            {
                // Retrieve callback pointer
                callback = uart_sps_env.tx.callback;
                
                if(callback != NULL)
                {
                    // Clear callback pointer
                    uart_sps_env.tx.callback = NULL;
                    
                    // Call handler
                    callback(UART_STATUS_OK);
                }
                else
                {
                    ASSERT_ERR(0);
                }
            }
            // Exit loop
            break;
        }
    }
}

/**
 ****************************************************************************************
 * @brief Check if RX FIFO is empty.
 *
 * @return FIFO empty      false: not empty  / true: empty
 *****************************************************************************************
 */
bool uart_sps_is_rx_fifo_empty(void)
{
    return (uart_data_rdy_getf() == 0);
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void uart_sps_init(uint8_t baudr, uint8_t mode )
{
    //Enable UART clock 
    SetBits16(CLK_PER_REG, UART1_ENABLE, 1);
	
    //ENABLE FIFO, REGISTER FCR IF UART_LCR_REG.DLAB=0
    SetBits16(UART_LCR_REG, UART_DLAB, 0);

    // XMIT FIFO RESET, RCVR FIFO RESET, FIFO ENABLED,
    SetWord16(UART_IIR_FCR_REG,0x87); //rcv int when rx fifo 1/2 full
    //SetWord16(UART_IIR_FCR_REG,0xC7); //rcv int when rx fifo 14/16 full   /* this option will cause the sps application to overflow the Rx FIFO */

    //DISABLE INTERRUPTS, REGISTER IER IF UART_LCR_REG.DLAB=0
    SetWord16(UART_IER_DLH_REG, 0);

    // ACCESS DIVISORLATCH REGISTER FOR BAUDRATE, REGISTER UART_DLH/DLL_REG IF UART_LCR_REG.DLAB=1
    SetBits16(UART_LCR_REG, UART_DLAB, 1);
    SetWord16(UART_IER_DLH_REG,0); 

    SetWord16(UART_IER_DLH_REG, (baudr&0xFF>>0x8));
    SetWord16(UART_RBR_THR_DLL_REG,baudr&0xFF); 

    // NO PARITY, 1 STOP BIT, 8 DATA LENGTH
    SetWord16(UART_LCR_REG, mode);

    //ENABLE TX INTERRUPTS, REGISTER IER IF UART_LCR_REG.DLAB=0
    SetBits16(UART_LCR_REG, UART_DLAB, 0);

    NVIC_DisableIRQ(UART_IRQn);
    NVIC_SetPriority(UART_IRQn,1);  
    NVIC_EnableIRQ(UART_IRQn);
    NVIC_ClearPendingIRQ(UART_IRQn);

#if (UART_HW_FLOW_ENABLED)
    GPIO_RegisterCallback(GPIO0_IRQn, gpio0_callback);
    if(GPIO_GetPinStatus(GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN) == false)
    {
        if(uart_sps_env.rx.flow_ctrl_cb != NULL)
            uart_sps_env.rx.flow_ctrl_cb(true);
        GPIO_EnableIRQ(GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN, GPIO0_IRQn, 0, 1, 0);
    }
    else if(GPIO_GetPinStatus(GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN) == true)
    {
        if(uart_sps_env.rx.flow_ctrl_cb != NULL)
            uart_sps_env.rx.flow_ctrl_cb(false);
        GPIO_EnableIRQ(GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN, GPIO0_IRQn, 1, 0, 0);
    }
#endif //UART_HW_FLOW_ENABLED


    // Configure UART environment
    uart_sps_env.errordetect = UART_ERROR_DETECT_DISABLED;
    uart_sps_env.rx.bufptr   = NULL;
    uart_sps_env.rx.flow_ctrl_cb = NULL;
    uart_sps_env.rx.size     = 0;

    uart_sps_env.tx.bufptr   = NULL;
    uart_sps_env.tx.state    = true;
    uart_sps_env.tx.size     = 0;

#if (UART_HW_FLOW_ENABLED)
    uart_sps_flow_off(false); 
#endif //UART_HW_FLOW_ENABLED

}

void uart_sps_flow_on(void)
{
#if (UART_HW_FLOW_ENABLED)
    // Configure modem (HW flow control enable)
    SetWord32(UART_MCR_REG, UART_AFCE|UART_RTS);
#endif //UART_HW_FLOW_ENABLED
#if (UART_SW_FLOW_ENABLED)
    if(!uart_thr_empty_getf())
    {
        uart_txdata_setf(UART_XON_BYTE);
    }
    else
    {
        uart_sps_env.tx.flow_on_pending = true;
        uart_sps_env.tx.flow_off_pending = false;
    }
#endif //UART_SW_FLOW_ENABLED
}

bool uart_sps_flow_off(bool force)
{
    bool flow_off = true;

    if(force)
    {
#if (UART_HW_FLOW_ENABLED)
        // Configure modem (HW flow control disable, 'RTS flow off')
        SetWord32(UART_MCR_REG, GetWord32(UART_MCR_REG) & (~UART_RTS)); 
#endif //UART_HW_FLOW_ENABLED
#if (UART_SW_FLOW_ENABLED)
        if(!uart_thr_empty_getf())
        {
            uart_txdata_setf(UART_XOFF_BYTE);
        }
        else
        {
            uart_sps_env.tx.flow_on_pending = false;
            uart_sps_env.tx.flow_off_pending = true;
        }
        flow_off = true;
#endif //UART_SW_FLOW_ENABLED
    }
    else do
    {
        // First check if no transmission is ongoing
        if ((uart_temt_getf() == 0) || (uart_thre_getf() == 0) || !uart_sps_is_rx_fifo_empty())
        {
            flow_off = false;
            break;
        }
#if (UART_HW_FLOW_ENABLED)
        // Configure modem (HW flow control disable, 'RTS flow off')
        SetWord32(UART_MCR_REG, GetWord32(UART_MCR_REG) & (~UART_RTS)); 
#endif //UART_HW_FLOW_ENABLED
#if (UART_SW_FLOW_ENABLED)
        uart_txdata_setf(UART_XOFF_BYTE);
#endif //UART_SW_FLOW_ENABLED
        
        // Wait for 1 character duration to ensure host has not started a transmission at the
        // same time
        for (int i=0;i<UART_WAIT_BYTE_COUNTER;i++);

        // Check if data has been received during wait time
        if(!uart_sps_is_rx_fifo_empty())
        {
            // Re-enable UART flow
            uart_sps_flow_on();

            // We failed stopping the flow
            flow_off = false;
        }
    } while(false);
    
    return (flow_off);
}

uint32_t uart_sps_get_tx_buffer_size(void)
{
    return uart_sps_env.tx.size;
}

void uart_sps_finish_transfers(void)
{
    // Configure modem (HW flow control disable, 'RTS flow off')
    uart_mcr_pack(UART_ENABLE,     // extfunc
                  UART_DISABLE,    // autorts
                  UART_ENABLE,     // autocts
                  UART_DISABLE);   // rts

    // Wait TX FIFO empty
    while(!uart_thre_getf() || !uart_temt_getf());
}

void uart_sps_read(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t, uint32_t))
{
    // Sanity check
    ASSERT_ERR(bufptr != NULL);
    ASSERT_ERR(size != 0);
    ASSERT_ERR(uart_sps_env.rx.bufptr == NULL);

    // Prepare RX parameters
    uart_sps_env.rx.size        = size;
    uart_sps_env.rx.bufptr      = bufptr;
    uart_sps_env.rx.callback    = callback;

    // Start data transaction
    uart_rec_data_avail_setf(1);
}

void uart_sps_write(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t))
{
   // Sanity check
    ASSERT_ERR(bufptr != NULL);
    ASSERT_ERR(size != 0);
    ASSERT_ERR(uart_sps_env.tx.bufptr == NULL);

    // Prepare TX parameters
    uart_sps_env.tx.size        = size;
    uart_sps_env.tx.bufptr      = bufptr;
    uart_sps_env.tx.callback    = callback; 

    uart_thr_empty_setf(1);
}

void uart_sps_register_flow_ctrl_cb(void (*callback) (bool))
{
    uart_sps_env.rx.flow_ctrl_cb = callback;
}

#ifdef __GNUC__
void __real_UART_Handler(void);
#else
void $Super$$UART_Handler(void);
#endif

#ifdef __GNUC__
void __wrap_UART_Handler(void)
#else
void $Sub$$UART_Handler(void)
#endif
{
    uint32_t idd;
        
    idd = 0x0F & GetWord32(UART_IIR_FCR_REG);
    if(idd!=NO_INT_PEND)
    {
        switch(idd)
        {
            case UART_TIMEOUT:
                if ((uart_sps_env.errordetect == UART_ERROR_DETECT_ENABLED) && uart_fifo_err_getf())
                {
                    uart_sps_rec_error_isr();
                }
                uart_sps_rcv_data_avail_isr();
                break;
            case RECEIVED_AVAILABLE:
                uart_sps_rcv_data_avail_isr();
                break;
            case THR_EMPTY:
                uart_sps_thr_empty_isr();
                break;
            default:
                break;
        }
    }
}

#if (UART_HW_FLOW_ENABLED)
void gpio0_callback(void)
{
    NVIC_DisableIRQ(GPIO0_IRQn);    // Disable this interrupt
    
    if(GPIO_GetPinStatus(GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN) == false)
    {
        //callback function will tell application that XON is received
        if(uart_sps_env.rx.flow_ctrl_cb != NULL)
            uart_sps_env.rx.flow_ctrl_cb(true);
        //Set interrupt to detect rising edge of CTS
        GPIO_EnableIRQ(GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN, GPIO0_IRQn, 0, 1, 0);
    }
    else if(GPIO_GetPinStatus(GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN) == true)
    {
        //callback function will tell application that XON is received
        if(uart_sps_env.rx.flow_ctrl_cb != NULL)
            uart_sps_env.rx.flow_ctrl_cb(false);
        //Set interrupt to detect falling edge of CTS
        GPIO_EnableIRQ(GPIO_UART1_CTS_PORT, GPIO_UART1_CTS_PIN, GPIO0_IRQn, 1, 0, 0);
    }
    NVIC_ClearPendingIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO0_IRQn);     // Enable this interrupt
}
#endif //UART_HW_FLOW_ENABLED

/**
 ****************************************************************************************
 * @brief Check rx or tx is ongoing.
 *
 * @return Activity      false: No activity / true: Tx or Rx
 *****************************************************************************************
 */

bool uart_sps_fifo_check()
{
    if ((uart_temt_getf() == 0) || (uart_thre_getf() == 0) || !uart_sps_is_rx_fifo_empty())
    {
        return true;
    }
    else
        return false;
}

/// @} UART

