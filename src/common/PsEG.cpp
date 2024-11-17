#include"PsAudioSystem.h"
#include "PsEG.h"

CPsEG::CPsEG()
{
	m_pMath = NULL;
	m_pConfig = NULL;
	Reset();
}

CPsEG::~CPsEG()
{

}

void CPsEG::TriggerOn(int key, F32 fVelocity)
{
	PsAssert(fVelocity >= 0 && fVelocity <= F_ONE);
	m_key = key;
	int tc = m_pEg->attack + (int)FMUL32(fVelocity, m_pEg->velocityToAttack);
	F32 t = (F32)FMUL32(m_pMath->DlsTime2FixedSec(tc), m_pConfig->GetUpdateFrequency());
	if(t > 0)
		m_delta = FRECIPROCAL(t);
	else
		m_delta = F_ONE;

	m_value = 0;
	m_state = ATTACK;
}

void CPsEG::SetParameters(CPsMath *pMath, CPsAudioConfig *pConfig, CPsPatch::PSENVELOPE *pEG)
{
	m_pMath = pMath;
	m_pConfig = pConfig;
	m_pEg = pEG;
	m_fSustain = m_pMath->DlsUnits2Fixed(pEG->sustain);
}

F32 CPsEG::Update()
{
	switch(m_state) {
	case ATTACK:
		m_value += m_delta;
		if(m_value >= F_ONE)
		{
			m_value = F_ONE;
			CalcDecayDelta();
			m_state = DECAY;
		}
		break;
	case DECAY:
		m_value -= m_delta;
		if(m_value <= m_fSustain)
		{
			m_value = m_fSustain;
			m_state = SUSTAIN;
		}
		break;
	case SUSTAIN:
		if(m_fSustain <= 0)
		{
			m_value = 0;
			m_state = OFF;
		}
		break;
	case RELEASE:
		m_value -= m_delta;
		if(m_value <= 0)
		{
			m_value = 0;
			m_state = OFF;
		}
		break;
	}
	return m_value;
}

void CPsEG::CalcDecayDelta()
{
	PsAssert(m_key >= 0 && m_key <= 127);
	int tc = m_pEg->decay + m_key * (m_pEg->keyToDecay / 127);
	F32 t = (F32)FMUL32(m_pMath->DlsTime2FixedSec(tc), m_pConfig->GetUpdateFrequency());
	if(t > 0)
		m_delta = FRECIPROCAL(t);
	else
		m_delta = F_ONE;
}

void CPsEG::CalcReleaseDelta()
{
	F32 tc = m_pMath->DlsTime2FixedSec(m_pEg->release);
	F32 t = (F32)FMUL32(tc, m_pConfig->GetUpdateFrequency());
	if(t > 0)
		m_delta = FRECIPROCAL(t);
	else
		m_delta = F_ONE;
}

void CPsEG::TriggerOff(int key, F32 fVelocity)
{
	if(m_state != RELEASE && m_state != OFF)
	{
		PsAssert(fVelocity >= 0 && fVelocity <= F_ONE);
		m_key = key;
		CalcReleaseDelta();
		m_state = RELEASE;
	}
}

void CPsEG::Reset()
{
	m_state = OFF;
	m_value = 0;
	m_pEg = NULL;
}
