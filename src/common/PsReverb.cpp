#include "PsAudioSystem.h"
#include "PsReverb.h"
#include "Win32/PsSys.h"

#define INPUTSHIFT	3 /*9*/

// if COEFF_SHIFT is changed, then a number of constant parameters must be recalculated!
#define COEFF_SHIFT			16
#define	COEFF_MULTIPLY		(1L << 	COEFF_SHIFT)

// if COMB_FILTER_SHIFT is changed, must recreate inverseFeedbackTable
#define COMB_FILTER_SHIFT		16
#define	COMB_FILTER_MULTIPLY	(1L << 	COMB_FILTER_SHIFT)
#define COMB_FILTER_ROUND		(COMB_FILTER_MULTIPLY >> 1)

// based on 0.7 feedback
#define kDiffusionFeedback	 	45875L
#define kDiffusionDelayGain		33423L

// based on 0.4 feedback
#define kStereoizerFeedbackL	26214L
#define kStereoizerDelayGainL	55050L
#define kStereoizerFeedbackR	(-26214L)
#define kStereoizerDelayGainR	55050L

CPsFilterReverb::CPsFilterReverb()
{
	m_pSystem = NULL;
	mIsInitialized = FALSE;
	CPsSys::memset(mReverbBuffer, 0, sizeof(mReverbBuffer));
	CPsSys::memset(mDiffusionBuffer, 0, sizeof(mDiffusionBuffer));
	mEarlyReflectionBuffer = NULL;
	mStereoizerBuffer = NULL;
}

CPsFilterReverb::~CPsFilterReverb()
{
	ShutdownNewReverb();
}

bool CPsFilterReverb::InitNewReverb(CPsAudioSystem *pSystem, int reverbType)
{
	int i;
	
	m_pSystem = pSystem;

	// memory allocation
	mIsInitialized = FALSE;
	m_reverbType = reverbType;

	// allocate the comb filter delay line memory
	for(i = 0; i < kNumberOfCombFilters; i++)
	{
		mReverbBuffer[i] = new int[kCombBufferFrameSize];
		if (mReverbBuffer[i] == NULL)
		{
			ShutdownNewReverb();
			return FALSE;
		}
	}

	mEarlyReflectionBuffer = new int[kEarlyReflectionBufferFrameSize];
	if (mEarlyReflectionBuffer == NULL)
	{
		ShutdownNewReverb();
		return FALSE;
	}

	// allocate the diffusion delay line memory
	for(i = 0; i < kNumberOfDiffusionStages; i++)
	{
		mDiffusionBuffer[i] = new int[kDiffusionBufferFrameSize];
		if (mDiffusionBuffer[i] == NULL)
		{
			ShutdownNewReverb();
			return FALSE;
		}
	}

	mStereoizerBuffer = new int[kStereoizerBufferFrameSize];
	if (mStereoizerBuffer == NULL)
	{
		ShutdownNewReverb();
		return FALSE;
	}

	mRoomChoice = 1;
	mRoomSize = 70 /*51*/;				// 0.4 ~ 51/128;
	mMaxRegen = 120 /*77*/;				// 0.6 ~ 77/128
	mDiffusedBalance = 0;				// 0-127	0 - 2x

	mReverbType = -1; // force parameter recalc

	CheckReverbType();

	GenerateDelayTimes();
	GenerateFeedbackValues();
	SetupDiffusion();
	SetupStereoizer();
	SetupEarlyReflections();

	mIsInitialized = TRUE;
	mSampleRate = m_pSystem->GetConfig()->GetMixFrequency();
	return TRUE;
}

void CPsFilterReverb::ShutdownNewReverb()
{
	int i;
	
	mIsInitialized = FALSE;		// do this before deallocating stuff!!

	// deallocate the comb filter buffers
	for(i = 0; i < kNumberOfCombFilters; i++)
	{
		delete mReverbBuffer[i];
		mReverbBuffer[i] = NULL;
	}

	// deallocate early reflection buffer
	delete mEarlyReflectionBuffer;
	mEarlyReflectionBuffer = NULL;

	// deallocate the diffusion buffers
	for(i = 0; i < kNumberOfDiffusionStages; i++)
	{
		delete mDiffusionBuffer[i];
		mDiffusionBuffer[i] = NULL;
	}

	// deallocate stereoizer buffers
	delete mStereoizerBuffer;
	mStereoizerBuffer = NULL;
}

