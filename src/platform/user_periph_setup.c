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
#include "spi.h"
#include "dsp.h"
#include "DATA24.h"
#include "ADS1292.h"
#include "AFE4400.h"
#include "ecgtohr.h"
#include "wkupct_quadec.h"
#include "app_easy_timer.h" 
#include "arch_wdg.h"
#include "app_bass.h"
SPI_Pad_t spi_FLASH_CS_Pad;
SAMPLE chunkBuffer[20];
extern uint8_t cur_batt_level;
extern bool connected;
bool systemstat=false;
bool charging=false;
bool battcheck=false;
void user_send_ble(void);
void user_send_70(void);
void user_send_35(void);
void 	user_send_flow(void);
void system_on(void);
void system_off(void);

float n_ECG=0.0;
float n_ECG_pre=0.0;
float n_ECG_LPF=0.0;
float n_ECG_LPF_pre=0.0;
float n_ECG_HPF_pre=0.0;
float n_ECG_HPF=0.0;

float m_PPG = 0.0;
float m_PPG_pre = 0.0;
float m_PPG_LPF = 0.0;
float m_PPG_LPF_pre = 0.0;
float m_PPG_HPF = 0.0;
float m_PPG_HPF_pre = 0.0;

extern const int SAMPLING;// = 500;
extern const float SAMPLINGTIME;// = 1.0 / SAMPLING;

//uint16_t rri_pre;
uint16_t rri_ready=0;
uint16_t ldcnt=0;
uint16_t ldoff=0;
uint16_t leastbeep=0;
uint16_t resetcnt=14;
uint8_t samplemode=0;

float n_ECG_Ts = 0.004;
float n_ECG_HPF_set3Hz = 0.929824561; //0.053 / (0.053 + n_ECG_Ts);
float n_ECG_LPF_set30Hz = 0.430107527; //(n_ECG_Ts / n_ECG_Ts + 0.0053))

float m_PPG_Ts = 0.004;
float m_PPG_HPF_set1Hz =  0.975460123;//((0.159 / (0.159 + m_PPG_Ts))
float m_PPG_LPF_set10Hz = 0.201005025; //m_PPG_Ts / (m_PPG_Ts + 0.0159)
	
