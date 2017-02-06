#include "stdafx.h"
#include "ecgtohr.h"

const int SAMPLING = 500;

const int RMS_MAX_RANGE = SAMPLING;

const float SAMPLINGTIME = 1.0 / SAMPLING;

int m_EcgInitFlag = 0;

float m_ECG = 0.0;
float m_ECG_pre = 0.0;
float m_ECG_diff = 0.0;
float m_ECG_LPF = 0.0;
float m_ECG_LPF_pre = 0.0;
float m_ECG_HPF_pre = 0.0;
float m_ECG_HPF = 0.0;
float m_ECG_LPF_diff = 0.0;
float m_ECG_Peak_Index = 0.0;
int m_TimeIndexECG = 0;
int m_RR_IntervalCnt = 0;

float m_ECG_Shift = 0.0;
float m_ECG_LPF_Shift = 0.0;
float m_ECG_Diff_Shift = 0.0;
float m_ECG_Diff_Shift_pre = 0.0;
float m_ECG_Lpf_Diff_Shift = 0.0;

float rmsMax = 0;
float rmsMax_next = 0;
int rmsMaxUsedCount = 0;

const int SHIFT_MAX_DATA = 10;
const int shiftData = SHIFT_MAX_DATA / 2;
int shiftCnt = 0;
float ECGShiftBuffer[SHIFT_MAX_DATA] = { 0.0 };
float ECGLPFShiftBuffer[SHIFT_MAX_DATA] = { 0.0 };
float ECGDiffShiftBuffer[SHIFT_MAX_DATA] = { 0.0 };
float ECGLpfDiffShiftBuffer[SHIFT_MAX_DATA] = { 0.0 };

int resultRRI = 0;
float resultAmplitude = 0;
bool IsPeak=false;


const float m_ECG_HPF_set3Hz = (0.053 / (0.053 + SAMPLINGTIME));
const float m_ECG_LPF_set30Hz = (SAMPLINGTIME / (SAMPLINGTIME + 0.0053));

int getHR(int ecg_raw) {
	int rri = getRRI(ecg_raw);

	if (rri != 0) {
		int hr = 60 * 1000 / rri;
		return hr;
	}
	
	return 0;
}

bool getPeak(float ecg_raw) {
	IsPeak = false;

	if (m_EcgInitFlag == 0) {
		m_EcgInitFlag = 1;
		m_ECG_pre = m_ECG = ecg_raw;
		m_ECG_LPF_pre = m_ECG_LPF = ecg_raw;
	}

	// Buffering for time shift
	m_ECG_pre = m_ECG;
	m_ECG = ecg_raw;
	m_ECG_diff = m_ECG - m_ECG_pre;
	
	//printf("ecg:%f,", m_ECG);

	
	// ECG High Pass Filter
	m_ECG_HPF_pre = m_ECG_HPF;
	m_ECG_HPF = (m_ECG_HPF_set3Hz * m_ECG_HPF_pre)
	+ m_ECG_HPF_set3Hz * (m_ECG - m_ECG_pre);	// HPF fc = 3 Hz
	
		// ECG Low Pass Filter
	m_ECG_LPF_pre = m_ECG_LPF;
	m_ECG_LPF = (1 - m_ECG_LPF_set30Hz) * m_ECG_LPF_pre 
	+ m_ECG_LPF_set30Hz * m_ECG_HPF_pre;	// LPF fc = 30 Hz
	
	m_ECG_LPF_diff = m_ECG_LPF_pre - m_ECG_LPF;
	
	
	ECGShiftBuffer[shiftCnt] = m_ECG;
	ECGLPFShiftBuffer[shiftCnt] = m_ECG_LPF;
	ECGDiffShiftBuffer[shiftCnt] = m_ECG_diff;
	ECGLpfDiffShiftBuffer[shiftCnt] = m_ECG_LPF_diff;
	
	
	


	int shiftLocation = shiftCnt - shiftData;
	//int shiftLocation = shiftCnt - shiftData;
	if (shiftLocation < 0) shiftLocation += SHIFT_MAX_DATA;

	shiftCnt++;
	if (shiftCnt == 10)shiftCnt = 0;

	m_ECG_Shift = ECGShiftBuffer[shiftLocation];
	m_ECG_LPF_Shift = ECGLPFShiftBuffer[shiftLocation];
	m_ECG_Diff_Shift = ECGDiffShiftBuffer[shiftLocation];
	m_ECG_Lpf_Diff_Shift = ECGLpfDiffShiftBuffer[shiftLocation];
	
	
	
	

	m_ECG_Peak_Index = getRms(ECGLpfDiffShiftBuffer);
	rmsMax = getMax(m_ECG_Peak_Index);
	
	if (m_ECG_Peak_Index > rmsMax * 0.65) {

		if ((m_ECG_Diff_Shift_pre > 0)
			&& (m_ECG_Diff_Shift <= 0) && (m_ECG_LPF_Shift > 0))
		{

			IsPeak=true;
		}
	}

	m_ECG_Diff_Shift_pre = m_ECG_Diff_Shift;

	resultAmplitude = m_ECG_LPF_Shift;

	m_TimeIndexECG++;

	return IsPeak;
}


