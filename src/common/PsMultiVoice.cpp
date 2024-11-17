#include "PsAudioSystem.h"
#include "PsChorus.h"
#include "Win32/PsSys.h"

CPsChorus::CPsChorus()
{
	m_pBuf = NULL;
}

CPsChorus::~CPsChorus()
{
	Shutdown();
}

bool CPsChorus::Init(CPsAudioConfig *pConfig)
{
	m_nGapFrames = pConfig->GetMixFrequency() * VOICE_DELAY / 1000;
	m_nDelayFrames = m_nGapFrames * VOICE_NUMBER;
	m_nBufferFrames = pConfig->GetFramesPerUpdate() + m_nDelayFrames;
	m_pBuf = new int[m_nBufferFrames * 2];
	if(!m_pBuf)
		return false;
	CPsSys::memset(m_pBuf, 0, m_nBufferFrames * sizeof(int) * 2);
	return true;
}

void CPsChorus::Shutdown()
{
	delete m_pBuf;
	m_pBuf = NULL;
}

void CPsChorus::Process(int *pBuf, int nSampleFrames)
{
	PsAssert(m_nBufferFrames - m_nDelayFrames == nSampleFrames);
	CPsSys::memmove(m_pBuf, m_pBuf + nSampleFrames * 2, m_nDelayFrames * sizeof(int) * 2);
	CPsSys::memcpy(m_pBuf + m_nDelayFrames * 2, pBuf, nSampleFrames * sizeof(int) * 2);

	int *pS = m_pBuf + m_nDelayFrames * 2 - 2;
	int *pE = pBuf;
	int *pD = pBuf + nSampleFrames * 2 - 2;

	while(pD >= pE)
	{
		int i;
		for(i = 0; i < VOICE_NUMBER; i++)
		{
			pD[0] += pS[-i * m_nGapFrames * 2];
			pD[1] += pS[-i * m_nGapFrames * 2 + 1];
		}
		pD -= 2;
		pS -= 2;
	}
//	CPsSys::memcpy(pBuf, m_pBuf, nSampleFrames * sizeof(int) * 2);

}
