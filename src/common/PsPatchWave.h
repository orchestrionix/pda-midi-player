#ifndef __PSPATCHWAVE_H__
#define __PSPATCHWAVE_H__

#include "PsObject.h"

class CPsPatchWave : public CPsObject  
{
public:
	bool ExpandTo16Bit();
	void UpdateWaveForInterpolation(int loopStart, int loopLength);
	void Free();
	bool Load(CPsMath *pMath, CPsReader *pReader, int link);
	CPsPatchWave();
	virtual ~CPsPatchWave();

	int GetChannelNumber(){
		return m_nChannel;
	}

	int GetBitPerSample(){
		return m_bitPerSample;
	}

	unsigned char* GetData(){
		return m_pData;
	}

	unsigned int GetSampleCount(){
		return m_nSamples;
	}

	unsigned int GetSampleRate(){
		return m_nSampleRate;
	}

	F32 GetSamplePitch(){
		return m_fPitch;
	}

protected:
	unsigned short m_link;
	unsigned char m_bitPerSample;
	unsigned char m_nChannel;
	unsigned int m_nSamples;
	unsigned int m_nSampleRate;
	unsigned char *m_pData;
	F32 m_fPitch;
};

#endif
