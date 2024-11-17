#include "PsAudioSystem.h"
#include "PsOscillator.h"
#include "PsInstrument.h"
#include "PsMixer.h"
#include "PsSynthesizer.h"
#include "PsSys.h"
#include "PsSequence.h"

CPsSynthesizer::CPsSynthesizer()
{
	m_pSystem = NULL;
	m_pSbFile = NULL;
}

CPsSynthesizer::~CPsSynthesizer()
{
	Close();
}

bool CPsSynthesizer::Open(CPsAudioSystem *pSystem, CPsReader *pFile)
{
	if(!m_oscPool.Open(pSystem, CPsOscPool::MAX_POLYPHONY))
		return false;
	if(!m_sb.Load(pFile))
		return false;
	m_keyTranspose = 0;
	m_pSbFile = pFile;
	m_pSystem = pSystem;
	GMSystemOn(NULL, 0, NULL);
	return true;
}

void CPsSynthesizer::Close()
{
	if(m_pSystem)
	{
		int i;
		for(i = 0; i < TOTAL_INSTRUMENT; i++)
		{
			m_inst[i].Close();
		}
		m_oscPool.Close(m_pSystem);
		m_sb.Free();
		m_pSbFile = NULL;
		m_pSystem = NULL;
	}
}

//return false if no oscillator is available
bool CPsSynthesizer::NoteOn(int channel, int note, int velocity)
{
	if(velocity == 0)
	{
		NoteOff(channel, note, F_ONE/2);
		return true;
	}
	++m_noteState[channel][note];
//	if(note == 81)
//		return true;
	if(!m_pPatch[channel])
		LoadProgram(channel);
//	if(channel != 11)
//		return true;
	if(m_flags[channel] & F_ENABLED)
	{
		return m_inst[channel].NoteOn(note, I2F(velocity) / 127);
	}
	return true;
}

void CPsSynthesizer::NoteOff(int channel, int note, int velocity)
{
	--m_noteState[channel][note];
	if(m_flags[channel] & F_ENABLED)
	{
		++m_noteState[channel][note];
		m_inst[channel].NoteOff(note, I2F(velocity) / 127, false);
	}
}

void CPsSynthesizer::SetCoarseVolume(int channel, int volume)
{
	m_inst[channel].SetVolume(I2F(volume) / 127);
}

void CPsSynthesizer::SetCoarseExpression(int channel, int volume)
{
	m_inst[channel].SetExpression(I2F(volume) / 127);
}

void CPsSynthesizer::SetCoarsePan(int channel, int pan)
{
	m_inst[channel].SetPan(I2F(pan - 64) / 128 + F_ONE / 256);
}

void CPsSynthesizer::SetCoarseChorus(int channel, int chorus)
{
	m_inst[channel].SetChorus(I2F(chorus) / 128);
}

void CPsSynthesizer::SetCoarseReverb(int channel, int reverb)
{
	m_inst[channel].SetReverb(I2F(reverb) / 128);
}

void CPsSynthesizer::SetCoarseBalance(int channel, int balance)
{
	m_inst[channel].SetBalance(I2F(balance - 64) / 128);
}

void CPsSynthesizer::SetTranspose(int nKey)
{
	m_keyTranspose = nKey;
	int i;
	for(i = 0; i < TOTAL_INSTRUMENT; i++)
	{
		m_inst[i].SetKeyTranspose(m_flags[i] & F_DRUM ? 0 : nKey);
	}
}

void CPsSynthesizer::AllNoteOff(int channel)
{
	CPsSys::memset(m_noteState[channel], 0, sizeof(m_noteState[0]));
	m_inst[channel].AllNoteOff();
}

void CPsSynthesizer::ResetAllControllers(int channel)
{
	SetCoarseRPN(channel, 0x7f);
	SetFineRPN(channel, 0x7f);
	SetCoarseVolume(channel, 100);
	SetCoarseExpression(channel, 127);
	SetCoarseModulation(channel, 0);
	SetHoldPedal(channel, 0);
	SetPitchBand(channel, 0x2000);
}

