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
 
 typedef struct SAMPLE//Chunk : [0]=cnt. [1]~[3]=ECG Data, [4]~[6]=PPG Data
{
	uint8_t part[7];
}SAMPLE;

typedef union DATA24
{
	int data;
	uint8_t part[4];
}DATA24;
#endif
