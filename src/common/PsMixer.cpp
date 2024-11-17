#include "PsMixer.h"
#include "PsPatch.h"
#include "PsAudioSystem.h"
#include "PsSys.h"

#define MIX_USE_INTERPOLATE
#define MIX_SMOOTH_VOLUME

const F16 pan_diff[129] = {
    0,   201,   402,   603,   803,  1004,  1205,  1405,
	1605,  1805,  2005,  2204,  2404,  2602,  2801,  2998,
	3196,  3393,  3589,  3785,  3980,  4175,  4369,  4563,
	4756,  4948,  5139,  5329,  5519,  5708,  5896,  6083,
	6269,  6455,  6639,  6822,  7005,  7186,  7366,  7545,
	7723,  7900,  8075,  8249,  8423,  8594,  8765,  8934,
	9102,  9268,  9434,  9597,  9759,  9920, 10079, 10237,
	10393, 10548, 10701, 10853, 11002, 11150, 11297, 11442,
	11585, 11726, 11866, 12003, 12139, 12273, 12406, 12536,
	12664, 12791, 12916, 13038, 13159, 13278, 13395, 13510,
	13622, 13733, 13842, 13948, 14053, 14155, 14255, 14353,
	14449, 14543, 14634, 14723, 14810, 14895, 14978, 15058,
	15136, 15212, 15286, 15357, 15426, 15492, 15557, 15618,
	15678, 15735, 15790, 15842, 15893, 15940, 15985, 16028,
	16069, 16107, 16142, 16175, 16206, 16234, 16260, 16284,
	16305, 16323, 16339, 16353, 16364, 16372, 16379, 16384, 16384
};

CPsMixer::CPsMixer()
{
	m_pMath = NULL;
	m_pConfig = NULL;

	m_pBuffer = NULL;
	m_pDryBuffer = NULL;
	m_pChorusBuffer = NULL;
	m_pReverbBuffer = NULL;
	SetMixVolume(DEFAULT_VOLUME);
	m_fReverbVolume = F_ONE * 1 / 8;
	m_fChorusVolume = 0;
	m_fPanSep = F_ONE;
	m_swapLR = false;
	m_isSurround = false;
	m_hasPanDelay = true;

	m_fBassVolume = 0;
	m_nBassCutoff = 80;
#ifdef USE_STEREO_DELAY
	m_pLPBuffer = NULL;
	m_pMPBuffer = NULL;
	m_pHPBuffer = NULL;
#endif

#ifdef AUTO_ADJUST_MIXER_VOLUME
	m_isAGC = false;
#endif
	CPsSys::memset(m_pChannels, 0, sizeof(m_pChannels));
	CPsSys::memset(m_pPlayers, 0, sizeof(m_pPlayers));
}

CPsMixer::~CPsMixer()
{
	Close();
}

