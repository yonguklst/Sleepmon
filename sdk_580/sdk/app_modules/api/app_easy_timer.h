/**
 ****************************************************************************************
 *
 * @file app_easy_timer.h
 *
 * @brief The easy timer api definitions.
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef _APP_EASY_TIMER_H_
#define _APP_EASY_TIMER_H_

/**
 ****************************************************************************************
 * @addtogroup APP_TIMER
 * @ingroup
 *
 * @brief
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "ke_msg.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

//Timer handler type
typedef uint8_t timer_hnd;

/*
 * DEFINES
 ****************************************************************************************
 */

//Value indicating an invalide timer operation
#define EASY_TIMER_INVALID_TIMER    (0x0)

/**
 ****************************************************************************************
 * @brief Create a new timer. Activate the ble if required.
 * @param[in] delay The amount of timer slots (10 ms) to wait
 * @param[in] fn The callback to be called when timer expires
 * @return The handler of the timer for future reference. If there are not timers available
 * EASY_TIMER_INVALID_TIMER will be returned.
 ****************************************************************************************
 */
timer_hnd app_easy_timer(const uint16_t delay, void(*fn)(void));

/**
 ****************************************************************************************
 * @brief Cancel an active timer.
 * @param[in] timerid The timer handler to cancel.
 * @return void
 ****************************************************************************************
 */
void app_easy_timer_cancel(const timer_hnd timer_id);

/**
 ****************************************************************************************
 * @brief Modify the delay of an existing timer.
 * @param[in] timerid The timer handler to modify
 * @param[in] delay The new delay value
 * @return The timer handler if everything is ok
 ****************************************************************************************
 */
timer_hnd app_easy_timer_modify(const timer_hnd timer_id, const uint16_t delay);

/**
 ****************************************************************************************
 * @brief Cancel all the active timers.
 * @return void
 ****************************************************************************************
 */
void app_easy_timer_cancel_all(void);

/// @} APP_TIMER

#endif // _APP_EASY_TIMER_H_