unsigned int CPsFilterReverb::GetSamplingRate()
{
	return m_pSystem->GetConfig()->GetMixFrequency() << 16;
}

bool CPsFilterReverb::CheckReverbType()
{
	bool changed = false;
	int i;
	
	if(mReverbType != m_reverbType)
	{
		mIsInitialized = FALSE;	// set to false to stop playback
		changed = TRUE;
		mReverbType = m_reverbType;
		
		mDiffusedBalance = 64;	// default to reasonable diffused sound
		
		switch(m_reverbType)
		{
		case REVERB_TYPE_1:		// no reverb
			break;
		case REVERB_TYPE_8:		// Early reflections
			mDiffusedBalance = 0;	// just early reflections
			break;
		case REVERB_TYPE_9:		// Basement
			mMaxRegen = 120;
			mRoomSize = 20;
			break;
		case REVERB_TYPE_10:	// Banquet hall
			mMaxRegen = 77;
			mRoomSize = 51;
			break;
		case REVERB_TYPE_11:	// Catacombs
			mMaxRegen = 120;
			mRoomSize = 80;
			break;
		}
		
		// clear out shared buffers, when switching reverb modes...
		for(i = 0; i < kNumberOfCombFilters; i++)
		{
			CPsSys::memset(mReverbBuffer[i], 0, sizeof(int)*kCombBufferFrameSize);
		}
		for(i = 0; i < kNumberOfDiffusionStages; i++)
		{
			CPsSys::memset(mDiffusionBuffer[i], 0, sizeof(int)*kDiffusionBufferFrameSize);
		}
		CPsSys::memset(mEarlyReflectionBuffer, 0, sizeof(int)*kEarlyReflectionBufferFrameSize);
		CPsSys::memset(mStereoizerBuffer, 0, sizeof(int)*kStereoizerBufferFrameSize);
		mIsInitialized = TRUE;	// enable runtime again
	}
	return changed;
}

/*
 	The below table represents 16 discrete value from 0.0 <= maxRegen <= 15/16
 	(maxRegen = i/16, where i is the index in the list )
 	
 	The value is COEFF_MULTIPLY * log10(0.7 + 0.3 * (float(i) / 16.0) ) / (-3.0) 	
 */
 
static const int regenList[] =
{
	3383,
	3133,
	2888,
	2650,
	2418,
	2191,
	1969,
	1753,
	1541,
	1334,
	1132,
	933,
	739,
	549,
	362,
	179
};

void CPsFilterReverb::ScaleDelayTimes()
{
	int i;
	int		T60;
	int		desiredMinFrames;
	int		kMinDelayFrames;
	int		currentMinFrames;
	int		delayScale;
	int		delayFrames;
	int		log10maxG;
	unsigned int	srate = GetSamplingRate() >> 16;
	int		maxDelayFrame;

	// mRoomSize 0-127
	T60 = mRoomSize * 4;	// range of 4 seconds
	
	//maxG = 0.7 + 0.3 * params->mMaxRegen;
	//if(maxG > 0.97) maxG = 0.97;
	
	// mMaxRegen should be between 0-127, shift over three to leave 4 bit index into table
	log10maxG = regenList[ (mMaxRegen >> 3 ) & 0xf ];		
	desiredMinFrames = srate * ((log10maxG * T60) >> 7);    /*log10(maxG) * T60 / -3.0 */
	
	// ok, at this point, desiredMinFrames if FIXED 16.16
	kMinDelayFrames = 485L * GetSR_44100Ratio();  /*485L << 16*/	/* 11ms */
	if(desiredMinFrames < kMinDelayFrames) desiredMinFrames = kMinDelayFrames;
	
	
	currentMinFrames = mUnscaledDelayFrames[0];
	
	delayScale = desiredMinFrames / currentMinFrames;

	// clip scale factor to max delay time
	maxDelayFrame = mUnscaledDelayFrames[kNumberOfCombFilters-1];
	
	if( ((delayScale * maxDelayFrame) >> 16) >= kCombBufferFrameSize )
	{
		delayScale = (kCombBufferFrameSize << 16) / maxDelayFrame;
		delayScale = (delayScale * 99) / 100;	// times 0.99
	}
	
	
	for(i = 0; i < kNumberOfCombFilters; i++)
	{
		// do the scaling
		mDelayFrames[i] = (delayScale * mUnscaledDelayFrames[i]) >> 16;
		delayFrames = mDelayFrames[i];
		
		// setup read and write indices
		PsAssert(kCombBufferFrameSize > delayFrames);
		mReadIndex[i] = kCombBufferFrameSize - delayFrames;
		mWriteIndex[i] = 0;

		PsAssert(delayFrames < kCombBufferFrameSize);
		PsAssert(delayFrames > 1);
	}	
}