bool CPsMixer::Open(CPsAudioSystem *pSystem, int nTickSize, bool isStereo)
{
	if(m_pBuffer)
		Close();

	m_pMath = pSystem->GetMath();
	m_pConfig = pSystem->GetConfig();

	m_nTickSize = nTickSize;

	m_nMixChannels = 0;
	m_nSampleChannel = isStereo?2:1;
	
	m_panDelayGap = F2I(m_pConfig->GetMixFrequency() * PAN_DELAY_TIME) / 1000;
	m_nRamp = m_pConfig->GetMixFrequency() / 1000;
	PsAssert(m_nRamp < m_nTickSize);
	m_invRamp = F_ONE / m_nRamp;

	m_pBuffer = new int[(m_panDelayGap + nTickSize) * m_nSampleChannel];
	if(!m_pBuffer)
		return false;
	CPsSys::memset(m_pBuffer, 0, (m_panDelayGap + m_nTickSize) * sizeof(int) * m_nSampleChannel);

	m_pDryBuffer = new int[nTickSize * m_nSampleChannel];
	if(!m_pDryBuffer)
	{
		Close();
		return false;
	}

	m_pChorusBuffer = new int[nTickSize * m_nSampleChannel];
	if(!m_pChorusBuffer)
	{
		Close();
		return false;
	}
	CPsSys::memset(m_pChorusBuffer, 0, m_nTickSize * sizeof(int) * m_nSampleChannel);

	m_pReverbBuffer = new int[nTickSize * m_nSampleChannel];
	if(!m_pReverbBuffer)
	{
		Close();
		return false;
	}
	CPsSys::memset(m_pReverbBuffer, 0, m_nTickSize * sizeof(int) * m_nSampleChannel);


#ifdef USE_STEREO_DELAY
	m_pLPBuffer = new int[nTickSize * m_nSampleChannel];
	if(!m_pLPBuffer)
	{
		Close();
		return false;
	}
	m_mpDelayGap = F2I(m_pConfig->GetMixFrequency() * STEREO_DELAY_TIME) / 1000;
	m_pMPBuffer = new int[(m_mpDelayGap + nTickSize) * m_nSampleChannel];
	if(!m_pMPBuffer)
	{
		Close();
		return false;
	}
	m_hpDelayGap = m_mpDelayGap * 2;
	m_pHPBuffer = new int[(m_hpDelayGap + nTickSize) * m_nSampleChannel];
	if(!m_pHPBuffer)
	{
		Close();
		return false;
	}

	m_LPL.InitLowPass(pSystem->GetMath(), 800, PSFLOAT2F(0.5), m_pConfig->GetMixFrequency());
	m_LPR.InitLowPass(pSystem->GetMath(), 800, PSFLOAT2F(0.5), m_pConfig->GetMixFrequency());
	m_HPL.InitHiPass(pSystem->GetMath(), 800, PSFLOAT2F(0.5), m_pConfig->GetMixFrequency());
	m_HPR.InitHiPass(pSystem->GetMath(), 800, PSFLOAT2F(0.5), m_pConfig->GetMixFrequency());
#endif
	m_LPFilter.InitLowPass();

	m_reverb.Init(m_pConfig);
	m_chorus.Init(m_pConfig);
	return true;
}

void CPsMixer::Close()
{
	m_chorus.Shutdown();
	m_reverb.Shutdown();

	delete m_pReverbBuffer;
	m_pReverbBuffer = NULL;

	delete m_pChorusBuffer;
	m_pChorusBuffer = NULL;

	delete m_pDryBuffer;
	m_pDryBuffer = NULL;

	delete m_pBuffer;
	m_pBuffer = NULL;
#ifdef USE_STEREO_DELAY
	delete m_pHPBuffer;
	m_pHPBuffer = NULL;
	delete m_pMPBuffer;
	m_pMPBuffer = NULL;
	delete m_pLPBuffer;
	m_pLPBuffer = NULL;
#endif

	CPsSys::memset(m_pChannels, 0, sizeof(m_pChannels));
	CPsSys::memset(m_pPlayers, 0, sizeof(m_pPlayers));
	
	m_pMath = NULL;
	m_pConfig = NULL;
}

inline int MulShift(int m, int n, int nShift)
{
	//todo: optimize the following by ARM asm
	F64 r = (F64)m * n;
	return (int)(r >> nShift);
}

