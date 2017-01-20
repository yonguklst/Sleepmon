/**
 ****************************************************************************************
 *
 * @file AFE4400.h
 *
 * @brief PPG Sensor Driver
 *
 ****************************************************************************************
 */
 
#include "gpio.h"
#include "spi.h"
#include "DATA24.h"

#define AFE4400_CS_PORT             GPIO_PORT_2
#define AFE4400_CS_PIN                 GPIO_PIN_1

DATA24 AFE4400_RDATA(void);
   
void AFE44x0_Default_Reg_Init(void);

void AFE4400_WREG(uint8_t addr, uint32_t data);