void CPsSynthesizer::LoadProgram(int channel)
{
	unsigned int b = (m_bank[channel][1] << 8) | m_bank[channel][0];
	if(m_flags[channel] & F_DRUM)
		b |= 0x80000000;
	m_pPatch[channel] = m_sb.LoadPatch(m_pSystem->GetMath(), b, m_prog[channel], NULL);
	if(m_pPatch[channel])
	{
		m_flags[channel] |= F_ENABLED;
		m_inst[channel].SetPatch(m_pPatch[channel]);
	}
	else
	{
		m_flags[channel] &= ~F_ENABLED;
	}
}

void CPsSynthesizer::SetProgram(int channel, int prog)
{
	m_prog[channel] = prog;
	m_pPatch[channel] = NULL;
	m_flags[channel] |= F_ENABLED;
}

void CPsSynthesizer::SetCoarseModulation(int channel, int m)
{
	m_inst[channel].SetModulation(I2F(m) / 127);
}

void CPsSynthesizer::SetCoarseDataEntry(int channel, int d)
{
	F32 f;
	int n;

	switch(m_RPN[channel])
	{
	case 0:
		f = m_inst[channel].GetPitchBandRange() % I2F(100);
		m_inst[channel].SetPitchBandRange(f + I2F(d * 100));
		break;
	case 1:
		PsAssert(F_SHIFT == 14);
		n = (m_inst[channel].GetMasterTuning() / 100 >> 1) + 0x2000;
		n = (n & 0x7f) | (d << 7);
		m_inst[channel].SetMasterTuning(((n - 0x2000) << 1) * 100);
		break;
	case 2:
		f = m_inst[channel].GetMasterTuning() % I2F(100);
		m_inst[channel].SetMasterTuning(f + I2F((d - 0x40) * 100));
		break;
	}
}

void CPsSynthesizer::SetFineDataEntry(int channel, int d)
{
	F32 f;
	int n;

	switch(m_RPN[channel])
	{
	case 0:
		f = m_inst[channel].GetPitchBandRange() / I2F(100);
		m_inst[channel].SetPitchBandRange(I2F(f * 100 + d));
		break;
	case 1:
		PsAssert(F_SHIFT == 14);
		n = (m_inst[channel].GetMasterTuning() / 100 >> 1) + 0x2000;
		n = (n & 0x3f80) | d;
		m_inst[channel].SetMasterTuning(((n - 0x2000) << 1) * 100);
		break;
	}
}

void CPsSynthesizer::ResetRPN(int channel)
{
	m_inst[channel].SetPitchBandRange(I2F(200));
	m_inst[channel].SetMasterTuning(0);
}

void CPsSynthesizer::GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList)
{
	m_sb.FreeAllPatchs();
	Reset();
	if(pProgList)
	{
		int i;
		for(i = 0; i < nProg; i++)
		{
			if(!(pProgList[i] & 0x80000000))//Load drum patch later
				m_sb.LoadPatch(m_pSystem->GetMath(), pProgList[i] >> 16, pProgList[i] & 0xffff, NULL);
		}
	}
	if(pDrumList)
		m_sb.LoadPatch(m_pSystem->GetMath(), 0x80000000, 0, pDrumList);
}

void CPsSynthesizer::Reset()
{
	m_oscPool.Reset(m_pSystem);
	int i;
	for(i = 0; i < TOTAL_INSTRUMENT; i++)
	{
		m_inst[i].Close();
		m_inst[i].Open(m_pSystem, &m_oscPool, i);
		m_flags[i] = 0;
		SetCoarseBank(i, 0);
		SetFineBank(i, 0);
		SetProgram(i, 0);
		ResetAllControllers(i);
		SetCoarseVolume(i, 100);
		SetCoarseExpression(i, 127);
		SetCoarsePan(i, 64);
		SetCoarseBalance(i, 64);
	}
	SetDrumChannel(CPsSequence::DRUM_CHANNEL, true);
	CPsSys::memset(m_noteState, 0, sizeof(m_noteState));
	SetTranspose(m_keyTranspose);
}

void CPsSynthesizer::SetPitchBand(int channel, int n)
{
	m_inst[channel].SetPitchBand(I2F(n) / 16384);
}

void CPsSynthesizer::SetHoldPedal(int channel, int n)
{
	m_inst[channel].SetHoldPedal(n >= 64);
}

void CPsSynthesizer::ReduceVoice()
{
	m_oscPool.ReduceVoice();
}
