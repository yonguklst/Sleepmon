/**
 ****************************************************************************************
 *
 * @file ADS1292.h
 *
 * @brief ECG Sensor Driver
 *
 ****************************************************************************************
 */
 
#include "gpio.h"
#include "spi.h"
#include "DATA24.h"

#define ADS1292_CS_PORT             GPIO_PORT_2
#define ADS1292_CS_PIN                 GPIO_PIN_2

#define AD1292_WAKEUP 0x02
#define AD1292_STANDBY 0x04
#define AD1292_RESET 0x06
#define AD1292_START 0x08
#define AD1292_STOP 0x0A
#define AD1292_OFFSETCAL 0x1A
#define AD1292_RDATAC 0x10
#define AD1292_SDATAC 0x11


DATA24 ADS1292R_RDATA(void);

void ADS1292R_WREG_multi(void);
uint8_t ADS1292R_RREG(uint8_t addr);
void ADS1292R_sendmsg(uint8_t msg);