/* the following table corresponds to feedback values (for the comb filter) from 0.5 -> 1.0
   in increments of 0.05. The actual value in the table is a ratio of delay frame size
   divided by the room size (0-127) multiplied by (1L << 16)
 */
 
static const int inverseFeedbackTable[] =	/* 100 entries */
{
	9062688, 8932591, 8803775, 8676216, 8549889, 8424772, 8300840, 8178072, 8056446, 7935941, 
	7816537, 7698213, 7580950, 7464730, 7349533, 7235343, 7122142, 7009912, 6898637, 6788302, 
	6678889, 6570385, 6462774, 6356041, 6250172, 6145154, 6040973, 5937615, 5835068, 5733319, 
	5632355, 5532165, 5432738, 5334060, 5236122, 5138912, 5042419, 4946633, 4851544, 4757142, 
	4663416, 4570357, 4477956, 4386204, 4295090, 4204608, 4114747, 4025499, 3936857, 3848812, 
	3761355, 3674480, 3588178, 3502442, 3417264, 3332638, 3248556, 3165012, 3081997, 2999507, 
	2917534, 2836071, 2755113, 2674653, 2594685, 2515204, 2436202, 2357675, 2279617, 2202022, 
	2124885, 2048200, 1971963, 1896167, 1820809, 1745882, 1671382, 1597304, 1523644, 1450396, 
	1377556, 1305120, 1233083, 1161441, 1090189, 1019323, 948839, 878733, 809002, 739640, 
	670644, 602010, 533735, 465814, 398244, 331022, 264144, 197606, 131405, 65537
};

void CPsFilterReverb::GenerateFeedbackValues()
{
	int i;

	if(mRoomSize == 0)
		mRoomSize = 1;	// avoid divide by zero

	for(i = 0; i < kNumberOfCombFilters; i++)
	{
		long frames = (mDelayFrames[i] * Get44100_SRRatio() ) >> 16;
		int fakeRatio = (frames << COMB_FILTER_SHIFT) / mRoomSize;
		int k;
		int g, kOneHalf;
		
		for(k = 0; k < 100; k++)
		{
			if(inverseFeedbackTable[k] <= fakeRatio)
				break;
		}
		
		if(k == 100)
			k = 99;
		
		kOneHalf = (1L << (COMB_FILTER_SHIFT-1) );
		
		g = kOneHalf + ((kOneHalf * k) / 100);
		
		// g is always negative to avoid "limit cycles" in fixed-point comb filter regen
		g = -g;
		
		mFeedbackList[i] = g;
	}

#if 0
	// code for generating the inverse table...
	FILE *fp = fopen("regenParams.out", "w");

	for(i = 0; i < 100; i++)
	{
		static count = 0;
		
		//int fakeRatio = (frames << 16) / params->mRoomSize;
		double g = 0.5 + (0.01 * i) * 0.5;
		
		const double k = -3 * 32 / 44100.0;
		
		double fakeRatio = log10(g) / k;
		int intFakeRatio = fakeRatio * (1L << COMB_FILTER_SHIFT);
		
		fprintf(fp, "%d, ", intFakeRatio);

		if(++count == 10)
		{
			count = 0;
			fprintf(fp, "\n");
		}
		
	}

	fclose(fp);
	ExitToShell();
#endif
}

static const long delay6tapList[6][6] =
{
	{
		1259,
		1459,
		1621,
		1831,
		1993,
		2269
	},
	{
		1279,
		1433,
		1669,
		1867,
		2113,
		2311
	},
	{
		1381,
		1543,
		1669,
		1907,
		2017,
		2203
	},
	{
		1249,
		1451,
		1747,
		1901,
		2017,
		2161
	},
	{
		1283,
		1471,
		1777,
		1907,
		1993,
		2203
	},
	{
		1381,
		1531,
		1753,
		1789,
		1979,
		2309
	}
};

