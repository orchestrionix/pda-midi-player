#include "PsAudioSystem.h"
#include "PsOscillator.h"

CPsOscillator::CPsOscillator()
{
}

CPsOscillator::~CPsOscillator()
{

}

void CPsOscillator::SetArticulation(CPsPatch::PSARTICULATION *pArt)
{
	if(m_pArt != pArt)
	{
		m_pArt = pArt;
		m_fEG2Pitch = CPsMath::DlsPitch2FixedPitch(m_pArt->EG2ToPitch);
		F32 res = m_pConfig->GetUpdateFrequency();
		if(m_pArt->Lfo.pitchScale || m_pArt->Lfo.volumeScale || m_pArt->Lfo.MWToAttenuation || m_pArt->Lfo.MWToPitch)
		{
			m_flag |= F_ENABLE_LFO;
			m_lfo.SetParameters(&m_pArt->Lfo, m_pMath, res);
		}
		else
			m_flag &= ~F_ENABLE_LFO;
		m_volEG.SetParameters(m_pMath, m_pConfig, m_pArt->Env);
		if(m_fEG2Pitch)
			m_pitchEG.SetParameters(m_pMath, m_pConfig, m_pArt->Env + 1);
	}
	if(m_pPatch->IsDrum())
	{
		m_fPan = (F16)(CPsMath::DlsUnits2Fixed(m_pArt->initialPan));
	}
}

void CPsOscillator::Open(CPsAudioSystem *pSystem)
{
	m_pMath = pSystem->GetMath();
	m_pConfig = pSystem->GetConfig();
	m_pPatch = NULL;
	m_pArt = NULL;
	m_pWaveInfo = NULL;
	m_pWaveData = NULL;
	m_pPar = NULL;
	m_notePitch = 0;
	m_state = STATE_STOPPED;
	m_fPan = 0;
	m_flag = F_NOTE_OFF;
	m_key = -1;
	m_fMaxVolume = -1;
	m_lfo.Reset();
	m_volEG.Reset();
	m_pitchEG.Reset();
}

void CPsOscillator::SetWave(CPsPatch::PSWAVEINFO *pInfo, CPsPatchWave *pWave)
{
	if(m_pWaveData != pWave || m_pWaveInfo != pInfo)
	{
		m_pWaveData = pWave;
		PsAssert(m_pWaveData->GetChannelNumber() == 1);
		PsAssert(m_pWaveData->GetBitPerSample() == sizeof(NativeSample)*8);
//		PsAssert(m_pWaveData->GetSampleCount() < (1 << (31 - F_SHIFT)));
		m_fWaveSamplePitch = m_pWaveData->GetSamplePitch();

		m_pWaveInfo = pInfo;
		m_wavePitch = m_pMath->DlsPitch2FixedPitch(m_pWaveInfo->pitch);
		m_sampleData.nBitPerSample = pWave->GetBitPerSample();
		m_sampleData.nChannel = pWave->GetChannelNumber();
		m_sampleData.pData = pWave->GetData();
		m_sampleData.loopType = m_pWaveInfo->loopType;
		if(m_pWaveInfo->loopType == CPsPatch::SAMPLE_LOOPFORWARD)
		{
			m_sampleData.loopStart = I2F64(m_pWaveInfo->loopStart);
			m_sampleData.loopLength = I2F64(m_pWaveInfo->loopLength);
		}
		else
		{
			m_sampleData.loopStart = 0;
			m_sampleData.loopLength = I2F64(m_pWaveData->GetSampleCount());
		}
	}
}