void CPsMixer::MixChannel(CPsChannel *pChannel)
{
	PsAssert(pChannel->GetSampleData()->nBitPerSample == 16);
	F32 pan = pChannel->GetPan();

	pan = FMUL16(pan, m_fPanSep);
	pan = pan + F_ONE / 2;
	if(pan < 0)
		pan = 0;
	else if(pan > F_ONE)
		pan = F_ONE;
	
	F32 f = pChannel->GetMixVolume();
//	f = FMUL32(f, F_ONE * 3 / 4);

//	PsAssert(f < 32768);
	if(f > 32767)
		f = 32767;
	
	F32 lvol, rvol;
	lvol = FMUL16(f, m_pMath->Sqrt(F_ONE - pan));
	rvol = FMUL16(f, m_pMath->Sqrt(pan));
//	lvol = FMUL16(f, F_ONE - pan);
//	rvol = FMUL16(f, pan);

	F32 bal = pChannel->GetBalance() * 2;
	if(bal < 0)
		rvol = FMUL16(rvol, F_ONE + bal);
	else if(bal > 0)
		lvol = FMUL16(lvol, F_ONE - bal);

#ifdef AUTO_ADJUST_MIXER_VOLUME
	m_fTotalVolume += lvol + rvol;
#endif

	const CPsSampleData *pSampleData = pChannel->GetSampleData();
	
#ifdef USE_64BIT_MIX
	F64 fMixPosition = pChannel->m_fMixPosition;
	F64 fLoopEnd = pSampleData->loopStart + pSampleData->loopLength;
#else
	F32 fMixPosition = pChannel->m_fMixPosition;
	F32 fLoopEnd = pSampleData->loopStart + pSampleData->loopLength;
#endif
	F32 fRelativeFreq = pChannel->GetMixFrequency();
	short *pS = (short*)pSampleData->pData;

	int *pDryBuffer = m_pDryBuffer;
	
	if(pChannel->GetSampleData()->nChannel == 1)
	{
		#ifdef MIX_SMOOTH_VOLUME
		if((pChannel->m_fLastLVol != lvol || pChannel->m_fLastRVol != rvol))
		{
			F32 lv = I2F(pChannel->m_fLastLVol);
			F32 dlvol = (lvol - pChannel->m_fLastLVol) * m_invRamp;

			F32 rv = I2F(pChannel->m_fLastRVol);
			F32 drvol = (rvol - pChannel->m_fLastRVol) * m_invRamp;
			int *pEnd = pDryBuffer + m_nRamp * 2;

			while(pDryBuffer < pEnd)
			{
				int smp = pS[F2I(fMixPosition)];
		#ifdef MIX_USE_INTERPOLATE
				int nsmp = pS[F2I(fMixPosition + F_ONE)];
				smp += F2I((nsmp - smp) * (fMixPosition & (F_ONE - 1)));
		#endif
				lv += dlvol;
				rv += drvol;
				*pDryBuffer++ = smp * F2I(lv) >> (F_SHIFT - MIX_SHIFT);
				*pDryBuffer++ = smp * F2I(rv) >> (F_SHIFT - MIX_SHIFT);

				fMixPosition += fRelativeFreq;
				if(fMixPosition >= fLoopEnd)
				{
					if(pSampleData->loopType == CPsPatch::SAMPLE_LOOPFORWARD)
					{
						fMixPosition -= pSampleData->loopLength;
					}
					else
					{
						pChannel->SetState(CPsChannel::STATE_STOPPED);
						break;
					}
				}
			}
		}
		#endif

		if(pChannel->GetState() != CPsChannel::STATE_STOPPED)
		{
			int *pEnd = m_pDryBuffer + m_nTickSize * 2;

			while(pDryBuffer < pEnd)
			{
				int smp = pS[F2I(fMixPosition)];
		#ifdef MIX_USE_INTERPOLATE
				int nsmp = pS[F2I(fMixPosition + F_ONE)];
				smp += F2I((nsmp - smp) * (fMixPosition & (F_ONE - 1)));
		#endif
				*pDryBuffer++ = smp * lvol >> (F_SHIFT - MIX_SHIFT);
				*pDryBuffer++ = smp * rvol >> (F_SHIFT - MIX_SHIFT);

				fMixPosition += fRelativeFreq;
				if(fMixPosition >= fLoopEnd)
				{
					if(pSampleData->loopType == CPsPatch::SAMPLE_LOOPFORWARD)
					{
						fMixPosition -= pSampleData->loopLength;
					}
					else
					{
						pChannel->SetState(CPsChannel::STATE_STOPPED);
						break;
					}
				}
			}
		}
	}
	else if(pChannel->GetSampleData()->nChannel == 2)
	{
		#ifdef MIX_SMOOTH_VOLUME
		if((pChannel->m_fLastLVol != lvol || pChannel->m_fLastRVol != rvol))
		{
			F32 lv = I2F(pChannel->m_fLastLVol);
			F32 dlvol = (lvol - pChannel->m_fLastLVol) * m_invRamp;

			F32 rv = I2F(pChannel->m_fLastRVol);
			F32 drvol = (rvol - pChannel->m_fLastRVol) * m_invRamp;
			int *pEnd = pDryBuffer + m_nRamp * 2;

			while(pDryBuffer < pEnd)
			{
				int wholePart = F2I(fMixPosition) * 2;
				int smp = pS[wholePart];
		#ifdef MIX_USE_INTERPOLATE
				int nsmp = pS[wholePart + 2];
				smp += F2I((nsmp - smp) * (fMixPosition & (F_ONE - 1)));
		#endif
				lv += dlvol;
				*pDryBuffer++ = smp * F2I(lv) >> (F_SHIFT - MIX_SHIFT);

				smp = pS[wholePart + 1];
		#ifdef MIX_USE_INTERPOLATE
				nsmp = pS[wholePart + 3];
				smp += F2I((nsmp - smp) * (fMixPosition & (F_ONE - 1)));
		#endif
				rv += drvol;
				*pDryBuffer++ = smp * F2I(rv) >> (F_SHIFT - MIX_SHIFT);

				fMixPosition += fRelativeFreq;
				if(fMixPosition >= fLoopEnd)
				{
					if(pSampleData->loopType == CPsPatch::SAMPLE_LOOPFORWARD)
					{
						fMixPosition -= pSampleData->loopLength;
					}
					else
					{
						pChannel->SetState(CPsChannel::STATE_STOPPED);
						break;
					}
				}
			}
		}
		#endif

		if(pChannel->GetState() != CPsChannel::STATE_STOPPED)
		{
			int *pEnd = m_pDryBuffer + m_nTickSize * 2;

			while(pDryBuffer < pEnd)
			{
				int wholePart = F2I(fMixPosition) * 2;
				int smp = pS[wholePart];
		#ifdef MIX_USE_INTERPOLATE
				int nsmp = pS[wholePart + 2];
				smp += F2I((nsmp - smp) * (fMixPosition & (F_ONE - 1)));
		#endif
				*pDryBuffer++ = smp * lvol >> (F_SHIFT - MIX_SHIFT);

				smp = pS[wholePart + 1];
		#ifdef MIX_USE_INTERPOLATE
				nsmp = pS[wholePart + 3];
				smp += F2I((nsmp - smp) * (fMixPosition & (F_ONE - 1)));
		#endif
				*pDryBuffer++ = smp * rvol >> (F_SHIFT - MIX_SHIFT);

				fMixPosition += fRelativeFreq;
				if(fMixPosition >= fLoopEnd)
				{
					if(pSampleData->loopType == CPsPatch::SAMPLE_LOOPFORWARD)
					{
						fMixPosition -= pSampleData->loopLength;
					}
					else
					{
						pChannel->SetState(CPsChannel::STATE_STOPPED);
						break;
					}
				}
			}
		}
	}
	else
		PsAssert(false);
	pChannel->m_fMixPosition = fMixPosition;
	pChannel->m_fLastLVol = lvol;
	pChannel->m_fLastRVol = rvol;

	int *pD1, *pD2;
	int ndelay = pan_diff[F2I(pan * 128)] - pan_diff[128 - F2I(pan * 128)];
	ndelay = F2I(ndelay * m_panDelayGap);
	if(ndelay < 0)
		ndelay = -ndelay;
	if(ndelay >= m_panDelayGap)
		ndelay = m_panDelayGap - 1;
	if(!m_hasPanDelay || ndelay == 0)
	{
		pD1 = m_pBuffer;
		pD2 = m_pBuffer;
	}
	else
	{
		if(pan < F_ONE / 2)
		{
			pD1 = m_pBuffer;
			pD2 = m_pBuffer + ndelay * m_nSampleChannel;
		}
		else
		{
			pD1 = m_pBuffer + ndelay * m_nSampleChannel;
			pD2 = m_pBuffer;
		}
	}

	F32 fChorus = m_fChorusVolume + pChannel->GetChorus();
	if(fChorus > F_ONE)
		fChorus = F_ONE;

	F32 fReverb = m_fReverbVolume + pChannel->GetReverb() * 3 / 4;
	if(fReverb > F_ONE)
		fReverb = F_ONE;
	int *pChorus = m_pChorusBuffer;
	int *pReverb = m_pReverbBuffer;
	int n = pDryBuffer - m_pDryBuffer;
	int i = 0;
#ifdef FULL_CHORUS_REVERB_MIX
	while(i < n)
	{
		int v = m_pDryBuffer[i];
		pD1[i] += v;
		pChorus[i] += MulShift(v, fChorus, F_SHIFT);
		pReverb[i] += MulShift(v, fReverb, F_SHIFT);
		++i;
		v = m_pDryBuffer[i];
		pD2[i] += v;
		pChorus[i] += MulShift(v, fChorus, F_SHIFT);
		pReverb[i] += MulShift(v, fReverb, F_SHIFT);
		++i;
	}
#else
	fChorus <<= 18;
	fReverb <<= 6;

	if(fChorus)
	{
		while(i < n)
		{
			int v = m_pDryBuffer[i];
			pD1[i] += v;
			pChorus[i] += (F64)v * fChorus >> 32;
			pReverb[i] += (F64)v * fReverb >> 32;
			++i;
			v = m_pDryBuffer[i];
			pD2[i] += v;
			pChorus[i] += (F64)v * fChorus >> 32;
			pReverb[i] += (F64)v * fReverb >> 32;
			++i;
		}
	}
	else
	{
		while(i < n)
		{
			int v = m_pDryBuffer[i];
			pD1[i] += v;
			pReverb[i] += (F64)v * fReverb >> 32;
			++i;
			v = m_pDryBuffer[i];
			pD2[i] += v;
			pReverb[i] += (F64)v * fReverb >> 32;
			++i;
		}
	}
#endif
}

