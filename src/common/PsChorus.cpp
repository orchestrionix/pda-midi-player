#include "PsAudioSystem.h"
#include "PsChorus.h"
#include "Win32/PsSys.h"

#define READINDEXSHIFT	8L
#define READINDEXMASK	((1L << READINDEXSHIFT) - 1)

#define PHISHIFT	16
#define PHIMASK		((1L << PHISHIFT) - 1)


#define	kMinDelayFrames 	500
#define	kMaxDepthRange 		1000 /*200*/

#define kModulationTableLength		200

#define	kReadIndexAdjust (kChorusBufferFrameSize << READINDEXSHIFT)

#define mFeedbackGain 8

CPsChorus::CPsChorus()
{
}

CPsChorus::~CPsChorus()
{
	Shutdown();
}

bool CPsChorus::Init(CPsAudioConfig *pConfig)
{
	CPsSys::memset(mChorusBufferL, 0, sizeof(mChorusBufferL));
	CPsSys::memset(mChorusBufferR, 0, sizeof(mChorusBufferR));

    mSampleRate = pConfig->GetMixFrequency();
	SetupDelay();
	

    mRate = 64;     /* 0.5  *  (1L << 7) */
    mPhi = 0;
	
//    mFeedbackGain = 10 /*10*/;		// value from 0-127

	return true;
}

void CPsChorus::Shutdown()
{
}

void CPsChorus::SetupDelay()
{
    mSampleFramesDelay = (mSampleRate * kMinDelayFrames) / 44100;

    mWriteIndex = 0;
	
    mReadIndexL = (kChorusBufferFrameSize - mSampleFramesDelay) << READINDEXSHIFT;
    mReadIndexR = (kChorusBufferFrameSize - mSampleFramesDelay) << READINDEXSHIFT;
}


// this table doesn't have to be extremely accurate -- just used to modulate chorus delay

static const int expTable[] = 	/* kModulationTableLength entries fixed point 16.16 */
{
    16384, 16612, 16844, 17079, 17318, 17559, 17805, 18053, 18305, 18561, 
    18820, 19082, 19349, 19619, 19893, 20171, 20452, 20738, 21027, 21321, 
    21618, 21920, 22226, 22536, 22851, 23170, 23493, 23821, 24154, 24491, 
    24833, 25180, 25531, 25888, 26249, 26615, 26987, 27364, 27746, 28133, 
    28526, 28924, 29328, 29737, 30152, 30573, 31000, 31433, 31871, 32316, 
    32768, 33225, 33689, 34159, 34636, 35119, 35610, 36107, 36611, 37122, 
    37640, 38165, 38698, 39238, 39786, 40342, 40905, 41476, 42055, 42642, 
    43237, 43841, 44453, 45073, 45702, 46340, 46987, 47643, 48308, 48983, 
    49667, 50360, 51063, 51776, 52498, 53231, 53974, 54728, 55492, 56266, 
    57052, 57848, 58656, 59475, 60305, 61147, 62000, 62866, 63743, 64633, 
    65536, 64633, 63743, 62866, 62000, 61147, 60305, 59475, 58656, 57848, 
    57052, 56266, 55492, 54728, 53974, 53231, 52498, 51776, 51063, 50360, 
    49667, 48983, 48308, 47643, 46987, 46340, 45702, 45073, 44453, 43841, 
    43237, 42642, 42055, 41476, 40905, 40342, 39786, 39238, 38698, 38165, 
    37640, 37122, 36611, 36107, 35610, 35119, 34636, 34159, 33689, 33225, 
    32768, 32316, 31871, 31433, 31000, 30573, 30152, 29737, 29328, 28924, 
    28526, 28133, 27746, 27364, 26987, 26615, 26249, 25888, 25531, 25180, 
    24833, 24491, 24154, 23821, 23493, 23170, 22851, 22536, 22226, 21920, 
    21618, 21321, 21027, 20738, 20452, 20171, 19893, 19619, 19349, 19082, 
    18820, 18561, 18305, 18053, 17805, 17559, 17318, 17079, 16844, 16612
};


