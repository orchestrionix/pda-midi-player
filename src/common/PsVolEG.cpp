#include "PsAudioSystem.h"
#include "PsVolEG.h"

CPsVolEG::CPsVolEG()
{
	m_pMath = NULL;
	m_pConfig = NULL;
	Reset();
}

CPsVolEG::~CPsVolEG()
{

}

void CPsVolEG::TriggerOn(int key, F32 fVelocity)
{
	PsAssert(fVelocity >= 0 && fVelocity <= F_ONE);
	m_key = key;
	int tc = m_pEg->attack + (int)FMUL32(fVelocity, m_pEg->velocityToAttack);
	F32 t = (F32)FMUL32(m_pMath->DlsTime2FixedSec(tc), m_pConfig->GetUpdateFrequency());
	if(t > 0x4000) //if the attack time == 0, t == 0x4000
	{
		m_delta = FRECIPROCAL(t);
		if(m_delta <= 0)
			m_delta = 1;
		m_attackGain = m_delta;
		m_value = m_pMath->Volume2FixedAttn(m_attackGain);
		m_state = ATTACK;
	}
	else
	{
		m_value = 0;
		CalcDecayDelta();
		m_state = DECAY;
	}
}

void CPsVolEG::SetParameters(CPsMath *pMath, CPsAudioConfig *pConfig, CPsPatch::PSENVELOPE *pEG)
{
	m_pMath = pMath;
	m_pConfig = pConfig;
	m_pEg = pEG;
	if(pEG->sustain)
		m_fSustain = m_pMath->Volume2FixedAttn(m_pMath->DlsUnits2Fixed(pEG->sustain));
	else
		m_fSustain = I2F(960);
}

F32 CPsVolEG::Update()
{
	switch(m_state) {
	case ATTACK:
		m_attackGain += m_delta;
		if(m_attackGain < F_ONE)
			m_value = m_pMath->Volume2FixedAttn(m_attackGain);
		else
		{
			m_value = 0;
			CalcDecayDelta();
			m_state = DECAY;
		}
		break;
	case DECAY:
		m_value += m_delta;
		if(m_value >= m_fSustain)
		{
			m_value = m_fSustain;
			m_state = SUSTAIN;
		}
		break;
	case SUSTAIN:
		if(m_fSustain >= I2F(960))
		{
			m_value = I2F(960);
			m_state = OFF;
		}
		break;
	case RELEASE:
		m_value += m_delta;
		if(m_value > I2F(960))
		{
			m_value = I2F(960);
			m_state = OFF;
		}
		break;
	}
	return m_value;
}

void CPsVolEG::CalcDecayDelta()
{
	PsAssert(m_key >= 0 && m_key <= 127);
	int tc = m_pEg->decay + m_key * (m_pEg->keyToDecay / 127);
	F32 t = (F32)FMUL32(m_pMath->DlsTime2FixedSec(tc), m_pConfig->GetUpdateFrequency());
	if(t > 0)
		m_delta = FDIV32(I2F(960), t);
	else
		m_delta = I2F(960);
}

void CPsVolEG::CalcReleaseDelta()
{
	F32 t = m_pMath->DlsTime2FixedSec(m_pEg->release);
	if(t < F_ONE / 32)
		t = F_ONE / 32;
	t = (F32)FMUL32(t, m_pConfig->GetUpdateFrequency());
	if(t > 0)
		m_delta = FDIV32(I2F(960), t);
	else
		m_delta = I2F(960);
}

void CPsVolEG::TriggerOff(int key, F32 fVelocity)
{
	if(m_state != RELEASE && m_state != OFF)
	{
		PsAssert(fVelocity >= 0 && fVelocity <= F_ONE);
		m_key = key;
		CalcReleaseDelta();
		m_state = RELEASE;
	}
}

void CPsVolEG::Reset()
{
	m_state = OFF;
	m_value = 0;
	m_pEg = NULL;
}

void CPsVolEG::CutOff()
{
	F32 t = (F32)FMUL32(F_ONE / 32, m_pConfig->GetUpdateFrequency());
	if(t > 0)
		m_delta = FDIV32(I2F(960), t);
	m_state = RELEASE;
}