void CPsFilterReverb::GenerateDelayTimes()
{
	int i;
	const long	*delayFrameList;
	int index = mRoomChoice;
	
	if(index > 5) index = 5;
	
	delayFrameList = delay6tapList[index];

	for(i = 0; i < kNumberOfCombFilters; i++)
	{		
		mUnscaledDelayFrames[i] = (delayFrameList[i] * GetSR_44100Ratio() ) >> 16;
	}
	
	ScaleDelayTimes();
}

static const int diffusionFrameList[] =
{
	105,
	176,
	246
};

void CPsFilterReverb::SetupDiffusion()
{
	int i;

	PsAssert(kNumberOfDiffusionStages == 3);	// diffusionFrameList has three elements currently
	
	for(i = 0; i < kNumberOfDiffusionStages; i++)
	{
		long delayFrames = (diffusionFrameList[i] * GetSR_44100Ratio() ) >> 16;
		
		PsAssert(kDiffusionBufferFrameSize > delayFrames);
		mDiffReadIndex[i] = kDiffusionBufferFrameSize - delayFrames;
		mDiffWriteIndex[i] = 0;
	}
}

// generated lopass filter coeffs for different sample rates
static const int lopassKList[] = {63467, 60227, 46883, 44824, 30573, 28693};
	
void CPsFilterReverb::SetupStereoizer()
{
	long delayFrames;
	
	int i;

	// output filter
	int freq = m_pSystem->GetConfig()->GetMixFrequency();

	if(freq <= 8000)
		i = 0;
	else if(freq <= 11025)
		i = 1;
	else if(freq <= 22050)
		i = 2;
	else if(freq <= 24000)
		i = 3;
	else if(freq <= 44100)
		i = 4;
	else
		i = 5;
	
	PsAssert(i >= 0 && i < 6);
	mLopassK = lopassKList[i]  /*30523*/;
	mFilterMemory = 0;
	delayFrames = (307 * GetSR_44100Ratio() ) >> 16;
	PsAssert(kStereoizerBufferFrameSize > delayFrames * 2);
	mStereoReadIndex = kStereoizerBufferFrameSize - delayFrames * 2;
	mStereoWriteIndex = 0;
}

// early reflection stuff
static const int earlyReflectionFrames[] =
 {0x00000f7b, 0x00000861, 0x00000de2, 0x00000ac2, 0x00000b74, 0x00001420, /*0x0000089d*/ 0x00000550};
 
static const int earlyReflectionsGains[] =
{
	50918,		// 	0.77695906162261963
	127900,		// 	1.9516065120697021
	59954,		//	0.91482698917388916
	87903,		// 	1.3413060903549194
	80038,		// 	1.2212871313095093
	34353,		// 	0.52418613433837891
	49152		//	0.75	 sent to diffused reverb
};

void CPsFilterReverb::SetupEarlyReflections()
{
	int i;
	
	mReflectionWriteIndex = 0;
	for(i = 0; i < kNumberOfEarlyReflections; i++)
	{
		int frames = (earlyReflectionFrames[i] * GetSR_44100Ratio() ) >> 16;
		mEarlyReflectionGain[i] = earlyReflectionsGains[i];
		mReflectionReadIndex[i] = kEarlyReflectionBufferFrameSize - frames;
	}
}

#define CIRCULAR_INCREMENT(index_, mask_)	\
	index_ = (index_ + 1) & mask_;


