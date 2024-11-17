#include "PsAudioSystem.h"
#include "PsInstrument.h"
#include "PsMixer.h"
#include "PsSys.h"

#ifdef _CONSOLE
#define TRACE_NOTE
#endif

bool CPsOscPool::Open(CPsAudioSystem *p, int nPoly)
{
	PsAssert(nPoly <= MAX_POLYPHONY);
	m_nPoly = nPoly;
	m_nActive = 0;
	for(int i = 0; i < MAX_POLYPHONY; i++)
	{
		m_channel[i] = -1;
		m_osc[i].Open(p);
	}
	return true;
}

void CPsOscPool::Close(CPsAudioSystem *p)
{
	Reset(p);
}

void CPsOscPool::Reset(CPsAudioSystem *p)
{
	CPsMixer *pMixer = p->GetMixer();
	for(int i = 0; i < MAX_POLYPHONY; i++)
	{
		pMixer->RemoveChannel(m_osc + i);
		m_osc[i].SetState(CPsOscillator::STATE_STOPPED);
		m_channel[i] = -1;
	}
}

void CPsOscPool::CheckConsistancy()
{
	int i;
	for(i = 0; i < MAX_POLYPHONY; i++)
	{
		if(m_channel[i] < 0)
		{
			if(m_osc[i].GetState() == CPsOscillator::STATE_PLAYING)
			{
				switch(m_osc[i].GetVolEGState()) {
				case CPsVolEG::RELEASE:
					break;
				default:
					PsAssert(0);
				}
			}
		}
	}
}

void CPsOscPool::GetChannelVolumes(F16* pVolArray)
{
	CPsSys::memset(pVolArray, 0, sizeof(pVolArray[0]) * 16);
	for(int i = 0; i < MAX_POLYPHONY; i++)
	{
		if(m_channel[i] >= 0 && m_osc[i].GetState() == CPsOscillator::STATE_PLAYING)
		{
			F16 vol = m_osc[i].m_fLastLVol;
			if(m_osc[i].m_fLastRVol > vol)
				vol = m_osc[i].m_fLastRVol;
			PsAssert(m_channel[i] < 16);
			if(pVolArray[m_channel[i]] < vol)
				pVolArray[m_channel[i]] = vol;
		}
	}
}

void CPsOscPool::ReduceVoice()
{
	int i;

	m_nActive = 0;
	
	for(i = 0; i < MAX_POLYPHONY; i++)
	{
		if(m_osc[i].GetState() != CPsOscillator::STATE_STOPPED)
			m_nActive++;
	}

	F32 fCutOffVol = 0;
	if(m_nActive > m_nPoly * 7 / 8)
	{
		fCutOffVol = F_ONE / 32;
	}
	else if(m_nActive > m_nPoly * 3 / 4)
	{
		fCutOffVol = F_ONE / 64;
	}
	else if(m_nActive > m_nPoly / 2)
	{
		fCutOffVol = F_ONE / 128;
	}
	else
		return;
	for(i = 0; i < MAX_POLYPHONY; i++)
	{
//		if(m_nActive > MAX_POLYPHONY * 3 / 4)
//			m_osc[i].SetHoldPedal(false);
		F32 vol = m_osc[i].GetMaxVolume();
		if(vol >= 0 && vol <= fCutOffVol)
			m_osc[i].CutOff();
	}
}

static unsigned char g_channelPriority[] = {
	15, 14, 13, 12, 11, 10, 8, 7, 6, 5, 4, 3, 2, 1, 0, 9
};

static unsigned char g_reservedVoiceNumber[] = {
	3, 3, 2, 2, 2, 2, 2, 2, 1, 4, 1, 1, 1, 1, 1, 1
};