void butcb0()
{

	static uint8_t i=0;
	static uint8_t cnt=0;
//	static bool select=false;
	uint8_t i_divided;
	
		wdg_reload(WATCHDOG_DEFAULT_PERIOD);
		wdg_resume();   
	
		DATA24 ecg_raw,ppg_raw;
	

		
		if(samplemode==1)//250sps(2CH)
		{

			if(i%4==0)
			{	

				ecg_raw=ADS1292R_RDATA();
				ppg_raw=AFE4400_RDATA();
				
				n_ECG_pre = n_ECG;
				n_ECG = ecg_raw.data;
				
				m_PPG_pre = m_PPG;
				m_PPG =ppg_raw.data;

				// ECG High Pass Filter
				n_ECG_HPF_pre = n_ECG_HPF;
				n_ECG_HPF = (n_ECG_HPF_set3Hz * n_ECG_HPF_pre)
				+ n_ECG_HPF_set3Hz * (n_ECG - n_ECG_pre);	// HPF fc = 3 Hz
				
					// ECG Low Pass Filter
				n_ECG_LPF_pre = n_ECG_LPF;
				n_ECG_LPF = (1 - n_ECG_LPF_set30Hz) * n_ECG_LPF_pre 
				+ n_ECG_LPF_set30Hz * n_ECG_HPF_pre;	// LPF fc = 30 Hz

				
				m_PPG_HPF_pre = m_PPG_HPF;
				m_PPG_HPF = (m_PPG_HPF_set1Hz * m_PPG_HPF_pre)
				+ m_PPG_HPF_set1Hz * (m_PPG - m_PPG_pre);	// HPF fc = 1 Hz
				
				m_PPG_LPF_pre = m_PPG_LPF;
				m_PPG_LPF = (1 - m_PPG_LPF_set10Hz) * m_PPG_LPF_pre 
				+ m_PPG_LPF_set10Hz * m_PPG_HPF_pre;	// LPF fc = 10 Hz
				
				ecg_raw.data=((int)n_ECG_LPF);
				ppg_raw.data=((int)m_PPG_LPF);
		
				i_divided=i/4;
				
				chunkBuffer[i_divided].part[3]=ecg_raw.part[2];
				chunkBuffer[i_divided].part[2]=ecg_raw.part[1];
				chunkBuffer[i_divided].part[1]=ecg_raw.part[0];
				chunkBuffer[i_divided].part[6]=ppg_raw.part[2];
				chunkBuffer[i_divided].part[5]=ppg_raw.part[1];
				chunkBuffer[i_divided].part[4]=ppg_raw.part[0];
				chunkBuffer[i_divided].part[0]=cnt++;
			}
			i++;
			if(i>=20)
			{	
				//if(select)  user_send_flow();
				user_send_35();
				//select= !select;
				i=0;
			}
			
		}
		else// if(samplemode==0) //500sps
		{
			ecg_raw=ADS1292R_RDATA();
			ppg_raw=AFE4400_RDATA();
			n_ECG_pre = n_ECG;
			n_ECG = ecg_raw.data;

			// ECG High Pass Filter
			n_ECG_HPF_pre = n_ECG_HPF;
			n_ECG_HPF = (n_ECG_HPF_set3Hz * n_ECG_HPF_pre)
			+ n_ECG_HPF_set3Hz * (n_ECG - n_ECG_pre);	// HPF fc = 3 Hz
			
			n_ECG_LPF_pre = n_ECG_LPF;
			n_ECG_LPF = (1 - n_ECG_LPF_set30Hz) * n_ECG_LPF_pre 
			+ n_ECG_LPF_set30Hz * n_ECG_HPF_pre;	// LPF fc = 30 Hz
			
			m_PPG_pre = m_PPG;
			m_PPG =ppg_raw.data;
			
			m_PPG_HPF_pre = m_PPG_HPF;
				m_PPG_HPF = (m_PPG_HPF_set1Hz * m_PPG_HPF_pre)
				+ m_PPG_HPF_set1Hz * (m_PPG - m_PPG_pre);	// HPF fc = 1 Hz
			
			m_PPG_LPF_pre = m_PPG_LPF;
			m_PPG_LPF = (1 - m_PPG_LPF_set10Hz) * m_PPG_LPF_pre 
				+ m_PPG_LPF_set10Hz * m_PPG_HPF_pre;	// LPF fc = 10 Hz
			
			ecg_raw.data=((int)n_ECG_LPF);
			ppg_raw.data=((int)m_PPG_LPF);
			
			chunkBuffer[i].part[3]=ecg_raw.part[2];
			chunkBuffer[i].part[2]=ecg_raw.part[1];
			chunkBuffer[i].part[1]=ecg_raw.part[0];
			chunkBuffer[i].part[6]=ppg_raw.part[2];
			chunkBuffer[i].part[5]=ppg_raw.part[1];
			chunkBuffer[i].part[4]=ppg_raw.part[0];
			chunkBuffer[i].part[0]=cnt++;
			i++;
			if(i>=20)
			{	
				user_send_ble();
				i=0;
			}
		}
}


