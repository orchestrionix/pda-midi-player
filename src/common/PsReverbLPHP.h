#ifndef __PSFILTERREVERBLPHP_H__
#define __PSFILTERREVERBLPHP_H__

#include "PsObject.h"
#include "PsMath.h"
#include "PsFilter.h"

class CPsReverbLPHP : public CPsObject
{
public:
	enum {
		REV_BUF0 = 344,
		REV_BUF1 = 684,
		REV_BUF2 = 2868,
		REV_BUF3 = 1368,
	};

	void SetSampleRate(int rate);
	bool Init(CPsAudioSystem *pSystem);
	void Shutdown();
	void Process(TSAMPLE *pBuf, int nSampleFrames);

	CPsReverbLPHP();
	virtual ~CPsReverbLPHP();
	
protected:
	int  spt0, rpt0;
	int  spt1, rpt1;
	int  spt2, rpt2;
	int  spt3, rpt3;
	TSAMPLE  buf0_L[REV_BUF0], buf0_R[REV_BUF0];
	TSAMPLE  buf1_L[REV_BUF1], buf1_R[REV_BUF1];
	TSAMPLE  buf2_L[REV_BUF2], buf2_R[REV_BUF2];
	TSAMPLE  buf3_L[REV_BUF3], buf3_R[REV_BUF3];

	F32 ta, tb;
	F32 HPFL, HPFR;
	F32 LPFL, LPFR;
	F32 EPFL, EPFR;
};

#endif