int CPsOscPool::AllocateOscillator(int channel)
{
	int i;
	short nActive[16];

	for(i = 0; i < 16; i++)
		nActive[i] = 0;
	for(i = GetPolyNumber() - 1; i >= 0; i--)
	{
		if(GetOscillator(i)->GetState() == CPsOscillator::STATE_STOPPED)
		{
			return i;
		}
		else
		{
			++nActive[GetChannel(i)];
		}
	}

	//All oscillators are playing, find the one with lowest priority
	int chn = -1;
	for(i = 0; i < 16; i++)
	{
		int id = g_channelPriority[i];
		if(nActive[id] > g_reservedVoiceNumber[id])
		{
			chn = id;
			break;
		}
	}
	if(chn < 0)
		return -1;
	for(i = GetPolyNumber() - 1; i >= 0; i--)
	{
		if(GetChannel(i) == chn)
		{
			CPsOscillator *pOsc = GetOscillator(i);
			pOsc->SetState(CPsOscillator::STATE_STOPPED);
			return i;
		}
	}
	return -1;
}

CPsInstrument::CPsInstrument()
{
	m_pSystem = NULL;
}

CPsInstrument::~CPsInstrument()
{

}

void CPsInstrument::SetPatch(CPsPatch *p)
{
	m_pPatch = p;
}

bool CPsInstrument::Open(CPsAudioSystem *p, CPsOscPool *pOscPool, int channel)
{
	m_pSystem = p;
	m_pOscPool = pOscPool;
	m_channel = channel;
	m_pPatch = NULL;

	m_par.m_fBalance = F_ONE;
	m_par.m_fReverb = 0;
	m_par.m_fChorus = 0;
	m_par.m_fMasterTuning = 0;
	m_par.m_fAttenuation = 0;
	m_par.m_fModulation = 0;
	m_par.m_fPan = 0;
	m_par.m_fPitchBand = 0;
	m_par.m_keyTranspose = 0;
	m_par.m_isMute = 0;

	m_fPitchBandRange = I2F(200);
	m_fExpression = F_ONE;
	m_flag = 0;
	SetKeyTranspose(0);
	SetPan(0);
	SetBalance(0);
	SetVolume(F_ONE*100/127);
	return true;
}

void CPsInstrument::Close()
{
	m_pOscPool = NULL;
	m_pSystem = NULL;
	m_pPatch = NULL;
}

bool CPsInstrument::NoteOn(unsigned char note, F32 velocity)
{
	int i;
	CPsPatch::PSREGION *pRgn;
	CPsOscillator *pOsc;
	int rid;

	pRgn = m_pPatch->FindRegion(note, rid);
	if(!pRgn)
		return true;
	if(pRgn->options & CPsPatch::REGION_SELFNONEXCLUSIVE || pRgn->group > 0)
	{
		for(i = m_pOscPool->GetMaxPoly() - 1; i >= 0; i--)
		{
			if(m_pOscPool->GetChannel(i) == m_channel)
			{
				pOsc = m_pOscPool->GetOscillator(i);
				if(pOsc->GetState() == CPsOscillator::STATE_PLAYING)
				{
					int key = pOsc->GetCurrentKey();
					if(key == note)
					{
						if(pRgn->options & CPsPatch::REGION_SELFNONEXCLUSIVE)
						{
							m_pOscPool->SetChannel(i, -1);
							pOsc->NoteOff(key, F_ONE / 2, true);
						}
					}
					else if(pRgn->group > 0)
					{
						int t;
						CPsPatch::PSREGION *p = m_pPatch->FindRegion(key, t);
						if(p && p->group == pRgn->group)
						{
							m_pOscPool->SetChannel(i, -1);
							pOsc->NoteOff(key, F_ONE / 2, true);
						}
					}
				}
			}
		}
	}
	int relId = m_pOscPool->AllocateOscillator(m_channel);
	if(relId < 0)
	{
#ifdef _CONSOLE1
		printf("skiped:%d\n", note);
#endif
		return false;
	}
	pOsc = m_pOscPool->GetOscillator(relId);
	F32 f;
	f = I2F(pRgn->minVelocity) / 127;
	if(velocity < f)
		velocity = f;
	else
	{
		f = I2F(pRgn->maxVelocity) / 127;
		if(velocity > f)
			velocity = f;
	}
#ifdef TRACE_NOTE
	const char *notename[] = {
		"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
	};
	printf("note on: %d, %d, %d, %d, %s%d, %d\n", m_pPatch->GetProgram(), m_pPatch->IsDrum(), note, velocity, notename[note%12], note / 12, m_pSystem->GetMixer()->GetMixChannels());
#endif
	pOsc->SetPatch(m_pPatch);

	pOsc->SetParameters(&m_par);

	pOsc->SetHoldPedal((m_flag & F_HOLDPEDAL) != 0);

	pOsc->NoteOn(note, velocity);
	m_pSystem->GetMixer()->AddChannel(pOsc);
	m_pOscPool->SetChannel(relId, m_channel);
	return true;
}