void butcb1()
{
	
	static uint8_t beepcnt=0;
	static bool beep=false;
	
	if(connected==false)
	{
			DATA24 ecg_raw;
			ecg_raw=ADS1292R_RDATA();
		
			GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_GPIO, true);//SPEAKER

			if(ldcnt>50 || cur_batt_level<10)
			{
				GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_GPIO, false);
				beep=false;
				beepcnt=0;
				system_off();
			}
			
			if(ecg_raw.data==0x007FFFFF) 
			{
				ldcnt++;
				leastbeep=0;
				
			}
			else 
			{
				ldcnt=0;
				leastbeep++;
			}

		n_ECG_pre = n_ECG;
		n_ECG = ecg_raw.data;
		/*				
		// ECG High Pass Filter
		n_ECG_HPF_pre = n_ECG_HPF;
		n_ECG_HPF = ((0.053 / (0.053 + SAMPLINGTIME)) * n_ECG_HPF_pre)
		+ (0.053 / (0.053 + SAMPLINGTIME)) * (n_ECG - n_ECG_pre);	// HPF fc = 3 Hz
		
			// ECG Low Pass Filter
		n_ECG_LPF_pre = n_ECG_LPF;
		n_ECG_LPF = (1 - (SAMPLINGTIME / (SAMPLINGTIME + 0.0053))) * n_ECG_LPF_pre 
		+ (SAMPLINGTIME / (SAMPLINGTIME + 0.0053)) * n_ECG_HPF_pre;	// LPF fc = 30 Hz
			
		m_PPG_pre = m_PPG;
		m_PPG =n_ECG_LPF;
		
		m_PPG_HPF_pre = m_PPG_HPF;
		m_PPG_HPF = ((0.053 / (0.053 + SAMPLINGTIME)) * m_PPG_HPF_pre)
		+ (0.053 / (0.053 + SAMPLINGTIME)) * (m_PPG - m_PPG_pre);	// HPF fc = 1 Hz
		
		m_PPG_LPF_pre = m_PPG_LPF;
		m_PPG_LPF = (1 - (SAMPLINGTIME / (SAMPLINGTIME + 0.0053))) * m_PPG_LPF_pre 
		+ (SAMPLINGTIME / (SAMPLINGTIME + 0.0053)) * m_PPG_HPF_pre;	// LPF fc = 10 Hz
	*/
			
			rri_ready = getRRI(n_ECG);

			if(rri_ready>=300 && rri_ready<=2000)
			{
				GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_GPIO, true);
				beep=true;
				rri_ready=0;
				leastbeep=0;
			}
			else if(beepcnt>50) 
			{
				GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_GPIO, false);
				beep=false;
				beepcnt=0;
			}
			if(beep) beepcnt++;
	
			wdg_reload(WATCHDOG_DEFAULT_PERIOD);
			wdg_resume();   
			GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_GPIO, false);//SPEAKER
		}
}

/**
 ****************************************************************************************
 * @brief Map port pins
 *
 * The Uart and SPI port pins and GPIO ports are mapped
 ****************************************************************************************
 */
void set_pad_functions(void)        // set gpio port function mode
{

	GPIO_ResetIRQ(GPIO0_IRQn);
	GPIO_ConfigurePin(ADS1292_CS_PORT, ADS1292_CS_PIN, OUTPUT, PID_GPIO, true);
  GPIO_ConfigurePin(AFE4400_CS_PORT, AFE4400_CS_PIN, OUTPUT, PID_GPIO, true);//ADSCS
	
	GPIO_ConfigurePin(SPI_GPIO_PORT, SPI_CS_PIN, OUTPUT, PID_GPIO, true);//FLASH
  GPIO_ConfigurePin(SPI_GPIO_PORT, SPI_CLK_PIN, OUTPUT, PID_SPI_CLK, false);
  GPIO_ConfigurePin(SPI_GPIO_PORT, SPI_DO_PIN, OUTPUT, PID_SPI_DO, false);
  GPIO_ConfigurePin(SPI_GPIO_PORT, SPI_DI_PIN, INPUT, PID_SPI_DI, false);
	spi_FLASH_CS_Pad.pin = SPI_CS_PIN;
  spi_FLASH_CS_Pad.port = SPI_GPIO_PORT;
	spi_init(&spi_FLASH_CS_Pad, SPI_MODE_8BIT, SPI_ROLE_MASTER, SPI_CLK_IDLE_POL_HIGH, SPI_PHA_MODE_1, SPI_MINT_DISABLE, SPI_XTAL_DIV_14);
	
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_0, INPUT, PID_ADC, false);//battadc
	GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_6, OUTPUT, PID_GPIO, battcheck);//battchk
	
	//if(connected) wdg_reload(WATCHDOG_DEFAULT_PERIOD);
	
	GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_8, OUTPUT, PID_GPIO, !connected);//LEDBT
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, true);//LED1
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_2, OUTPUT, PID_GPIO, true);//LED2
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_GPIO, true);//LED3
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_GPIO, true);//LED4
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_5, OUTPUT, PID_GPIO, true);//LED5
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_GPIO, true);//LED6
	
	GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_0, OUTPUT, PID_GPIO, connected);//afe shdn
	GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_1, OUTPUT, PID_GPIO, true);//ads RST
	
	//GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_2, INPUT, PID_GPIO, true);//DRDY
	GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_2, OUTPUT, PID_GPIO, false);//DRDY
	GPIO_ResetIRQ(GPIO0_IRQn);
	
	GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_5, OUTPUT, PID_GPIO, false);//AMPSHDN
	GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_GPIO, battcheck);//SPEAKER
	GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_3, INPUT, PID_GPIO, false);//CHG stat
	
}
//
ke_msg_id_t timer_used;	

