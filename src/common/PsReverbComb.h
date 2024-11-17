#ifndef __PSFILTERREVERBCOMB_H__
#define __PSFILTERREVERBCOMB_H__

#include "PsObject.h"
#include "PsMath.h"
#include "PsFilter.h"

class CPsReverbComb : public CPsObject
{
public:
	bool Init(CPsAudioSystem *pSystem);
	void Shutdown();
	void Process(INT *pBuf, int nSampleFrames);
	void SetLevel(F16 level){
		m_fLevel = (F16)(level / 2 + F_ONE / 4);
	}

	CPsReverbComb();
	virtual ~CPsReverbComb();

protected:
	CPsAudioSystem *m_pSystem;
	F16 m_fLevel;
	int m_nReverbPos, m_nReverbBufferSize;
	int *m_pReverb;
	int LPfilterL, LPfilterR, LPfilterLz, LPfilterRz;
	int m_reverbPtr;
};

#endif
