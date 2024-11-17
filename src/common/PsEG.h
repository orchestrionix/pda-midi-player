#ifndef __PSEG_H__
#define __PSEG_H__

#include "PsObject.h"
#include "PsPatch.h"
#include "PsMath.h"

class CPsEG : public CPsObject  
{
public:
	enum {
		OFF,
		ATTACK,
		DECAY,
		SUSTAIN,
		RELEASE
	};

	void Reset();
	void SetParameters(CPsMath *pMath, CPsAudioConfig *pConfig, CPsPatch::PSENVELOPE *pEG);
	F32 Update();
	void TriggerOn(int key, F32 fVelocity);
	void TriggerOff(int key, F32 fVelocity);
	CPsEG();
	virtual ~CPsEG();

protected:
	void CalcReleaseDelta();
	void CalcDecayDelta();
	short m_state;
	short m_key;
	F32 m_delta;
	F32 m_value;
	F32 m_fSustain;
	CPsMath *m_pMath;
	CPsAudioConfig *m_pConfig;
	CPsPatch::PSENVELOPE *m_pEg;
};

#endif
