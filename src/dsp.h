/**
 ****************************************************************************************
 *
 * @file dsp.h
 *
 * @brief digital signal processing library
 *
 ****************************************************************************************
 */
 
#define NCoef 4 
#define NCoefLPF 20
#define PI2 6.28318530717959
#define FILTER_CAP          0.001
#define NCoef_Stop 4

float iir_HPF_PPG(float NewSample, float OldSample, float OldResult, float cutoff, float SPR);

float iir_LPF_10Hz_Sample_500Hz(float NewSample);

float iir_LPF_20Hz_Sample_500Hz(float NewSample);

float iir_Notch_10Hz_Sample_500Hz(float NewSample);
	
float iir_Notch_20Hz_Sample_500Hz(float NewSample);

float iir_Notch_40Hz_Sample_500Hz(float NewSample);

float iir_Notch_60Hz_Sample_500Hz(float NewSample);

float iir_Notch_60Hz_Sample_500Hz_2nd(float NewSample);

float iir_Notch_70Hz_Sample_500Hz(float NewSample);

float iir_Notch_80Hz_Sample_500Hz(float NewSample);

float Moving_Average(float input);