int getRRI(float ecg_raw) 
{
	resultRRI = 0;

	if (m_EcgInitFlag == 0) {
		m_EcgInitFlag = 1;
		m_ECG_pre = m_ECG = ecg_raw;
		m_ECG_LPF_pre = m_ECG_LPF = ecg_raw;
	}

	// Buffering for time shift
	m_ECG_pre = m_ECG;
	m_ECG = ecg_raw;
	m_ECG_diff = m_ECG - m_ECG_pre;
	
	//printf("ecg:%f,", m_ECG);

	
	// ECG High Pass Filter
	m_ECG_HPF_pre = m_ECG_HPF;
	m_ECG_HPF = (m_ECG_HPF_set3Hz * m_ECG_HPF_pre)
	+ m_ECG_HPF_set3Hz * (m_ECG - m_ECG_pre);	// HPF fc = 9Hz
	
		// ECG Low Pass Filter
	m_ECG_LPF_pre = m_ECG_LPF;
	m_ECG_LPF = (1 - m_ECG_LPF_set30Hz) * m_ECG_LPF_pre 
	+ m_ECG_LPF_set30Hz * m_ECG_HPF_pre;	// LPF fc = 30 Hz
	
	m_ECG_LPF_diff = m_ECG_LPF - m_ECG_LPF_pre; //원래반대였음
	
	
	for(shiftCnt=0; shiftCnt<SHIFT_MAX_DATA-1; shiftCnt++)
  {
    ECGShiftBuffer[shiftCnt] = ECGShiftBuffer[shiftCnt+1];
		ECGLPFShiftBuffer[shiftCnt] = ECGLPFShiftBuffer[shiftCnt+1];
		ECGDiffShiftBuffer[shiftCnt] = ECGDiffShiftBuffer[shiftCnt+1];
		ECGLpfDiffShiftBuffer[shiftCnt] = ECGLpfDiffShiftBuffer[shiftCnt+1];
  }

/*
	shiftCnt = SHIFT_MAX_DATA-1;
	memmove(&ECGShiftBuffer[0], &ECGShiftBuffer[1], shiftCnt);
	memmove(&ECGLPFShiftBuffer[0], &ECGLPFShiftBuffer[1],shiftCnt);
	memmove(&ECGDiffShiftBuffer[0], &ECGDiffShiftBuffer[1],shiftCnt);
	memmove(&ECGLpfDiffShiftBuffer[0], &ECGLpfDiffShiftBuffer[1], shiftCnt);
	*/
	
	
	ECGShiftBuffer[shiftCnt] = m_ECG;
	ECGLPFShiftBuffer[shiftCnt] = m_ECG_LPF;
	ECGDiffShiftBuffer[shiftCnt] = m_ECG_diff;
	ECGLpfDiffShiftBuffer[shiftCnt] = m_ECG_LPF_diff;
	//int shiftLocation = shiftCnt - shiftData;
	//int shiftLocation = shiftCnt - shiftData;
//	if (shiftLocation < 0) shiftLocation += SHIFT_MAX_DATA;

	//shiftCnt++;
//	if(shiftCnt == 10) shiftCnt = 0;

	m_ECG_Shift = ECGShiftBuffer[0];
	m_ECG_LPF_Shift = ECGLPFShiftBuffer[0];
	m_ECG_Diff_Shift = ECGDiffShiftBuffer[0];
	m_ECG_Lpf_Diff_Shift = ECGLpfDiffShiftBuffer[0];

	m_ECG_Peak_Index = getRms(ECGLpfDiffShiftBuffer);
	rmsMax = getMax(m_ECG_Peak_Index);
	
	if (m_ECG_Peak_Index > rmsMax * 0.65) 
	{

		if ((m_ECG_Diff_Shift_pre > 0) && (m_ECG_Diff_Shift <= 0) && (m_ECG_LPF_Shift >=0))
		{

			resultRRI = m_RR_IntervalCnt * 1000 / SAMPLING;
			m_RR_IntervalCnt = 0;
		}
	}

	m_ECG_Diff_Shift_pre = m_ECG_Lpf_Diff_Shift;

	resultAmplitude = m_ECG_LPF_Shift;

	m_TimeIndexECG++;
	m_RR_IntervalCnt++;

	return resultRRI;
}

float getRms(float value[]) 
{
	float rms = 0;

	for (int i = 0; i < SHIFT_MAX_DATA; i++) rms +=( value[i] * value[i]);
	rms /= SHIFT_MAX_DATA;
	rms = sqrt(rms);

	return rms;
}

float getMax(float value) {

	if (rmsMax < value)
	{
		rmsMax = value;
		rmsMaxUsedCount = 0;
		rmsMax_next = 0;
	}
	else 
	{
		if (rmsMax_next < value)	rmsMax_next = value;
		rmsMaxUsedCount++;
	}

	if (rmsMaxUsedCount >= RMS_MAX_RANGE) 
	{
		rmsMax = rmsMax_next;
		rmsMaxUsedCount = 0;
		rmsMax_next = 0;
	}
	//printf("rmsMax:%d,%f\n", m_TimeIndexECG,rmsMax);
	return rmsMax;
}

float getAmplitude() 
{
	return resultAmplitude;
}
