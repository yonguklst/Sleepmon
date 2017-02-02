/**
 ****************************************************************************************
 *
 * @file dsp.c
 *
 * @brief digital signal processing library
 *
 ****************************************************************************************
 */


#include "dsp.h"



float iir_HPF_PPG(float NewSample, float OldSample, float OldResult, float cutoff, float SPR)
{
    float tau, ts, omega, filter_R, HPF;

    ts = 1.0 / SPR;
    filter_R = 1.0 / (cutoff * PI2 * FILTER_CAP); // 1 / (2*pi*f*c)
    tau = FILTER_CAP * filter_R;
    omega = tau / (tau + ts);

    HPF = omega * OldResult + omega * (NewSample-OldSample);

    return HPF;
}

float iir_LPF_10Hz_Sample_500Hz(float NewSample)
{
    float ACoef[NCoefLPF+1] = {
        0.00362453228686862190,
        0.00724906457373724390,
        0.00362453228686862190
    };

    float BCoef[NCoefLPF+1] = {
        1.00000000000000000000,
        -1.82269492519630800000,
        0.83718165125602251000
    };

    static float y[NCoefLPF+1]; //output samples
    static float x[NCoefLPF+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

float iir_LPF_20Hz_Sample_500Hz(float NewSample)
{
    float ACoef[NCoefLPF+1] = {
        0.01336978300344553300,
        0.02673956600689106700,
        0.01336978300344553300
    };

    float BCoef[NCoefLPF+1] = {
        1.00000000000000000000,
        -1.64745998107697660000,
        0.70089678118840248000
    };

    static float y[NCoefLPF+1]; //output samples
    static float x[NCoefLPF+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

float iir_Notch_10Hz_Sample_500Hz(float NewSample)
{
    float ACoef[NCoef+1] = {
        0.96539268279076262000,
        -3.83233138387353240000,
        5.73409879897254800000,
        -3.83233138387353240000,
        0.96539268279076262000
    };

    float BCoef[NCoef+1] = {
        1.00000000000000000000,
        -3.88454465515939380000,
        5.68717500860020750000,
        -3.71901384600540070000,
        0.91660312435074764000
    };

    static float y[NCoef+1]; //output samples
    static float x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

float iir_Notch_20Hz_Sample_500Hz(float NewSample) 
{
    float ACoef[NCoef+1] = {
        0.96092034863431064000,
        -3.72410118655697710000,
        5.53008191815285690000,
        -3.72410118655697710000,
        0.96092034863431064000
    };

    float BCoef[NCoef+1] = {
        1.00000000000000000000,
        -3.79240881789719090000,
        5.51031761662175960000,
        -3.63080416250698910000,
        0.91660312434042190000
    };

    static float y[NCoef+1]; //output samples
    static float x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

float iir_Notch_40Hz_Sample_500Hz(float NewSample)
{
    float ACoef[NCoef+1] = {
        0.95338534760684035000,
        -3.34288751475714330000,
        4.83709078151721080000,
        -3.34288751475714330000,
        0.95338534760684035000
    };

    float BCoef[NCoef+1] = {
        1.00000000000000000000,
        -3.43110773956860670000,
        4.85775280508986910000,
        -3.28489908681373200000,
        0.91660312432170887000
    };

    static float y[NCoef+1]; //output samples
    static float x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

float iir_Notch_60Hz_Sample_500Hz(float NewSample)
{
    float ACoef[NCoef_Stop+1] = 
    {
        0.91024132565011984000,
        -2.65603666584433600000,
        3.75802673206117270000,
        -2.65603666584433600000,
        0.91024132565011984000
    };

    float BCoef[NCoef_Stop+1] = 
    {
        1.00000000000000000000,
        -2.84023127667379870000,
        3.91155348318045390000,
        -2.69266918930618140000,
        0.89885899410165904000
    };

    static float y[NCoef_Stop+1]; //output samples
    static float x[NCoef_Stop+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef_Stop; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef_Stop; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}
float iir_Notch_60Hz_Sample_500Hz_2nd(float NewSample)
{
    float ACoef[NCoef_Stop+1] = 
    {
        0.91024132565011984000,
        -2.65603666584433600000,
        3.75802673206117270000,
        -2.65603666584433600000,
        0.91024132565011984000
    };

    float BCoef[NCoef_Stop+1] = 
    {
        1.00000000000000000000,
        -2.84023127667379870000,
        3.91155348318045390000,
        -2.69266918930618140000,
        0.89885899410165904000
    };

    static float y[NCoef_Stop+1]; //output samples
    static float x[NCoef_Stop+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef_Stop; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef_Stop; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}
float iir_Notch_70Hz_Sample_500Hz(float NewSample) 
	{
    float ACoef[NCoef+1] = {
        0.97751217400974821000,
        -2.49314620222980080000,
        3.54471758943351390000,
        -2.49314620222980080000,
        0.97751217400974821000
    };

    float BCoef[NCoef+1] = {
        1.00000000000000000000,
        -2.49578193843998040000,
        3.47164988271752550000,
        -2.38943001290087760000,
        0.91660312429177226000
    };

    static float y[NCoef+1]; //output samples
    static float x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

float iir_Notch_80Hz_Sample_500Hz(float NewSample) {
    float ACoef[NCoef+1] = {
        0.96453602853065767000,
        -2.06795007603302140000,
        3.03748516721737970000,
        -2.06795007603302140000,
        0.96453602853065767000
    };

    float BCoef[NCoef+1] = {
        1.00000000000000000000,
        -2.09798636157287130000,
        3.01473557520101560000,
        -2.00858556663177710000,
        0.91660312428209345000
    };

    static float y[NCoef+1]; //output samples
    static float x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}

#define FILTERSIZE 15

float Mdata[FILTERSIZE] = {0,};

float Moving_Average(float input)
{
  float sumM, averageM;
  int i_M;
  for(i_M=0; i_M<FILTERSIZE; i_M++)
  {
    sumM += Mdata[i_M];
  } 
  sumM += input;

  averageM = sumM/(FILTERSIZE+1);

  for(i_M=0; i_M<FILTERSIZE-1; i_M++)
  {
    Mdata[i_M] = Mdata[i_M+1];
  }
  Mdata[i_M] = input;

  return averageM;      
}

