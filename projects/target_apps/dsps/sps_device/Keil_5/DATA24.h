/**
 ****************************************************************************************
 *
 * @file DATA24.h
 *
 * @brief DATA Typdef
 *
 ****************************************************************************************
 */
 #ifndef _DATA24_H_
	#define _DATA24_H_
	
#include <stdint.h>
 
 typedef struct SAMPLE
{
	uint8_t part[7];
}SAMPLE;

typedef union DATA24
{
	int data;
	uint8_t part[4];
}DATA24;
#endif
