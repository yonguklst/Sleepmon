/**
 ****************************************************************************************
 *
 * @file ADS1292.c
 *
 * @brief ECG Sensor Driver
 *
 ****************************************************************************************
 */
 
#include "ADS1292.h"

uint8_t ADS1292R_Default_Register_Settings[12] =  // ADS1292R EVB set external clock 2.048 MHz
{                                                                       // Fuction Generator  +: ERL,ELL , -: ERA  OK    ,ELA any or don't care
                                                                        // Human              L: ERL,ELL , R: ERA  OK
                                                                        // research note book 2015.04.22 figure #7.
	0x00, // Address 0 : ID         : ID Conttrol Register (Factory-Programmed, Read Only)
	0x04, // Address 1 : CONFIG1    : Configuration Register 1
	0xE3, // Address 2 : CONFIG2    : Configuration Register 2
	0x10, // Address 3 : LOFF       : Lead-Off Control Register
	0x00, // Address 4 : CH1SET     : Channel 1 Settings
	0x00, // Address 5 : CH2SET     : Channel 2 Settings
	0x2C, // Address 6 : RLD_SENS   : Right Leg Drive Sense Selection
	0x0F, // Address 7 : LOFF_SENS  : Lead-Off Sense Selection
	0x80, // Address 8 : LOFF_STAT  : Ledad-Off Status
	0x02, // Address 9 : RESP1      : Respiration Control Register 1
	0x83, // Address A : RESP2      : Respiration Control Register 2
	0x0C  // Address B : GPIO       : General-Purpose I/O Register
};

DATA24 ADS1292R_RDATA()
{
	DATA24 sample;

	GPIO_SetInactive(ADS1292_CS_PORT, ADS1292_CS_PIN);
	spi_access(0x00);
	spi_access(0x00);
	spi_access(0x00);

	spi_access(0x00);
	spi_access(0x00);
	spi_access(0x00);
	
	
	sample.part[2]=spi_access(0x00);
	sample.part[1]=spi_access(0x00);
	sample.part[0]=spi_access(0x00);
	if(sample.part[2]>0x7F) sample.part[3]=0xFF;
	else sample.part[3]=0x00;
	
	GPIO_SetActive(ADS1292_CS_PORT, ADS1292_CS_PIN);
	
	return sample;
}

void ADS1292R_WREG_multi()
{
	int i=1;
	GPIO_SetInactive(ADS1292_CS_PORT, ADS1292_CS_PIN);
	spi_access(0x41);
	spi_access(10);
	for(i=1;i<11;i++) spi_access(ADS1292R_Default_Register_Settings[i]);
	GPIO_SetActive(ADS1292_CS_PORT, ADS1292_CS_PIN);
	 //systick_wait(500000);
}
uint8_t ADS1292R_RREG(uint8_t addr)
{
	uint8_t data,cmd;
	cmd=(0x2F&addr);
	GPIO_SetInactive(ADS1292_CS_PORT, ADS1292_CS_PIN);
	spi_access(cmd);
	spi_access(1);
	data = spi_access(0x00);
	GPIO_SetActive(ADS1292_CS_PORT, ADS1292_CS_PIN);
	return data;
}
void ADS1292R_sendmsg(uint8_t msg)
{
	GPIO_SetInactive(ADS1292_CS_PORT, ADS1292_CS_PIN);
	spi_access(msg);
	GPIO_SetActive(ADS1292_CS_PORT, ADS1292_CS_PIN);
	 //systick_wait(500000);
}

