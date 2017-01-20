/**
 ****************************************************************************************
 *
 * @file arch_patch.h
 *
 * @brief ROM code patches.
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

#ifndef _ARCH_PATCH_H_
#define _ARCH_PATCH_H_
#include "rwip_config.h" 


// external function declarations
void patch_llm_task(void);
void patch_gtl_task(void);
void patch_llc_task(void);

#if ((BLE_APP_PRESENT == 0 || BLE_INTEGRATED_HOST_GTL == 1) && BLE_HOST_PRESENT )
    #define BLE_GTL_PATCH 1
#else 
	#define BLE_GTL_PATCH 0
#endif

#if (!(defined( __DA14581__)) && defined(CFG_MEM_LEAK_PATCH))
	#define BLE_MEM_LEAK_PATCH 1
#else
	#define BLE_MEM_LEAK_PATCH 0
#endif

#endif
/// @} 