#if 1
//#define NO_STEREO_REVERB
void CPsFilterReverb::RunNewReverb(int *sourceP, int *destP, int nSampleFrames, int nStep)
{
	if(!mIsInitialized) return;	// we're not properly initialized for processing...

	if(CheckReverbType() || mSampleRate != m_pSystem->GetConfig()->GetMixFrequency())
	{
		// sample rate has changed
		mSampleRate = m_pSystem->GetConfig()->GetMixFrequency();
		
		GenerateDelayTimes();
		GenerateFeedbackValues();
		SetupDiffusion();
		SetupStereoizer();
		SetupEarlyReflections();
		
		return;
	}
	
	while(nSampleFrames-- > 0)
	{
		mEarlyReflectionBuffer[mReflectionWriteIndex] = *sourceP >> INPUTSHIFT;
		sourceP += nStep;
		CIRCULAR_INCREMENT( mReflectionWriteIndex, kEarlyReflectionBufferMask);
		
		int preDelayGain = (mEarlyReflectionGain[6] * mDiffusedBalance) >> 6;
		register int combInput = (mEarlyReflectionBuffer[mReflectionReadIndex[kNumberOfEarlyReflections - 1]] * preDelayGain) >> COEFF_SHIFT;
		CIRCULAR_INCREMENT( mReflectionReadIndex[kNumberOfEarlyReflections - 1], kEarlyReflectionBufferMask);

		register int reflSum = 0;
		register int diffInput = 0;
		int i;
		for(i = 0; i < kNumberOfCombFilters; i++)
		{
			// comb filter bank
			int tap1 = (mReverbBuffer[i][mReadIndex[i]] * mFeedbackList[i]) >> COMB_FILTER_SHIFT;
			// feed delay outputs back into delay inputs
			mReverbBuffer[i][mWriteIndex[i]] = combInput + tap1;
			diffInput += tap1;
			CIRCULAR_INCREMENT( mReadIndex[i], kCombBufferMask);
			CIRCULAR_INCREMENT( mWriteIndex[i], kCombBufferMask);

			// early reflections
			reflSum += (mEarlyReflectionBuffer[mReflectionReadIndex[i]] * mEarlyReflectionGain[i]) >> COEFF_SHIFT;
			// increment reflection indices
			CIRCULAR_INCREMENT( mReflectionReadIndex[i], kEarlyReflectionBufferMask);
		}

		diffInput >>= 1;

		register int temp;
		for(i = 0; i < kNumberOfDiffusionStages; i++)
		{
			temp =  mDiffusionBuffer[i][mDiffReadIndex[i]];
			mDiffusionBuffer[i][mDiffWriteIndex[i]] = diffInput + ((temp * kDiffusionFeedback) >> COEFF_SHIFT);
			diffInput = (-diffInput * kDiffusionFeedback + temp * kDiffusionDelayGain) >> COEFF_SHIFT;
			CIRCULAR_INCREMENT(mDiffReadIndex[i], kDiffusionBufferMask);
			CIRCULAR_INCREMENT(mDiffWriteIndex[i], kDiffusionBufferMask);
		}

		int wet =  reflSum + diffInput;	// reverb level

		// now run through its filter
		int filterOutput = (mLopassK * mFilterMemory) >> COEFF_SHIFT;
		mFilterMemory = wet  + mFilterMemory - filterOutput;

#ifdef NO_STEREO_REVERB
		*destP++ -= filterOutput;
		*destP++ -= filterOutput;
#else
		// stereoizer stage
		temp =  mStereoizerBuffer[mStereoReadIndex++];
		mStereoizerBuffer[mStereoWriteIndex++] = filterOutput + ((temp * kStereoizerFeedbackL) >> COEFF_SHIFT);
		*destP++ += (-filterOutput * kStereoizerFeedbackL + temp * kStereoizerDelayGainL) >> COEFF_SHIFT;
		
		temp =  mStereoizerBuffer[mStereoReadIndex];
		mStereoizerBuffer[mStereoWriteIndex] = filterOutput + ((temp * kStereoizerFeedbackR) >> COEFF_SHIFT);
		*destP++ += (-filterOutput * kStereoizerFeedbackR + temp * kStereoizerDelayGainR) >> COEFF_SHIFT;

		CIRCULAR_INCREMENT( mStereoReadIndex, kStereoizerBufferMask);
		CIRCULAR_INCREMENT( mStereoWriteIndex, kStereoizerBufferMask);
#endif

		// ADD in reverb to output buffer
//		*destP++ += reverbOutL << (INPUTSHIFT);
//		*destP++ += reverbOutR << (INPUTSHIFT);
	}
}
#else

