#ifndef __PSCHORUS_H__
#define __PSCHORUS_H__

#include "PsObject.h"
#include "PsMath.h"

class CPsChorus : public CPsObject
{
public:
	enum {
		VOICE_DELAY = 20, //in ms
		VOICE_NUMBER = 4
	};
	bool Init(CPsAudioConfig *pConfig);
	void Shutdown();
	void Process(int *pBuf, int nSampleFrames);

	CPsChorus();
	virtual ~CPsChorus();

protected:
	int m_nGapFrames;
	int m_nDelayFrames;
	int m_nBufferFrames;
	int *m_pBuf;
};

#endif