unsigned int CPsMixer::Write(NativeSample *pOut)
{
	int i;

	for(i = 0; i < MAX_PLAYER; i++)
		if(m_pPlayers[i])
			m_pPlayers[i]->Update(false);

	m_fMixTime = CPsSys::GetTickInMS();
	m_nMixChannels = 0;
	CPsSys::memcpy(m_pBuffer, m_pBuffer + m_nTickSize * m_nSampleChannel, m_panDelayGap * sizeof(int) * m_nSampleChannel);
	CPsSys::memset(m_pBuffer + m_panDelayGap * m_nSampleChannel, 0, m_nTickSize * sizeof(int) * m_nSampleChannel);
	CPsSys::memset(m_pChorusBuffer, 0, m_nTickSize * sizeof(int) * m_nSampleChannel);
	CPsSys::memset(m_pReverbBuffer, 0, m_nTickSize * sizeof(int) * m_nSampleChannel);

#ifdef AUTO_ADJUST_MIXER_VOLUME
	m_fTotalVolume = 0;
#endif
	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if(m_pChannels[i])
		{
			m_nMixChannels++;
			m_pChannels[i]->Update();
			if(m_pChannels[i]->GetState() == CPsChannel::STATE_PLAYING)
				MixChannel(m_pChannels[i]);
			if(m_pChannels[i]->GetState() == CPsChannel::STATE_STOPPED)
				RemoveChannel(m_pChannels[i]);
		}
	}
	m_fMixTime = I2F(CPsSys::GetTickInMS() - m_fMixTime);
	for(i = 0; i < MAX_PLAYER; i++)
		if(m_pPlayers[i])
			m_pPlayers[i]->Update(true);

