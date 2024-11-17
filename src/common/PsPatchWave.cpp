#include "psmath.h"
#include "PsPatchWave.h"
#include "wavpack/wavpack.h"
#include "PsSys.h"

#define EXTRA_INTERP	2

CPsPatchWave::CPsPatchWave()
{
	m_nSamples = 0;
	m_pData = NULL;
}

CPsPatchWave::~CPsPatchWave()
{
	Free();
}

bool CPsPatchWave::Load(CPsMath *pMath, CPsReader *pReader, int link)
{
	Free();
	m_link = link;

//	printf("link: %d\n", link);
	int encode = pReader->ReadShort();
	if(encode == 1)
	{
		m_bitPerSample = pReader->ReadChar();
		m_nChannel = pReader->ReadChar();
		m_nSampleRate = pReader->ReadInt();
		unsigned int n = pReader->ReadInt();
		unsigned int tn = EXTRA_INTERP * m_nChannel * m_bitPerSample / 8;
		PsAssert(n >= tn);
		m_pData = new unsigned char[n + tn];
		if(!m_pData)
			return false;
		if(pReader->ReadBlock(m_pData, n) != n)
		{
			Free();
			return false;
		}
		m_nSamples = n / (m_bitPerSample * m_nChannel / 8);
	}
	else if(encode == 0x1234)
	{
		m_bitPerSample = pReader->ReadChar();
		m_nChannel = pReader->ReadChar();
		m_nSampleRate = pReader->ReadInt();
		unsigned int npacked = pReader->ReadInt();
		unsigned char *ptmp = new unsigned char[npacked];
		if(!ptmp)
			return false;
		if(pReader->ReadBlock(ptmp, npacked) != npacked)
		{
			Free();
			return false;
		}
		unsigned int n = get_unpacked_size(ptmp);
		unsigned int tn = EXTRA_INTERP * m_nChannel * m_bitPerSample / 8;
		PsAssert(n >= tn);
		m_pData = new unsigned char[n + tn];
		if(!m_pData)
		{
			delete ptmp;
			return false;
		}
		wavunpack16(m_pData, ptmp, npacked);
		delete ptmp;

		m_nSamples = n / (m_bitPerSample * m_nChannel / 8);
	}
	else
		PsAssert(0);

	m_fPitch = pMath->AbsFreq2FixedPitch(I2F(m_nSampleRate));
	return true;
}

void CPsPatchWave::Free()
{
	delete m_pData;
	m_pData = NULL;
	m_nSamples = 0;
}

void CPsPatchWave::UpdateWaveForInterpolation(int loopStart, int loopLength)
{
	int tn = EXTRA_INTERP * m_nChannel * m_bitPerSample / 8;
	if(loopStart < 0)
	{
		unsigned char *p = m_pData + m_nSamples * (m_nChannel * m_bitPerSample / 8);
		CPsSys::memset(p, 0, tn);
	}
	else
	{
		unsigned char *p = m_pData + (loopStart + loopLength) * (m_nChannel * m_bitPerSample / 8);
		CPsSys::memcpy(p, m_pData + loopStart * (m_nChannel * m_bitPerSample / 8), tn);
	}
}

bool CPsPatchWave::ExpandTo16Bit()
{
	if(m_bitPerSample == 16)
		return true;

	PsAssert(m_bitPerSample == 8);
	if(m_bitPerSample != 8)
		return false;
	short *p = new short[EXTRA_INTERP + m_nSamples];
	if(!p)
		return false;
	int i, n = EXTRA_INTERP + m_nSamples;
	for(i = 0; i < n; i++)
	{
		p[i] = (m_pData[i] - 128) << 8;
	}
	delete m_pData;
	m_pData = (unsigned char*)p;
	m_bitPerSample = 16;
	return true;
}
