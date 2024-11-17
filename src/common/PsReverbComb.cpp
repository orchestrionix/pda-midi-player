#include "PsAudioSystem.h"
#include "PsReverbComb.h"
#include "Win32/PsSys.h"

#define REVERB_BUFFER_SIZE 16384
#define REVERB_BUFFER_MASK 32767

CPsReverbComb::CPsReverbComb()
{
	m_pSystem = NULL;
}

CPsReverbComb::~CPsReverbComb()
{
	Shutdown();
}

bool CPsReverbComb::Init(CPsAudioSystem *pSystem)
{
	m_pSystem = pSystem;

	LPfilterL = 0;
	LPfilterR = 0;
	LPfilterLz = 0;
	LPfilterRz = 0;
	m_reverbPtr = 0;
	m_nReverbPos = 0;

	m_nReverbBufferSize = REVERB_BUFFER_SIZE;
	m_pReverb = new int[m_nReverbBufferSize * 2];
	if(!m_pReverb)
		return false;
	CPsSys::memset(m_pReverb, 0, m_nReverbBufferSize * 2 * sizeof(m_pReverb[0]));
	m_fLevel = F_ONE;
	return true;
}

void CPsReverbComb::Shutdown()
{
	delete m_pReverb;
	m_pReverb = NULL;
}

void CPsReverbComb::Process(int *pBuf, int nSampleFrames)
{
	register int b, c, bz, cz;
	register int *sourceLR;
	register int *reverbBuf;
	register int reverbPtr1, reverbPtr2, reverbPtr3, reverbPtr4;
	register int a = 4 * m_pSystem->GetConfig()->GetMixFrequency() / 22050;

	if(a == 0)
	{
		return;
	}
	PsAssert(a * 1711 + 1 < REVERB_BUFFER_SIZE * 2);
	reverbBuf = m_pReverb;
	if (reverbBuf)
	{
		sourceLR = pBuf;
		
		b = LPfilterL;
		c = LPfilterR;
		bz = LPfilterLz;
		cz = LPfilterRz;
		reverbPtr1 = m_reverbPtr;
		
		reverbPtr2 = (reverbPtr1 - 1100*a) & REVERB_BUFFER_MASK; 
		reverbPtr3 = (reverbPtr1 - 1473*a) & REVERB_BUFFER_MASK;
		reverbPtr4 = (reverbPtr1 - 1711*a) & REVERB_BUFFER_MASK;
		
		for (a = nSampleFrames; a > 0; --a)
		{
			b -= (bz + b) >> 2;
			bz = b;
			b += (reverbBuf[reverbPtr2] + reverbBuf[reverbPtr3] + reverbBuf[reverbPtr4]) >> 3;
			reverbBuf[reverbPtr1] = *sourceLR + b - (b >> 2);
			*sourceLR = *sourceLR +  ((c + b) >> 1);
			sourceLR++;
			
			c -= (cz + c) >> 2;
			cz = c;
			c += (reverbBuf[reverbPtr2+1] + reverbBuf[reverbPtr3+1] + reverbBuf[reverbPtr4+1]) >> 3;
			reverbBuf[reverbPtr1+1] = *sourceLR + c - (c >> 2);
			*sourceLR = *sourceLR + ((c + b) >> 1);
			sourceLR++;
			
			reverbPtr1 = (reverbPtr1 + 2) & REVERB_BUFFER_MASK;
			reverbPtr2 = (reverbPtr2 + 2) & REVERB_BUFFER_MASK;
			reverbPtr3 = (reverbPtr3 + 2) & REVERB_BUFFER_MASK;
			reverbPtr4 = (reverbPtr4 + 2) & REVERB_BUFFER_MASK;
		}
		LPfilterL = b;
		LPfilterLz = bz;
		LPfilterR = c;
		LPfilterRz = cz;
		m_reverbPtr = reverbPtr1;
	}
}