void sleepcb()
{
	DATA24 leadoff;
	static int chgcnt=0;//,batcnt=0;
	static bool isled=true;
	app_easy_timer_cancel(timer_used);
	
	if(GPIO_GetPinStatus(GPIO_PORT_1, GPIO_PIN_3))
	{
		arch_disable_sleep();
		
		
		
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, true);//LED1
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_2, OUTPUT, PID_GPIO, true);//LED2
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_GPIO, true);//LED3
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_GPIO, true);//LED4
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_5, OUTPUT, PID_GPIO, true);//LED5
		if(cur_batt_level>100) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false);//LED1
		if(cur_batt_level>90) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_2, OUTPUT, PID_GPIO, false);//LED2
		if(cur_batt_level>70) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_GPIO, false);//LED3
		if(cur_batt_level>50) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_GPIO, false);//LED4
		if(cur_batt_level>20) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_5, OUTPUT, PID_GPIO, false);//LED5
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_GPIO, false);//LED6
		charging=true;
		if(chgcnt<=0)
		{
			battcheck=true;
			GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_GPIO, battcheck);//battcheck
			GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_6, OUTPUT, PID_GPIO, battcheck);
		}
		else 
		{
			chgcnt--;
			battcheck=false;
		}
		if(isled) isled=false;
		else isled=true;
		
		
		if(cur_batt_level>110) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false);//LED1
		else if(cur_batt_level>100) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, isled);//LED1
		else if(cur_batt_level>90) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_2, OUTPUT, PID_GPIO, isled);//LED2
		else if(cur_batt_level>70) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_GPIO, isled);//LED3
		else if(cur_batt_level>50) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_GPIO, isled);//LED4
		else if(cur_batt_level>20) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_5, OUTPUT, PID_GPIO, isled);//LED5
		else GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_GPIO, isled);//LED6
	}
	else if(battcheck)
	{
		app_batt_lvl();
 		
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, true);//LED1
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_2, OUTPUT, PID_GPIO, true);//LED2
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_GPIO, true);//LED3
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_GPIO, true);//LED4
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_5, OUTPUT, PID_GPIO, true);//LED5
		if(cur_batt_level>100) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false);//LED1
		if(cur_batt_level>90) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_2, OUTPUT, PID_GPIO, false);//LED2
		if(cur_batt_level>70) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_GPIO, false);//LED3
		if(cur_batt_level>50) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_GPIO, false);//LED4
		if(cur_batt_level>20) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_5, OUTPUT, PID_GPIO, false);//LED5
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_GPIO, false);//LED6
		
		battcheck=false;
		GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_6, OUTPUT, PID_GPIO, battcheck);
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_GPIO, battcheck);//battcheck
		
		chgcnt=5;
		
		if(isled) isled=false;
		else isled=true;
		
		if(cur_batt_level>110) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false);//LED1
		else if(cur_batt_level>100) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, isled);//LED1
		else if(cur_batt_level>90) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_2, OUTPUT, PID_GPIO, isled);//LED2
		else if(cur_batt_level>70) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_GPIO, isled);//LED3
		else if(cur_batt_level>50) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_GPIO, isled);//LED4
		else if(cur_batt_level>20) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_5, OUTPUT, PID_GPIO, isled);//LED5
		else GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_GPIO, isled);//LED6
	}
	else
	{
		charging=false;
		battcheck=false;
		chgcnt=0;
		GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_6, OUTPUT, PID_GPIO, battcheck);
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_7, OUTPUT, PID_GPIO, battcheck);//battcheck
		arch_set_extended_sleep();
		
	}
	//else arch_set_extended_sleep();
	

	if(connected==false) 
	{
		if(charging==false)
		{
			leadoff = ADS1292R_RDATA();
			if(leadoff.data!=0x007FFFFF) ldoff++;
			else 
			{
				ldoff=0;
			}
			if(ldoff>2) system_on();
			else timer_used = app_easy_timer(50, sleepcb);
		}
		else timer_used = app_easy_timer(50, sleepcb);
	}
	else if(systemstat==false) 
	{
		timer_used = app_easy_timer(50, sleepcb);
		resetcnt--;
	}
	
	
	
	if(resetcnt)
	{
		wdg_reload(WATCHDOG_DEFAULT_PERIOD);
		wdg_resume();
	}
  else platform_reset(RESET_AFTER_SPOTA_UPDATE);	
}

