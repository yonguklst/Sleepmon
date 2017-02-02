/**
 ****************************************************************************************
 *
 * @file AFE4400.c
 *
 * @brief PPG Sensor Driver
 *
 ****************************************************************************************
 */
 #include "AFE4400.h"
 
 const uint32_t 	AFE44xx_Default_Register_500Hz_Settings[49] = 
{
	0x00000,//CNTRL0
	6050,	 //LED2STC
	7998,   //LED2ENDC
	6000,   //LED2LEDSTC
	7999,   //LED2LEDENDC
	
	50,     //ALED2STC
	1998,     //ALED2ENDC
	
	2050,     //LED1STC
	3998,     //LED1ENDC
	2000,		 //LED1LEDSTC
	3999,		//LED1LEDENDC
	
	4050,		 //ALED1STC
	5998,   //ALED1ENDC
	
	4,					 //LED2CONVST
	1999,		 //LED2CONVEND
	
	2004,		 //ALED2CONVST
	3999,		 //ALED2CONVEND
	
	4004,		 //LED1CONVST 
	5999,	 //LED1CONVEND 
	
	6004,	 //ALED1CONVST
	7999,	 //ALED1CONVEND
	
	0,					 //ADCRSTSTCT0
	3,					 //ADCRSTENDCT0 
	2000,		 //ADCRSTSTCT1
	2003,		 //ADCRSTENDCT1
	4000,		 //ADCRSTSTCT2
	4003,		 //ADCRSTENDCT2
	6000,	 //ADCRSTSTCT3 
	6003,	 //ADCRSTENDCT3 
	
	7999,	 //PRPCOUNT
	
	0x00101,//CONTROL1
	0x00000,//SPARE1 
	0x00000,//TIAGAIN
	0x040FD,//TIA_AMB_GAIN 
	0x1000F,//LEDCNTRL 
	0x20100,//CONTROL2 
	0x00000,//SPARE2 
	0x00000,//SPARE3 
	0x00000,//SPARE4
	0x00000,//RESERVED1 
	0x00000,//RESERVED2
	0x00000,//ALARM 
	0x00000,//LED2VAL
	0x00000,//ALED2VAL 
	0x00000,//LED1VAL 
	0x00000,//ALED1VAL
	0x00000,//LED2-ALED2VAL
	0x00000,//LED1-ALED1VAL
	0x00000//DIAG
};

 DATA24 AFE4400_RDATA()
{
	DATA24 sample;

	GPIO_SetInactive(AFE4400_CS_PORT, AFE4400_CS_PIN);
	spi_access(0x2E);//afeaddress
	sample.part[2]=	spi_access(0x00);
	sample.part[1]= spi_access(0x00);
	sample.part[0]= spi_access(0x00);
	
	if(sample.part[2]>0x7F) sample.part[3]=0xFF;
	else sample.part[3]=0x00;
	
  GPIO_SetActive(AFE4400_CS_PORT, AFE4400_CS_PIN);//afecshigh
	
	return sample;
}

  
void AFE44x0_Default_Reg_Init()
{
    unsigned char Reg_Init_i;
    
    for ( Reg_Init_i = 0; Reg_Init_i < 49; Reg_Init_i++)
    {
        AFE4400_WREG(Reg_Init_i, AFE44xx_Default_Register_500Hz_Settings[Reg_Init_i]);
        
      //systick_wait(1);
    }
}

void AFE4400_WREG(uint8_t addr, uint32_t data)
{
	GPIO_SetInactive(AFE4400_CS_PORT, AFE4400_CS_PIN);//AFECS//afesclow
	spi_access(addr);
	spi_access((data>>16)&0xFF);
	spi_access((data>>8)&0xFF);
	spi_access((data)&0xFF);
	GPIO_SetActive(AFE4400_CS_PORT, AFE4400_CS_PIN);//afecshigh
}
