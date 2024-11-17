#ifndef __PSMIXER_H__
#define __PSMIXER_H__

#include "PsObject.h"
#include "PsMath.h"
#include "PsReverbSchroeder.h"
#include "PsFilter.h"
#include "PsChorus.h"

//#define USE_STEREO_DELAY
//#define AUTO_ADJUST_MIXER_VOLUME
#define DEFAULT_VOLUME (F_ONE * 3 / 4)


class CPsMixer : public CPsGenerator  
{
public:
	enum {
#ifdef _CONSOLE
		MAX_CHANNELS = 74,
#else
		MAX_CHANNELS = 74,
#endif
		MAX_PLAYER = 1,

		PAN_DELAY_TIME = PSFLOAT2F(0.5), //ms
		STEREO_DELAY_TIME = PSFLOAT2F(0.42), //ms
	};

	void SetReverbVolume(F16 vol);
	void RemovePlayer(CPsPlayer *p);
	void AddPlayer(CPsPlayer *p);
	void RemoveChannel(CPsChannel *p);
	int AddChannel(CPsChannel *pChannel);
	unsigned int Write(NativeSample *pOut);
	bool Open(CPsAudioSystem *pSystem, int nTickSize, bool isStereo);
	void Close();
	CPsMixer();
	virtual ~CPsMixer();

	void SetPanSeperation(F32 sep)
	{
		PsAssert(sep <= F_ONE * 2);
		m_fPanSep = sep;
	}

	void SetSurround(bool enabled){
		m_isSurround = enabled;
	}

	void SetSwapLR(bool swapped){
		m_swapLR = swapped;
	}

	bool IsSurround(){
		return m_isSurround;
	}

	bool IsSwapLR(){
		return m_swapLR;
	}

	F32 GetPanSeperation(){
		return m_fPanSep;
	}

	F16 GetReverbVolume(){
		return m_fReverbVolume;
	}
	
	int GetMixChannels(){
		return m_nMixChannels;
	}
	
	F16 GetMixVolume(){
		return m_fVolume;
	}
	
	void SetMixVolume(F16 vol){
		m_fVolume = vol;
		if(vol < 1)
			vol = 1;
		m_maxValueBeforeMix = (int)(((F64)I2F(32767)) / vol);
	}
	
	F32 GetMixTime(){
		return m_fMixTime;
	}

	void SetBassVolume(F16 vol){
		PsAssert(vol >= 0 && vol <= F_ONE);
		m_fBassVolume = vol;
		m_LPFilter.SetParameters(m_pMath, m_nBassCutoff, m_fBassVolume * 2 + F_ONE, m_pConfig->GetMixFrequency());
	}
	
	void SetBassCutoff(int freq){
		m_nBassCutoff = freq;
		m_LPFilter.SetParameters(m_pMath, m_nBassCutoff, m_fBassVolume * 2 + F_ONE, m_pConfig->GetMixFrequency());
	}

	F16 GetBassVolume(){
		return m_fBassVolume;
	}

	int GetBassCutoff(){
		return m_nBassCutoff;
	}
	
#ifdef AUTO_ADJUST_MIXER_VOLUME
	void ResetAGC(){
		SetMixVolume(DEFAULT_VOLUME);
	}
	void SetAGC(bool enable){
		m_isAGC = enable;
	}
#else
	void ResetAGC(){
	}
	void SetAGC(bool enable){
	}
#endif
	
protected:
	void DoAGC();
	void ProcessReverb(NativeSample *pBuf);
	void ProcessStereoDelay(NativeSample *pBuf);
	void Generate16outputStereo(NativeSample *dest16);
	void MixReverb();
	void MixChannel(CPsChannel *pChannel);

	CPsMath *m_pMath;
	CPsAudioConfig *m_pConfig;

	int m_nSampleChannel;
	int m_nTickSize;//number of frames per tick
	int *m_pDryBuffer;
	int *m_pBuffer;
	int *m_pChorusBuffer;
	int *m_pReverbBuffer;
	
	bool m_hasPanDelay;
	bool m_swapLR;
	bool m_isSurround;
	F32 m_fPanSep;
	
	int m_nMixChannels;
	F32 m_fMixTime;//in ms
	CPsChannel *m_pChannels[MAX_CHANNELS];
	CPsPlayer *m_pPlayers[MAX_PLAYER];

	F16 m_fVolume;
	F16 m_fChorusVolume;
	int m_maxValueBeforeMix;

	F16 m_fReverbVolume;
	CPsReverbSchroeder m_reverb;
	CPsChorus m_chorus;

	unsigned short m_panDelayGap;//in frame
	unsigned short m_nRamp;
	unsigned int m_invRamp;

	F16 m_fBassVolume;
	int m_nBassCutoff;

#ifdef USE_STEREO_DELAY
	int *m_pLPBuffer;
	int *m_pMPBuffer;
	int *m_pHPBuffer;
	unsigned short m_hpDelayGap;
	unsigned short m_mpDelayGap;
	CPsFilterLowPass m_LPL, m_LPR;
	CPsFilterLHPass m_HPL, m_HPR;
#endif
	CPsFilterLowPass m_LPFilter;

#ifdef AUTO_ADJUST_MIXER_VOLUME
	F32 m_fTotalVolume;
	bool m_isAGC;
#endif
};

#endif