void afeon()
{
	app_easy_timer_cancel(timer_used);
	AFE44x0_Default_Reg_Init();
	AFE4400_WREG(0x00,0x000001);
	
	wdg_reload(WATCHDOG_DEFAULT_PERIOD);
	 wdg_resume();   
	ADS1292R_sendmsg(AD1292_RDATAC);//start
	
	ADS1292R_sendmsg(AD1292_START);//start
	
	GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_2, INPUT, PID_GPIO, true);//DRDY
	GPIO_EnableIRQ( GPIO_PORT_1, GPIO_PIN_2, GPIO0_IRQn, true, true,0); 
	GPIO_RegisterCallback(GPIO0_IRQn, butcb0);
	
}

void system_on()
{		
		//arch_force_active_mode();
		arch_disable_sleep();
		app_easy_timer_cancel(timer_used);
		//wkupct_disable_irq();
		GPIO_SetInactive(ADS1292_CS_PORT, ADS1292_CS_PIN);//ads RST
		GPIO_SetActive(ADS1292_CS_PORT, ADS1292_CS_PIN);//ads RST
		app_batt_lvl();
		
		
		GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_8, OUTPUT, PID_GPIO, !connected);//LEDBT
	
	
		if(cur_batt_level>100) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false);//LED1
		if(cur_batt_level>90) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_2, OUTPUT, PID_GPIO, false);//LED2
		if(cur_batt_level>70) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_GPIO, false);//LED3
		if(cur_batt_level>50) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_GPIO, false);//LED4
		if(cur_batt_level>20) GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_5, OUTPUT, PID_GPIO, false);//LED5
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_GPIO, false);//LED6
	
	

	
		//ADS1292R_sendmsg(AD1292_START);
		ADS1292R_sendmsg(AD1292_SDATAC);
		ADS1292R_WREG_multi();
	
		if(connected)
		{
			GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_0, OUTPUT, PID_GPIO, true);//
			AFE4400_WREG(0x00,0x00000C);
			timer_used = app_easy_timer(50, afeon);
		}
		else
		{
			ADS1292R_sendmsg(AD1292_RDATAC);//start
			ADS1292R_sendmsg(AD1292_START);//start
			
			GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_2, INPUT, PID_GPIO, true);//DRDY
			GPIO_EnableIRQ( GPIO_PORT_1, GPIO_PIN_2, GPIO0_IRQn, true, true,0); 
			GPIO_RegisterCallback(GPIO0_IRQn, butcb1);
		}
		systemstat=true;
		resetcnt=14;
}
void system_off()
{
	app_easy_timer_cancel(timer_used);
	set_pad_functions();
	wdg_reload(WATCHDOG_DEFAULT_PERIOD);
	 wdg_resume();   
	GPIO_ConfigurePin(GPIO_PORT_1, GPIO_PIN_0, OUTPUT, PID_GPIO, false);//afe disable
	
	GPIO_SetInactive(ADS1292_CS_PORT, ADS1292_CS_PIN);//ads RST
	GPIO_SetActive(ADS1292_CS_PORT, ADS1292_CS_PIN);//ads RST
	//spi_transaction(0x06);//reset
	//spi_transaction(0x04);//standby
	
	if(connected==false)
	{		
		ADS1292R_sendmsg(AD1292_SDATAC);
		ADS1292R_WREG_multi();

		ADS1292R_sendmsg(AD1292_RDATAC);//start	
		ADS1292R_sendmsg(AD1292_START);//start
	}
	
	timer_used = app_easy_timer(50, sleepcb);
	
	systemstat=false;
	arch_set_extended_sleep();
	resetcnt=14;
	//arch_restore_sleep_mode();
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

    // Enable the pads
    SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}