#ifdef AUTO_ADJUST_MIXER_VOLUME
	if(m_isAGC)
		DoAGC();
#endif

#ifdef USE_STEREO_DELAY
	ProcessStereoDelay(pOut);
#endif
	m_chorus.Process(m_pChorusBuffer, m_pBuffer, m_nTickSize);
	m_reverb.Process(m_pReverbBuffer, m_pBuffer, m_nTickSize);
	m_LPFilter.ProcessStereo(m_pBuffer, m_nTickSize);
	Generate16outputStereo(pOut);
//	ProcessReverb(pOut);

	return m_nTickSize;
}

#ifdef AUTO_ADJUST_MIXER_VOLUME
void CPsMixer::DoAGC()
{
	F32 AGCVol = 0x26000000 / (m_fTotalVolume + 1);
	if(m_fVolume > AGCVol)
	{
		int val = 0;
		for(int i = 0; i < m_nTickSize * m_nSampleChannel; i++)
		{
			int v = m_pBuffer[i] >> MIX_SHIFT;
			if(v < 0)
				v = -v;
			if(v > val)
				val = v;
		}
		if(FMUL32(val, m_fVolume) >= 0x6000)
			m_fVolume -= F_ONE / 2;
	}
	if(m_fVolume < DEFAULT_VOLUME)
	{
		m_fVolume += F_ONE / 16 / F2I(m_pConfig->GetUpdateFrequency());
	}
	printf("vol: %d, %d, %d\n", m_fVolume, m_fTotalVolume, AGCVol);
}
#endif