void CPsFilterReverb::RunNewReverb(int *sourceP, int *destP, int nSampleFrames, int nStep)
{
	// get local state variables from global struct (for efficiency)
	int *delayBuffer1 = mReverbBuffer[0];
	int *delayBuffer2 = mReverbBuffer[1];
	int *delayBuffer3 = mReverbBuffer[2];
	int *delayBuffer4 = mReverbBuffer[3];
	int *delayBuffer5 = mReverbBuffer[4];
	int *delayBuffer6 = mReverbBuffer[5];

	int	intFeedback1 = mFeedbackList[0];
	int	intFeedback2 = mFeedbackList[1];
	int	intFeedback3 = mFeedbackList[2];
	int	intFeedback4 = mFeedbackList[3];
	int	intFeedback5 = mFeedbackList[4];
	int	intFeedback6 = mFeedbackList[5];
	
	int		readIndex1 = mReadIndex[0];
	int		readIndex2 = mReadIndex[1];
	int		readIndex3 = mReadIndex[2];
	int		readIndex4 = mReadIndex[3];
	int		readIndex5 = mReadIndex[4];
	int		readIndex6 = mReadIndex[5];
	
	int		writeIndex1 = mWriteIndex[0];
	int		writeIndex2 = mWriteIndex[1];
	int		writeIndex3 = mWriteIndex[2];
	int		writeIndex4 = mWriteIndex[3];
	int		writeIndex5 = mWriteIndex[4];
	int		writeIndex6 = mWriteIndex[5];
	
	// diffusion parameters	
	int	*diffusionBuffer1 = mDiffusionBuffer[0];
	int	*diffusionBuffer2 = mDiffusionBuffer[1];
	int	*diffusionBuffer3 = mDiffusionBuffer[2];
	
	int		diffReadIndex1 = mDiffReadIndex[0];
	int		diffReadIndex2 = mDiffReadIndex[1];
	int		diffReadIndex3 = mDiffReadIndex[2];

	int		diffWriteIndex1 = mDiffWriteIndex[0];
	int		diffWriteIndex2 = mDiffWriteIndex[1];
	int		diffWriteIndex3 = mDiffWriteIndex[2];
	
	

	int	*stereoizerBuffer = mStereoizerBuffer;

	int 	stereoReadIndex = mStereoReadIndex;
	int 	stereoWriteIndex = mStereoWriteIndex;



	// early reflection stuff
	int earlyReflectionGain1 = mEarlyReflectionGain[0];
	int earlyReflectionGain2 = mEarlyReflectionGain[1];
	int earlyReflectionGain3 = mEarlyReflectionGain[2];
	int earlyReflectionGain4 = mEarlyReflectionGain[3];
	int earlyReflectionGain5 = mEarlyReflectionGain[4];
	int earlyReflectionGain6 = mEarlyReflectionGain[5];
	
	// 0-127 corresponds to 0x to 2x the mEarlyReflectionGain[6] value
	int preDelayGain = (mEarlyReflectionGain[6] * mDiffusedBalance) >> 6;
	
	int		reflectionReadIndex1 = mReflectionReadIndex[0];
	int		reflectionReadIndex2 = mReflectionReadIndex[1];
	int		reflectionReadIndex3 = mReflectionReadIndex[2];
	int		reflectionReadIndex4 = mReflectionReadIndex[3];
	int		reflectionReadIndex5 = mReflectionReadIndex[4];
	int		reflectionReadIndex6 = mReflectionReadIndex[5];
	int		reflectionReadIndex7 = mReflectionReadIndex[6];
	
	int		reflectionWriteIndex = mReflectionWriteIndex;

	int	*earlyReflectionBuffer = mEarlyReflectionBuffer;


	// filter stuff
	int lopassK = mLopassK;
	int filterMemory = mFilterMemory;

	int framesToProcess = nSampleFrames;


	if(!mIsInitialized) return;	// we're not properly initialized for processing...

	if(CheckReverbType() || mSampleRate != m_pSystem->GetOutputFrequency())
	{
		// sample rate has changed
		mSampleRate = m_pSystem->GetOutputFrequency();
		
		GenerateDelayTimes();
		GenerateFeedbackValues();
		SetupDiffusion();
		SetupStereoizer();
		SetupEarlyReflections();
		
		return;
	}
	
	while(framesToProcess-- > 0)
	{
		int reverbOutL, reverbOutR;
		int diffOut1, diffOut2, diffOut3;
		int wet, combOutput, temp, combInput, diffInput, filterOutput, stereoInput;

		int input = *sourceP >> INPUTSHIFT;
		sourceP += nStep;

		// comb filter bank
		int tap1 = (delayBuffer1[readIndex1] * intFeedback1) >> COMB_FILTER_SHIFT;
		int tap2 = (delayBuffer2[readIndex2] * intFeedback2) >> COMB_FILTER_SHIFT;
		int tap3 = (delayBuffer3[readIndex3] * intFeedback3) >> COMB_FILTER_SHIFT;
		int tap4 = (delayBuffer4[readIndex4] * intFeedback4) >> COMB_FILTER_SHIFT;
		int tap5 = (delayBuffer5[readIndex5] * intFeedback5) >> COMB_FILTER_SHIFT;
		int tap6 = (delayBuffer6[readIndex6] * intFeedback6) >> COMB_FILTER_SHIFT;


		// early reflections
		int refl1 = (earlyReflectionBuffer[reflectionReadIndex1] * earlyReflectionGain1) >> COEFF_SHIFT;
		int refl2 = (earlyReflectionBuffer[reflectionReadIndex2] * earlyReflectionGain2) >> COEFF_SHIFT;
		int refl3 = (earlyReflectionBuffer[reflectionReadIndex3] * earlyReflectionGain3) >> COEFF_SHIFT;
		int refl4 = (earlyReflectionBuffer[reflectionReadIndex4] * earlyReflectionGain4) >> COEFF_SHIFT;
		int refl5 = (earlyReflectionBuffer[reflectionReadIndex5] * earlyReflectionGain5) >> COEFF_SHIFT;
		int refl6 = (earlyReflectionBuffer[reflectionReadIndex6] * earlyReflectionGain6) >> COEFF_SHIFT;
		int preDelay = (earlyReflectionBuffer[reflectionReadIndex7] * preDelayGain) >> COEFF_SHIFT;

		int reflSum = refl1 + refl2 + refl3 + refl4 + refl5 + refl6;

		earlyReflectionBuffer[reflectionWriteIndex] = input;

		// increment reflection indices
		CIRCULAR_INCREMENT( reflectionReadIndex1, kEarlyReflectionBufferMask);
		CIRCULAR_INCREMENT( reflectionReadIndex2, kEarlyReflectionBufferMask);
		CIRCULAR_INCREMENT( reflectionReadIndex3, kEarlyReflectionBufferMask);
		CIRCULAR_INCREMENT( reflectionReadIndex4, kEarlyReflectionBufferMask);
		CIRCULAR_INCREMENT( reflectionReadIndex5, kEarlyReflectionBufferMask);
		CIRCULAR_INCREMENT( reflectionReadIndex6, kEarlyReflectionBufferMask);
		CIRCULAR_INCREMENT( reflectionReadIndex7, kEarlyReflectionBufferMask);
		CIRCULAR_INCREMENT( reflectionWriteIndex, kEarlyReflectionBufferMask);

		combInput = preDelay;

		
		// feed delay outputs back into delay inputs
		delayBuffer1[writeIndex1] = combInput + tap1;
		delayBuffer2[writeIndex2] = combInput + tap2;
		delayBuffer3[writeIndex3] = combInput + tap3;
		delayBuffer4[writeIndex4] = combInput + tap4;
		delayBuffer5[writeIndex5] = combInput + tap5;
		delayBuffer6[writeIndex6] = combInput + tap6;
		
		// increment read indices
		CIRCULAR_INCREMENT( readIndex1, kCombBufferMask);
		CIRCULAR_INCREMENT( readIndex2, kCombBufferMask);
		CIRCULAR_INCREMENT( readIndex3, kCombBufferMask);
		CIRCULAR_INCREMENT( readIndex4, kCombBufferMask);
		CIRCULAR_INCREMENT( readIndex5, kCombBufferMask);
		CIRCULAR_INCREMENT( readIndex6, kCombBufferMask);
		
		// increment write indices
		CIRCULAR_INCREMENT( writeIndex1, kCombBufferMask);
		CIRCULAR_INCREMENT( writeIndex2, kCombBufferMask);
		CIRCULAR_INCREMENT( writeIndex3, kCombBufferMask);
		CIRCULAR_INCREMENT( writeIndex4, kCombBufferMask);
		CIRCULAR_INCREMENT( writeIndex5, kCombBufferMask);
		CIRCULAR_INCREMENT( writeIndex6, kCombBufferMask);

		combOutput = (tap1 + tap2 + tap3 + tap4 + tap5 + tap6) >> 1;

		diffInput = combOutput;

		
		// diffusion 1
		temp =  diffusionBuffer1[diffReadIndex1];
		diffusionBuffer1[diffWriteIndex1] = diffInput + ((temp * kDiffusionFeedback) >> COEFF_SHIFT);
		diffOut1 = (-diffInput * kDiffusionFeedback + temp * kDiffusionDelayGain) >> COEFF_SHIFT;
		
		// diffusion 2
		temp =  diffusionBuffer2[diffReadIndex2];
		diffusionBuffer2[diffWriteIndex2] = diffOut1 + ((temp * kDiffusionFeedback) >> COEFF_SHIFT);
		diffOut2 = (-diffOut1 * kDiffusionFeedback + temp * kDiffusionDelayGain) >> COEFF_SHIFT;
		
		// diffusion 3
		temp =  diffusionBuffer3[diffReadIndex3];
		diffusionBuffer3[diffWriteIndex3] = diffOut2 + ((temp * kDiffusionFeedback) >> COEFF_SHIFT);
		diffOut3 = (-diffOut2 * kDiffusionFeedback + temp * kDiffusionDelayGain) >> COEFF_SHIFT;
		
		
		// update diffusion indices
		CIRCULAR_INCREMENT( diffReadIndex1, kDiffusionBufferMask);
		CIRCULAR_INCREMENT( diffReadIndex2, kDiffusionBufferMask);
		CIRCULAR_INCREMENT( diffReadIndex3, kDiffusionBufferMask);
		
		CIRCULAR_INCREMENT( diffWriteIndex1, kDiffusionBufferMask);
		CIRCULAR_INCREMENT( diffWriteIndex2, kDiffusionBufferMask);
		CIRCULAR_INCREMENT( diffWriteIndex3, kDiffusionBufferMask);

		wet =  reflSum + diffOut3;	// reverb level




		// now run through its filter
		filterOutput = (lopassK * filterMemory) >> COEFF_SHIFT;
		filterMemory = wet  + filterMemory - filterOutput;



		stereoInput = filterOutput;


		// stereoizer stage
		temp =  stereoizerBuffer[stereoReadIndex];
		stereoizerBuffer[stereoWriteIndex] = stereoInput + ((temp * kStereoizerFeedbackL) >> COEFF_SHIFT);
		reverbOutL = (-stereoInput * kStereoizerFeedbackL + temp * kStereoizerDelayGainL) >> COEFF_SHIFT;
		CIRCULAR_INCREMENT( stereoReadIndex, kStereoizerBufferMask);
		CIRCULAR_INCREMENT( stereoWriteIndex, kStereoizerBufferMask);
		
		temp =  stereoizerBuffer[stereoReadIndex];
		stereoizerBuffer[stereoWriteIndex] = stereoInput + ((temp * kStereoizerFeedbackR) >> COEFF_SHIFT);
		reverbOutR = (-stereoInput * kStereoizerFeedbackR + temp * kStereoizerDelayGainR) >> COEFF_SHIFT;

		CIRCULAR_INCREMENT( stereoReadIndex, kStereoizerBufferMask);
		CIRCULAR_INCREMENT( stereoWriteIndex, kStereoizerBufferMask);

		// ADD in reverb to output buffer
		*destP++ = reverbOutL;
		*destP++ = reverbOutR;
//		destP++;
	}


	// put local copies of indices back into global struct
	mReadIndex[0] = readIndex1;
	mReadIndex[1] = readIndex2;
	mReadIndex[2] = readIndex3;
	mReadIndex[3] = readIndex4;
	mReadIndex[4] = readIndex5;
	mReadIndex[5] = readIndex6;
	
	mWriteIndex[0] = writeIndex1;
	mWriteIndex[1] = writeIndex2;
	mWriteIndex[2] = writeIndex3;
	mWriteIndex[3] = writeIndex4;
	mWriteIndex[4] = writeIndex5;
	mWriteIndex[5] = writeIndex6;

	mDiffReadIndex[0] = diffReadIndex1;
	mDiffReadIndex[1] = diffReadIndex2;
	mDiffReadIndex[2] = diffReadIndex3;

	mDiffWriteIndex[0] = diffWriteIndex1;
	mDiffWriteIndex[1] = diffWriteIndex2;
	mDiffWriteIndex[2] = diffWriteIndex3;

	mStereoReadIndex = stereoReadIndex;
	mStereoWriteIndex = stereoWriteIndex;
	
	mReflectionReadIndex[0] = reflectionReadIndex1;
	mReflectionReadIndex[1] = reflectionReadIndex2;
	mReflectionReadIndex[2] = reflectionReadIndex3;
	mReflectionReadIndex[3] = reflectionReadIndex4;
	mReflectionReadIndex[4] = reflectionReadIndex5;
	mReflectionReadIndex[5] = reflectionReadIndex6;
	mReflectionReadIndex[6] = reflectionReadIndex7;
	
	mReflectionWriteIndex = reflectionWriteIndex;

	mFilterMemory = filterMemory;
}
#endif
