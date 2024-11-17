#include "PsAudioSystem.h"
#include "PsReverbSchroeder.h"
#include "PsSys.h"

const unsigned short CPsReverbSchroeder::m_nCombLen[NUM_COMB] =
{
	1116, 1188, 1277, 1356//, 1422, 1491, 1557, 1617
};

const unsigned short CPsReverbSchroeder::m_nAllPassLen[NUM_ALLPASS] =
{
	556, 441//, 341, 225
};

CPsReverbSchroeder::CPsReverbSchroeder()
{
}

void CPsReverbSchroeder::Mute()
{
	CPsSys::memset(m_buffer, 0, sizeof(m_buffer));
}

CPsReverbSchroeder::~CPsReverbSchroeder()
{
	Shutdown();
}

#ifndef FAST_REVERB
void CPsReverbSchroeder::Update()
{
	m_fWet1 = FMUL16(m_fWet, (m_fWidth + F_ONE) / 2);
	m_fWet2 = FMUL16(m_fWet, (F_ONE - m_fWidth) / 2);
	m_fWet1 = F2F8(m_fWet1);
	m_fWet2 = F2F8(m_fWet2);
}

void CPsReverbSchroeder::SetWet(F32 fWet)
{
	m_fWet = fWet;
	Update();
}

void CPsReverbSchroeder::SetWidth(F32 fWidth)
{
	m_fWidth = fWidth;
	Update();
}

void CPsReverbSchroeder::SetRoomSize(F32 f)
{
	m_fRoomSize = F2F8(f);
}

void CPsReverbSchroeder::SetDamp(F32 f)
{
	m_fDamp = F2F8(f);
}
#endif

void CPsReverbSchroeder::SetSampleRate(int rate)
{
	int *p = m_buffer;
	int i;
	for(i = 0; i < NUM_COMB; i++)
	{
		m_comb[i][0].len = m_nCombLen[i] * rate / 44100;
		m_comb[i][0].pos = 0;
		m_comb[i][0].buf = p;
		p += m_comb[i][0].len;
		m_comb[i][1].len = (m_nCombLen[i] + STEREOSPREAD) * rate / 44100;
		m_comb[i][1].pos = 0;
		m_comb[i][1].buf = p;
		p += m_comb[i][1].len;
		PsAssert(p <= m_buffer + BUFFER_SIZE);
		m_combStored[i][0] = m_combStored[i][1] = 0;
	}
	for(i = 0; i < NUM_ALLPASS; i++)
	{
		m_allpass[i][0].len = m_nAllPassLen[i] * rate / 44100;
		m_allpass[i][0].pos = 0;
		m_allpass[i][0].buf = p;
		p += m_allpass[i][0].len;
		m_allpass[i][1].len = (m_nAllPassLen[i] + STEREOSPREAD) * rate / 44100;
		m_allpass[i][1].pos = 0;
		m_allpass[i][1].buf = p;
		p += m_allpass[i][1].len;
		PsAssert(p <= m_buffer + BUFFER_SIZE);
	}
}

bool CPsReverbSchroeder::Init(CPsAudioConfig *pConfig)
{
	m_fWidth = F_ONE;

	SetSampleRate(pConfig->GetMixFrequency());
#ifndef FAST_REVERB
	SetWet(PSFLOAT2F(1.0));
	SetRoomSize(PSFLOAT2F(0.93));
	SetDamp(PSFLOAT2F(0.125));
	SetWidth(PSFLOAT2F(0.75));
#endif
	// Buffer will be full of rubbish - so we MUST mute them
	Mute();

	return true;
}

void CPsReverbSchroeder::Shutdown()
{
}

void CPsReverbSchroeder::Process(int *sourceP, int *destP, int nSampleFrames)
{
	int i;
	int fDamp2 = F8_ONE - m_fDamp;

	for(i = 0; i < nSampleFrames; i++)
	{
#ifdef FULL_CHORUS_REVERB_MIX
		int inputL = sourceP[0] >> (MIX_SHIFT + 4);//FMUL16(pBuf[0], PSFLOAT2F(0.025));
		int inputR = sourceP[1] >> (MIX_SHIFT + 4);//FMUL16(pBuf[1], PSFLOAT2F(0.025));
#else
		int inputL = sourceP[0];//FMUL16(pBuf[0], PSFLOAT2F(0.025));
		int inputR = sourceP[1];//FMUL16(pBuf[1], PSFLOAT2F(0.025));
#endif
		int j;
		CIRCLEBUFFER *pcb = m_comb[0];
		int outL, outR;
#ifdef FAST_REVERB
		outL = outR = 0;
		for(j = 0; j < NUM_COMB; j++)
		{
			int output;
			
			output = pcb->buf[pcb->pos];
			outL += output;
			if(output >= 0)
				output = output * 216 >> F8_SHIFT;
			else
				output = -(-output * 216 >> F8_SHIFT);
			pcb->buf[pcb->pos] = inputL + output;
			if(++pcb->pos >= pcb->len)
				pcb->pos = 0;
			++pcb;

			output = pcb->buf[pcb->pos];
			outR += output;
			if(output >= 0)
				output = output * 216 >> F8_SHIFT;
			else
				output = -(-output * 216 >> F8_SHIFT);
			pcb->buf[pcb->pos] = inputR + output;
			if(++pcb->pos >= pcb->len)
				pcb->pos = 0;
			++pcb;
		}
#else
		outL = outR = 0;
		int *pStored = m_combStored[0];
		// Accumulate comb filters in parallel
		for(j = 0; j < NUM_COMB; j++)
		{
			int output;
			
			output = pcb->buf[pcb->pos];
			outL += output;
			*pStored = (output * fDamp2 + *pStored * m_fDamp) >> F8_SHIFT;
			pcb->buf[pcb->pos] = inputL + (*pStored * m_fRoomSize >> F8_SHIFT);
			++pStored;
			if(++pcb->pos >= pcb->len)
				pcb->pos = 0;
			++pcb;

			output = pcb->buf[pcb->pos];
			outR += output;
			*pStored = (output * fDamp2 + *pStored * m_fDamp) >> F8_SHIFT;
			pcb->buf[pcb->pos] = inputR + (*pStored * m_fRoomSize >> F8_SHIFT);
			++pStored;
			if(++pcb->pos >= pcb->len)
				pcb->pos = 0;
			++pcb;
		}
#endif
		pcb = m_allpass[0];
		for(j = 0; j < NUM_ALLPASS; j++)
		{
			int output;

			output = pcb->buf[pcb->pos];
			pcb->buf[pcb->pos] = outL + (output >> 1);
			outL = output - outL;
			if(++pcb->pos >= pcb->len)
				pcb->pos = 0;
			++pcb;

			output = pcb->buf[pcb->pos];
			pcb->buf[pcb->pos] = outR + (output >> 1);
			outR = output - outR;
			if(++pcb->pos >= pcb->len)
				pcb->pos = 0;
			++pcb;
		}

#ifdef FAST_REVERB
		destP[0] += (outL * 2 + outR) << (MIX_SHIFT - 1);
		destP[1] += (outR * 2 + outL) << (MIX_SHIFT - 1);
#else
		F32 f = (outL * m_fWet1 + outR * m_fWet2) >> F8_SHIFT;
		destP[0] += f << (MIX_SHIFT);
		f = (outR * m_fWet1 + outL * m_fWet2) >> F8_SHIFT;
		destP[1] += f << (MIX_SHIFT);
#endif
		sourceP += 2;
		destP += 2;
	}
}
