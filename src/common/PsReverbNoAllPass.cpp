#include "PsAudioSystem.h"
#include "PsReverbNoAllPass.h"
#include "Win32/PsSys.h"

/* Hard coded default delays in miliseconds */
static unsigned char DelayMsec[2][CPsReverbNoAllPass::NUM_COMB]={{17,18,21,24},{17,18,21,24}};  

CPsReverbNoAllPass::CPsReverbNoAllPass()
{
	m_pSystem = NULL;
}

CPsReverbNoAllPass::~CPsReverbNoAllPass()
{
	Shutdown();
}

bool CPsReverbNoAllPass::Init(CPsAudioSystem *pSystem, int reverbType)
{
	m_pSystem = pSystem;
	m_reverbType = reverbType;
	m_delayPos = 0;
	int i;
	for(i = 0; i < GET_ELEMENT_COUNT(m_nDelay); i++)
	{
		m_nDelay[i] = pSystem->GetConfig()->GetMixFrequency() * (DelayMsec[m_reverbType][i]) / 1000;
		PsAssert(m_nDelay[i] < MAX_DELAY_SAMPLES);
	}
	CPsSys::memset(m_delayBuf, 0, sizeof(m_delayBuf));
	SetLength(15);
	CalcFilter(1600, PSFLOAT2F(2));
	m_fLevel = F_ONE;
	return true;
}

void CPsReverbNoAllPass::Shutdown()
{
}

void CPsReverbNoAllPass::Process(NativeSample *pBuf, int nSampleFrames, int nStep)
{
	NativeSample i0 = m_in[0], i1 = m_in[1], o0 = m_out[0], o1 = m_out[1];
	for(int i = 0; i < nSampleFrames; i++)
	{
		register F16 fComb, fOut;
		register F16 input = *pBuf / 4;
		register int j;

		fComb = 0;
		for(j = 0; j < NUM_COMB; j++)
		{
			fOut = m_delayBuf[j][m_delayPos];
			fComb += fOut;
			m_delayBuf[j][(m_delayPos + m_nDelay[j]) & DELAY_MASK] = input + FMUL16(fOut, m_fGain[j]);
		}
		m_delayPos = (m_delayPos + 1) & DELAY_MASK;

		fOut = FMUL16(m_a[0], fComb) + FMUL16(m_a[1], i0) + FMUL16(m_a[2], i1) -
			FMUL16(m_b[0], o0) - FMUL16(m_b[1], o1);
		i1 = i0;
		i0 = fComb;
		o1 = o0;
		o0 = fOut;

		*pBuf = NATIVE_SAMPLE_SAT(FMUL16(fOut, m_fLevel) + *pBuf);

		pBuf += nStep;
	}
	m_in[0] = i0; m_in[1] = i1; m_out[0] = o0; m_out[1] = o1;
}

void CPsReverbNoAllPass::SetLength(int len)
{
	int i;
	for(i = 0; i < GET_ELEMENT_COUNT(m_fGain); i++)
		m_fGain[i] = m_pSystem->GetMath()->Pow2(-PSFLOAT2F(0.11253) * DelayMsec[m_reverbType][i] / (len + 1));
}

void CPsReverbNoAllPass::CalcFilter(int freq, F32 rez)
{
	CPsMath *pMath = m_pSystem->GetMath();
	F32 c = FRECIPROCAL(pMath->Tan(FMULDIV(PSFLOAT2F(PI), freq, m_pSystem->GetConfig()->GetMixFrequency())));
	m_a[0] = FRECIPROCAL(F_ONE + FMUL32(rez, c) + FMUL32(c, c));
	m_a[1] = 2 * m_a[0];
	m_a[2] = m_a[0];
	F32 b0 = 2 * (F_ONE - FMUL32(c, c));
	b0 = FMUL32(b0, m_a[0]);
	PsAssert((b0 >> 16) == 0 || (b0 >> 16) == -1);
	m_b[0] = b0;
	m_b[1] = FMUL32((F_ONE - FMUL32(rez, c) + FMUL32(c, c)), m_a[0]);
	m_in[0] = m_in[1] = 0;
	m_out[0] = m_out[1] = 0;
}