//++------------------------------------------------------------------------------
//	GetChorusReadIncrement()
//
//++------------------------------------------------------------------------------
int CPsChorus::GetReadIncrement(int readIndex, long writeIndex, long nSampleFrames, int phase)
{
    int	phi = mPhi;
    long	sampleFramesDelay = mSampleFramesDelay;

    long	currentDelayFrame;
    long	desiredDelayFrame;

    int	ratio;

    int		sampleFramesShift = 9;

    int	phiIndex = (phi * kModulationTableLength) + (phase << PHISHIFT);	// in 16.16
    int 	b = expTable[ 	(phiIndex >> PHISHIFT) 		% kModulationTableLength];
    int	c = expTable[( 	(phiIndex >> PHISHIFT) + 1) % kModulationTableLength];
    int 	expValue = (((int) (phiIndex & PHIMASK) * (c - b))>>PHISHIFT) + b;
    int	offsetFrame = (kMaxDepthRange * expValue) >> 16;
	
    // compensate for sampling rate
    offsetFrame = mSampleRate * offsetFrame / 44100;
	
    // don't let delay get below minimum required for chorusing
    if(sampleFramesDelay < kMinDelayFrames)
	{
	    sampleFramesDelay = kMinDelayFrames;
	}
	
    currentDelayFrame = ((long)(kChorusBufferFrameSize + writeIndex - (readIndex >> READINDEXSHIFT) )) % kChorusBufferFrameSize;
		
    desiredDelayFrame = sampleFramesDelay + /*depth * */ (offsetFrame );

    ratio = (currentDelayFrame - desiredDelayFrame + nSampleFrames) << READINDEXSHIFT;

    ratio /= nSampleFrames;
	
    return ratio;
}

void CPsChorus::Process(int *sourceP, int *destP, int nSampleFrames)
{
#define FEEDBACKSHIFT	7

    int	readIndexL = mReadIndexL;
    int	readIndexR = mReadIndexR;
    int readIndexIncrL =  GetReadIncrement(readIndexL, mWriteIndex, nSampleFrames, 0);
    int readIndexIncrR =  GetReadIncrement(readIndexR, mWriteIndex, nSampleFrames, kModulationTableLength/2);

    mPhi += (mRate * nSampleFrames) >> 7;
    mPhi %= 65536;
	

    while(nSampleFrames-- > 0)
	{
	    // calculate sample value
	    int intReadIndex = readIndexL >> READINDEXSHIFT;
	    int b = mChorusBufferL[intReadIndex];
	    int tap = (((int) (readIndexL & READINDEXMASK) * (mChorusBufferL[(intReadIndex + 1) % kChorusBufferFrameSize] - b))>>READINDEXSHIFT) + b;
	    mChorusBufferL[mWriteIndex] = *sourceP++ - ((tap*mFeedbackGain) >> FEEDBACKSHIFT) ;
	    *destP++ += tap;

		//kReadIndexAdjust must be power of 2
		PsAssert(((kReadIndexAdjust - 1) & kReadIndexAdjust) == 0);
	    readIndexL = (readIndexL + readIndexIncrL) % kReadIndexAdjust;
		
		PsAssert(readIndexR >= 0);
	    intReadIndex = readIndexR >> READINDEXSHIFT;
	    b = mChorusBufferR[intReadIndex];
	    tap = (((int) (readIndexR & READINDEXMASK) * (mChorusBufferR[(intReadIndex + 1) % kChorusBufferFrameSize] - b))>>READINDEXSHIFT) + b;
	    mChorusBufferR[mWriteIndex] = *sourceP++ - ((tap*mFeedbackGain) >> FEEDBACKSHIFT) ;
	    *destP++ += tap;
	    readIndexR = (readIndexR + readIndexIncrR) % kReadIndexAdjust;

	    // wrap-around write pointer
	    mWriteIndex = (mWriteIndex + 1) % kChorusBufferFrameSize;
	}

    // remember state
    mReadIndexL = readIndexL;
    mReadIndexR = readIndexR;
}
