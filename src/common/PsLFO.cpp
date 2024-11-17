#include "PsLFO.h"

CPsLFO::CPsLFO()
{
	m_pMath = NULL;
	m_pLFO = NULL;
	Reset();
}

CPsLFO::~CPsLFO()
{

}

void CPsLFO::SetParameters(CPsPatch::PSLFO *pLFO, CPsMath *pMath, F32 fUpdateFreq)
{
	m_pMath = pMath;
	PsAssert(fUpdateFreq > 0);
	m_pLFO = pLFO;
	F32 freq = pMath->DlsPitch2AbsFreq(pLFO->pitch);
	m_fStep = (F32)((F64)PSFLOAT2F(PI * 2) * freq / fUpdateFreq);
	m_fInitPos = (F32)FMUL32(m_pMath->DlsTime2FixedSec(m_pLFO->startDelay), FMUL32(PSFLOAT2F(PI * 2), freq));
}

void CPsLFO::TriggerOn()
{
	UpdateModWheel();
	m_fCurPos = -m_fInitPos;
	m_fCurValue = 0;
}

void CPsLFO::Update()
{
	if(m_fCurPos >= 0)
		m_fCurValue = m_pMath->Sin(m_fCurPos);
	m_fCurPos += m_fStep;
	while(m_fCurPos >= PSFLOAT2F(PI * 2))
		m_fCurPos -= PSFLOAT2F(PI * 2);
}

void CPsLFO::UpdateModWheel()
{
	if(m_pLFO)
	{
		m_fVolumeScale = CPsMath::DlsGain2FixedAttn(m_pLFO->volumeScale) + (F32)FMUL32(m_fModulation, CPsMath::DlsGain2FixedAttn(m_pLFO->MWToAttenuation));
		m_fVolumeScale /= 2;
		m_fPitchScale = CPsMath::DlsPitch2FixedPitch(m_pLFO->pitchScale) + (F32)FMUL32(m_fModulation, CPsMath::DlsPitch2FixedPitch(m_pLFO->MWToPitch));
	}
}

F32 CPsLFO::GetVolumeScale()
{
	return (F32)FMUL32(m_fCurValue + F_ONE, m_fVolumeScale);
}

F32 CPsLFO::GetPitchScale()
{
	return (F32)FMUL32(m_fCurValue, m_fPitchScale);
}

void CPsLFO::Reset()
{
	m_fStep = m_fCurPos = 0;
	m_fCurValue = 0;
	m_fVolumeScale = 0;
	m_fPitchScale = 0;
	m_fModulation = 0;
	m_pLFO = NULL;
}
