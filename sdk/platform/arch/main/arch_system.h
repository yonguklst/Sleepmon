/**
 ****************************************************************************************
 *
 * @file arch_system.h
 *
 * @brief Architecture System calls
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

#ifndef _ARCH_SYSTEM_H_
#define _ARCH_SYSTEM_H_

#include <stdbool.h>   // boolean definition
#include "datasheet.h"
#include "arch.h"
#include "arch_api.h"


extern uint32_t lp_clk_sel;
static __inline bool arch_clk_is_RCX20( void )
{
  return (((lp_clk_sel == LP_CLK_RCX20) && (CFG_LP_CLK == LP_CLK_FROM_OTP)) || (CFG_LP_CLK == LP_CLK_RCX20));
}

static __inline bool arch_clk_is_XTAL32( void )
{
 return (((lp_clk_sel == LP_CLK_XTAL32) && (CFG_LP_CLK == LP_CLK_FROM_OTP)) || (CFG_LP_CLK == LP_CLK_XTAL32));
}


/**
 ****************************************************************************************
 * @brief  Calibrate the RCX20 clock if selected from the user
 * @return void
 ****************************************************************************************
 */
static __inline  void rcx20_calibrate()
{
	if ( arch_clk_is_RCX20() )
	{
		calibrate_rcx20(20);
		read_rcx_freq(20);
	}
}

static __inline  void rcx20_read_freq()
{
	if ( arch_clk_is_RCX20() )
     read_rcx_freq(20);
}

/**
 ****************************************************************************************
 * @brief  Calibrate the RCX20 clock if selected from the user
 * @return void
 ****************************************************************************************
 */
static __inline  void xtal16__trim_init()
{
	#if USE_POWER_OPTIMIZATIONS
    	SetWord16(TRIM_CTRL_REG, 0x00); // ((0x0 + 1) + (0x0 + 1)) * 250usec but settling time is controlled differently
	#else
    	SetWord16(TRIM_CTRL_REG, XTAL16_TRIM_DELAY_SETTING); // ((0xA + 1) + (0x2 + 1)) * 250usec settling time
	#endif
    SetBits16(CLK_16M_REG, XTAL16_CUR_SET, 0x5);
}

#define DEFAULT_XTAL16M_TRIM_VALUE (1302)
/**
 ****************************************************************************************
 * @brief  Check that XTAL 16 MHz clock is calibrated.
 * @return void
 * About: Check if XTAL 16 MHz clock is calibrated and if not make corrective actions.
 ****************************************************************************************
 */
static __inline void  xtal16_calibration_check(void)
{
    if(DEFAULT_XTAL16M_TRIM_VALUE_USED)
    {
    // Apply the default XTAL16 trim value if a trim value has not been programmed in OTP
        if ( 0 == GetWord16(CLK_FREQ_TRIM_REG) )
        {
            set_xtal16m_trim_value(DEFAULT_XTAL16M_TRIM_VALUE);
        }
    }

}

void system_init(void);

#endif
/// @} 
