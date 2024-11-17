#ifndef __PSCHORUS_H__
#define __PSCHORUS_H__

#include "PsObject.h"
#include "PsMath.h"

class CPsChorus : public CPsObject
{
public:
	enum {
		kChorusBufferFrameSize = 4096L,
	};

	bool Init(CPsAudioConfig *pConfig);
	void Shutdown();
	void SetupDelay();
	int GetReadIncrement(int readIndex, long writeIndex, long nSampleFrames, int phase);
	void Process(int *sourceP, int *destP, int nSampleFrames);

	CPsChorus();
	virtual ~CPsChorus();

protected:
	int				mSampleRate;
	
	int				mChorusBufferL[kChorusBufferFrameSize];
	int				mChorusBufferR[kChorusBufferFrameSize];

	int				mWriteIndex;
	int				mReadIndexL;
	int				mReadIndexR;

	int				mSampleFramesDelay;
	int				mRate;
	int				mPhi;
//	int				mFeedbackGain;	// between 0-127
};

#endif
