#ifndef __PSFILTERREVERBA_H__
#define __PSFILTERREVERBA_H__

#include "PsObject.h"
#include "PsMath.h"
#include "PsFilter.h"

class CPsReverbNoAllPass : public CPsObject
{
public:
	enum {
		NUM_COMB = 4,
		MAX_DELAY_SAMPLES = 2048,
		DELAY_MASK = (MAX_DELAY_SAMPLES - 1)
	};
	void CalcFilter(int freq, F32 rez);
	void SetLength(int len);
	bool Init(CPsAudioSystem *pSystem, int reverbType);
	void Shutdown();
	void Process(NativeSample *pBuf, int nSampleFrames, int nStep);
	void SetLevel(F16 level){
		m_fLevel = (F16)(level * 2 / 4 + F_ONE / 4);
	}

	CPsReverbNoAllPass();
	virtual ~CPsReverbNoAllPass();
	
protected:
	CPsAudioSystem *m_pSystem;
	int m_reverbType;
	int m_delayPos;
	F16 m_delayBuf[NUM_COMB][MAX_DELAY_SAMPLES];
	unsigned short m_nDelay[NUM_COMB];
	F16 m_fGain[NUM_COMB];
	F16 m_fLevel;

	NativeSample m_in[2], m_out[2];
	F16 m_a[3], m_b[2];
};

#endif