int CPsMixer::AddChannel(CPsChannel *pChannel)
{
	int i;
	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if(m_pChannels[i] == pChannel)
		{
			return i;
		}
		if(!m_pChannels[i])
		{
			m_pChannels[i] = pChannel;
			return i;
		}
	}
	return -1;
}

void CPsMixer::RemoveChannel(CPsChannel *p)
{
	int i;
	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if(m_pChannels[i] == p)
		{
			m_pChannels[i] = NULL;
			return;
		}
	}
}

void CPsMixer::AddPlayer(CPsPlayer *p)
{
	int i;
	for(i = 0; i < MAX_PLAYER; i++)
	{
		if(m_pPlayers[i] == p)
			return;
		if(!m_pPlayers[i])
		{
			m_pPlayers[i] = p;
			return;
		}
	}
}

void CPsMixer::RemovePlayer(CPsPlayer *p)
{
	int i;
	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if(m_pPlayers[i] == p)
		{
			m_pPlayers[i] = NULL;
			return;
		}
	}
}

void CPsMixer::SetReverbVolume(F16 vol)
{
	m_fReverbVolume = vol;
//	m_reverb.SetWet(m_fReverbVolume * 3 / 4 + F_ONE / 4);
}

void CPsMixer::Generate16outputStereo(NativeSample *dest16)
{
	register int v;
#ifdef USE_STEREO_DELAY
	register int *pS = m_pLPBuffer;
#else
	register int *pS = m_pBuffer;
#endif
	register int *pE = pS + m_nTickSize * m_nSampleChannel;
	NativeSample *pD = dest16;
	if(m_swapLR)
	{
		while(pS < pE)
		{
			v = *pS++ >> MIX_SHIFT;
			if(v > m_maxValueBeforeMix)
				v = m_maxValueBeforeMix;
			else if(v < -m_maxValueBeforeMix)
				v = -m_maxValueBeforeMix;
			pD[1] = FMUL16(v, m_fVolume);
			v = *pS++ >> MIX_SHIFT;
			if(v > m_maxValueBeforeMix)
				v = m_maxValueBeforeMix;
			else if(v < -m_maxValueBeforeMix)
				v = -m_maxValueBeforeMix;
			*pD = FMUL16(v, m_fVolume);
			pD += 2;
		}
	}
	else while(pS < pE)
	{
		v = *pS++ >> MIX_SHIFT;
		if(v > m_maxValueBeforeMix)
			v = m_maxValueBeforeMix;
		else if(v < -m_maxValueBeforeMix)
			v = -m_maxValueBeforeMix;
		*pD++ = FMUL16(v, m_fVolume);
	}
}

void CPsMixer::ProcessReverb(NativeSample *pBuf)
{
//	m_reverb.Process(pBuf, m_nTickSize);
}

#ifdef USE_STEREO_DELAY
void CPsMixer::ProcessStereoDelay(NativeSample *pBuf)
{
	CPsSys::memcpy(m_pMPBuffer, m_pMPBuffer + m_nTickSize * m_nSampleChannel, m_mpDelayGap * sizeof(m_pMPBuffer[0]) * m_nSampleChannel);
	CPsSys::memcpy(m_pHPBuffer, m_pHPBuffer + m_nTickSize * m_nSampleChannel, m_hpDelayGap * sizeof(m_pHPBuffer[0]) * m_nSampleChannel);
	CPsSys::memcpy(m_pMPBuffer + m_mpDelayGap * m_nSampleChannel, m_pBuffer, m_nTickSize * m_nSampleChannel * sizeof(m_pBuffer[0]));
	m_LPL.Process(m_pBuffer, m_pLPBuffer, m_nTickSize, 2);
	m_LPR.Process(m_pBuffer + 1, m_pLPBuffer + 1, m_nTickSize, 2);
	int *pH = m_pHPBuffer + m_hpDelayGap * m_nSampleChannel;
	m_HPL.Process(m_pBuffer, pH, m_nTickSize, 2);
	m_HPR.Process(m_pBuffer + 1, pH + 1, m_nTickSize, 2);
	int n = m_nTickSize * m_nSampleChannel;
	int i;
	for(i = 0; i < n; i++)
	{
		m_pLPBuffer[i] = m_pLPBuffer[i];// + m_pMPBuffer[i] + m_pHPBuffer[i];
	}
}
#endif
