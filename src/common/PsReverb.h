#ifndef __PSFILTERREVERB_H__
#define __PSFILTERREVERB_H__

#include "PsObject.h"
#include "PsMath.h"


#define kCombBufferFrameSize			1024	/* 5000 */
#define kDiffusionBufferFrameSize		1024	/* 4410 */
#define kStereoizerBufferFrameSize		1024	/* 1000 */
#define kEarlyReflectionBufferFrameSize	0x2000	/* 0x1500 */

#define kCombBufferMask					(kCombBufferFrameSize - 1)
#define kDiffusionBufferMask			(kDiffusionBufferFrameSize - 1)
#define kStereoizerBufferMask			(kStereoizerBufferFrameSize - 1)
#define kEarlyReflectionBufferMask		(kEarlyReflectionBufferFrameSize - 1)


#define kNumberOfCombFilters		6
#define kNumberOfEarlyReflections	(kNumberOfCombFilters + 1)


#define kNumberOfDiffusionStages	3

class CPsFilterReverb : public CPsObject
{
public:
	enum {
		REVERB_TYPE_1, // no reverb
		REVERB_TYPE_8, // Early reflections
		REVERB_TYPE_9, // Basement
		REVERB_TYPE_10, // Banquet hall
		REVERB_TYPE_11 // Catacombs
	};
	
	bool InitNewReverb(CPsAudioSystem *pSystem, int reverbType);
	void ShutdownNewReverb();
	void RunNewReverb(int *sourceP, int*destP, int nSampleFrames, int nStep);

	unsigned int GetSR_44100Ratio(){
		return GetSamplingRate() / 44100UL;
	}
	
	unsigned int Get44100_SRRatio(){
		return (44100UL << 16L) / (GetSamplingRate() >> 16L);
	}
	
	CPsFilterReverb();
	virtual ~CPsFilterReverb();
	
protected:
	unsigned int GetSamplingRate();
	bool CheckReverbType();
	void ScaleDelayTimes();
	void GenerateFeedbackValues();
	void GenerateDelayTimes();
	void SetupDiffusion();
	void SetupStereoizer();
	void SetupEarlyReflections();

	CPsAudioSystem *m_pSystem;
	int m_reverbType;

	bool			mIsInitialized;
	unsigned int	mSampleRate;
	int				mReverbType;	
	
	/* early reflection params */
	int				*mEarlyReflectionBuffer;
	int				mEarlyReflectionGain[kNumberOfEarlyReflections];
	int				mReflectionWriteIndex;
	int				mReflectionReadIndex[kNumberOfEarlyReflections];
	
	
	/* comb filter params */	
	int				*mReverbBuffer[kNumberOfCombFilters];
	
	int				mReadIndex[kNumberOfCombFilters];
	int				mWriteIndex[kNumberOfCombFilters];
	
	int				mUnscaledDelayFrames[kNumberOfCombFilters];
	int				mDelayFrames[kNumberOfCombFilters];
	
	int 			mFeedbackList[kNumberOfCombFilters];
	
	int				mRoomSize;
	int				mRoomChoice;
	int				mMaxRegen;		// 0-127
	int				mDiffusedBalance;
	
	/* diffusion params */
	int				*mDiffusionBuffer[kNumberOfDiffusionStages];
	int				mDiffReadIndex[kNumberOfDiffusionStages];
	int				mDiffWriteIndex[kNumberOfDiffusionStages];
	
	/* output filter */
	int				mLopassK;
	int				mFilterMemory;
	
	/* stereoizer params */
	int				*mStereoizerBuffer;
	int				mStereoReadIndex;
	int				mStereoWriteIndex;
};

#endif