void CPsInstrument::NoteOff(unsigned char note, F32 velocity, bool bForce)
{
	int i;
	CPsOscillator *pOsc;
#ifdef TRACE_NOTE
	printf("note off: %d, %d, %d, %d\n", m_pPatch->GetProgram(), m_pPatch->IsDrum(), note, bForce);
#endif
	for(i = m_pOscPool->GetMaxPoly() - 1; i >= 0; i--)
	{
		if(m_pOscPool->GetChannel(i) == m_channel)
		{
			pOsc = m_pOscPool->GetOscillator(i);
			if(pOsc->GetState() == CPsOscillator::STATE_PLAYING)
			{
				if(pOsc->GetCurrentKey() == note && !pOsc->IsNoteOff())
				{
					pOsc->NoteOff(note, velocity, bForce);
					break;
				}
			}
		}
	}
}

void CPsInstrument::SetVolume(F32 vol){
	m_fVolume = vol;
	m_par.m_fAttenuation = 2 * m_pSystem->GetMath()->Volume2FixedAttn(FMUL16(m_fVolume, m_fExpression));
}

void CPsInstrument::SetExpression(F32 f){
	m_fExpression = f;
	m_par.m_fAttenuation = 2 * m_pSystem->GetMath()->Volume2FixedAttn(FMUL16(m_fVolume, m_fExpression));
}

void CPsInstrument::SetPan(F32 f)
{
	m_par.m_fPan = f;
/*	int i;
	for(i = m_pOscPool->GetMaxPoly() - 1; i >= 0; i--)
	{
		if(m_pOscPool->GetChannel(i) == m_channel)
			m_pOscPool->GetOscillator(i)->SetPan(m_fPan);
	}
	*/
}

void CPsInstrument::AllNoteOff()
{
	int i;
	CPsOscillator *pOsc;
	for(i = m_pOscPool->GetMaxPoly() - 1; i >= 0; i--)
	{
		if(m_pOscPool->GetChannel(i) == m_channel)
		{
			pOsc = m_pOscPool->GetOscillator(i);
			if(pOsc->GetState() == CPsOscillator::STATE_PLAYING)
			{
				pOsc->NoteOff(pOsc->GetCurrentKey(), F_ONE / 2, true);
			}
		}
	}
}

void CPsInstrument::SetPitchBand(F32 f)
{
	m_fPitchBand = f;
	m_par.m_fPitchBand = (F32)FMUL32(m_fPitchBand * 2 - F_ONE, m_fPitchBandRange);
}

void CPsInstrument::SetHoldPedal(bool bHold)
{
	if(bHold)
		m_flag |= F_HOLDPEDAL;
	else
		m_flag &= ~F_HOLDPEDAL;

	int i;
	CPsOscillator *pOsc;
	for(i = m_pOscPool->GetMaxPoly() - 1; i >= 0; i--)
	{
		if(m_pOscPool->GetChannel(i) == m_channel)
		{
			pOsc = m_pOscPool->GetOscillator(i);
			if(pOsc->GetState() == CPsOscillator::STATE_PLAYING)
			{
				pOsc->SetHoldPedal(bHold);
			}
		}
	}
}

void CPsInstrument::SetModulation(F32 f)
{
	m_par.m_fModulation = f;

	int i;
	CPsOscillator *pOsc;
	for(i = m_pOscPool->GetMaxPoly() - 1; i >= 0; i--)
	{
		if(m_pOscPool->GetChannel(i) == m_channel)
		{
			pOsc = m_pOscPool->GetOscillator(i);
			if(pOsc->GetState() == CPsOscillator::STATE_PLAYING)
			{
				pOsc->UpdateModWheel();
			}
		}
	}
}
