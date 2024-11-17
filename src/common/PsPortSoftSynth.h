#ifndef __PSPORTSOFTSYNTH_H__
#define __PSPORTSOFTSYNTH_H__

#include "PsObject.h"

class CPsSynthesizer;
class CPsPortSoftSynth : public CPsOutputPort
{
public:
	CPsPortSoftSynth();
	void AttachSynthesizer(CPsSynthesizer *pSynth);
	void DetachSynthesizer();
	virtual const char* GetName();
	virtual void Reset();
	virtual void AllNoteOff();
	virtual void ReduceVoice();
	virtual int GetActiveCount();
	virtual void GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList);
	virtual void Send(unsigned int framePosition, void *pEvent);

	void SetEnable(bool isEnable){
		m_isEnabled = isEnable;
	}

protected:
	CPsSynthesizer *m_pSynth;
	bool m_isEnabled;
};



#endif