void CPsOscillator::Update()
{
	F32 gain = m_volEG.Update();
	F32 pitch = m_notePitch;

	if(m_flag & F_ENABLE_LFO)
	{
		m_lfo.Update();
		pitch += m_lfo.GetPitchScale();
		gain += m_lfo.GetVolumeScale();
	}

	if(m_fEG2Pitch)
		pitch += (F32)FMUL32(m_pitchEG.Update(), m_fEG2Pitch);

	pitch += m_pPar->m_fMasterTuning + m_pPar->m_fPitchBand;
	pitch += m_fWaveSamplePitch - m_pConfig->GetMixPitch();
	pitch -= m_wavePitch;
	m_fRelativeFreq = m_pMath->Pitch2RelativFreq(pitch);

	if(m_pPar->m_isMute)
	{
		m_fMixVolume = 0;
	}
	else
	{
		gain += CPsMath::DlsGain2FixedAttn(m_pWaveInfo->attenuation);
		gain += m_pPar->m_fAttenuation;
		m_fMixVolume = (F32)FMUL32(m_pMath->FixedAttn2Volume(gain), m_keyOnVel);
		//	printf("%d\n", m_fMixVolume);

		//reduce volume to prevent the mixer from overflow
		//m_fMixVolume >>= 1;

		if(m_fMixVolume > F_ONE)
			m_fMixVolume = F_ONE;
	}

	//	printf("state: %d %d\n", GetState(), m_volEG.GetState());
	if(m_volEG.GetState() == CPsVolEG::OFF)
	{
		SetState(STATE_STOPPED);
	}
	else if(m_volEG.GetState() == CPsVolEG::RELEASE)
	{
		PsAssert(m_fMixVolume <= F_ONE);
		if(m_fMixPosition >= m_sampleData.loopStart)
		{
			m_fMaxVolume = FMUL16(m_fMixVolume, m_pWaveInfo->maxLoopAmplitude);
		}
	}
}

void CPsOscillator::NoteOn(int key, F16 velocity)
{
	PsAssert(GetState() == STATE_STOPPED);
	m_keyOnVel = velocity;
	m_key = key;
	int rid = 0;
	CPsPatch::PSREGION *pRgn = m_pPatch->FindRegion(key, rid);
	SetArticulation(m_pPatch->GetArticulation(m_pPatch->IsDrum()?rid:0));
	SetWave(&pRgn->waveInfo, (CPsPatchWave*)pRgn->waveInfo.waveLink);
	int effectiveNote = key + m_pPar->m_keyTranspose;
	if(effectiveNote <= 0)
		effectiveNote = 1;
	else if(effectiveNote > 127)
		effectiveNote = 127;
	m_notePitch = m_pMath->Note2Pitch(effectiveNote);
	if(m_flag & F_ENABLE_LFO)
		m_lfo.TriggerOn();
	m_flag &= ~(F_SUSTAINING | F_NOTE_OFF);
	m_volEG.TriggerOn(key, velocity);
	if(m_fEG2Pitch)
		m_pitchEG.TriggerOn(key, velocity);
	m_fMixPosition = 0;
	m_fLastLVol = 0;
	m_fLastRVol = 0;
	m_fMaxVolume = F_ONE;
	SetState(STATE_PLAYING);
}

bool CPsOscillator::NoteOff(int key, F16 velocity, bool bForce)
{
	if(key == m_key)
	{
		m_flag |= F_NOTE_OFF;
		if(GetState() == STATE_PLAYING)
		{
			if(!bForce && (m_flag & F_HOLDPADEL))
			{
				m_flag |= F_SUSTAINING;
				return false;
			}
			else
			{
				m_volEG.TriggerOff(key, velocity);
				if(m_fEG2Pitch)
					m_pitchEG.TriggerOff(key, velocity);
				m_flag &= ~F_SUSTAINING;
			}
		}
		m_key = -1;
		return true;
	}
	return false;
}

F32 CPsOscillator::GetMixVolume()
{
	return m_fMixVolume;
}

F32 CPsOscillator::GetMixFrequency()
{
	return m_fRelativeFreq;
}

F32 CPsOscillator::GetPan()
{
	return m_fPan;
}

F32 CPsOscillator::GetBalance()
{
	return m_pPar->m_fBalance;
}

void CPsOscillator::SetHoldPedal(bool bHold)
{
	if(bHold)
	{
		m_flag |= F_HOLDPADEL;
	}
	else
	{
		m_flag &= ~F_HOLDPADEL;
		if(m_flag & F_SUSTAINING)
			NoteOff(m_key, F_ONE/2, true);
	}
}

int CPsOscillator::GetState()
{
	return m_state;
}

void CPsOscillator::SetState(int state)
{
	m_state = state;
	if(state == STATE_STOPPED)
		m_fMaxVolume = -1;
}

const CPsSampleData* CPsOscillator::GetSampleData()
{
	return &m_sampleData;
}
