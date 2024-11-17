#include "PsAudioSystem.h"
#include "PsPortSoftSynth.h"
#include "PsTrack.h"
#include "PsSynthesizer.h"
#include "PsSys.h"

CPsOutputPort::CPsOutputPort()
{
	CPsSys::memset(m_channelDelays, 0, sizeof(m_channelDelays));
	CPsSys::memset(m_channelCompression, 0x7f, sizeof(m_channelCompression));
}

CPsPortSoftSynth::CPsPortSoftSynth()
{
	m_pSynth = NULL;
	m_isEnabled = true;
}

void CPsPortSoftSynth::AttachSynthesizer(CPsSynthesizer *pSynth)
{
	m_pSynth = pSynth;
}

void CPsPortSoftSynth::DetachSynthesizer()
{
	m_pSynth = NULL;
}

void CPsPortSoftSynth::Send(unsigned int framePosition, void *pEvent)
{
	CPsTrack::EVENT *p = (CPsTrack::EVENT*)pEvent;

	if(!m_isEnabled)
		return;

	switch(p->eventCode)
	{
	case CPsTrack::EVENT_NOTE_ON:
		m_pSynth->NoteOn(p->channel, p->par1, p->par2);
		break;

	case CPsTrack::EVENT_NOTE_OFF:
		m_pSynth->NoteOff(p->channel, p->par1, p->par2);
		break;

	case CPsTrack::EVENT_CONTROL_CHANGE:
		switch(p->par1)
		{
		case 0://Coarse Bank Select
			m_pSynth->SetCoarseBank(p->channel, p->par2);
			break;
		case 32://Fine Bank Select
			m_pSynth->SetFineBank(p->channel, p->par2);
			break;
		case 1://Coarse Modulation
			m_pSynth->SetCoarseModulation(p->channel, p->par2);
			break;
		case 7://Coarse Volume
			{
				int v = p->par2 * m_channelCompression[p->channel] / 127;
				if(v < 0)
					v = 0;
				else if(v > 127)
					v = 127;
				m_pSynth->SetCoarseVolume(p->channel, v);
			}
			break;
/*		case 39://Fine Volume
			m_pSynth->SetFineVolume(p->channel, p->par2);
			break;*/
		case 11://Coarse Expression
			m_pSynth->SetCoarseExpression(p->channel, p->par2);
			break;
/*		case 43://Fine Expression
			m_pSynth->SetFineExpression(p->channel, p->par2);
			break;*/
		case 8:
			m_pSynth->SetCoarseBalance(p->channel, p->par2);
			break;
		case 10:
			m_pSynth->SetCoarsePan(p->channel, p->par2);
			break;
		case 64:
			m_pSynth->SetHoldPedal(p->channel, p->par2);
			break;
		case 91:
			m_pSynth->SetCoarseReverb(p->channel, p->par2);
			break;
		case 93:
			m_pSynth->SetCoarseChorus(p->channel, p->par2);
			break;
		case 6:
			m_pSynth->SetCoarseDataEntry(p->channel, p->par2);
			break;
		case 38:
			m_pSynth->SetFineDataEntry(p->channel, p->par2);
			break;
		case 100://fine RPN
			m_pSynth->SetFineRPN(p->channel, p->par2);
			break;
		case 101://coarse RPN
			m_pSynth->SetCoarseRPN(p->channel, p->par2);
			break;
		case 123://All Note Off
			m_pSynth->AllNoteOff(p->channel);
			break;
		case 121:
			m_pSynth->ResetAllControllers(p->channel);
			break;
		}
		break;

	case CPsTrack::EVENT_PROGRAM_CHANGE:
		m_pSynth->SetProgram(p->channel, p->par1);
		break;

	case CPsTrack::EVENT_PITCH_BEND:
		m_pSynth->SetPitchBand(p->channel, p->par1);
		break;
	}
}

const char* CPsPortSoftSynth::GetName()
{
	return m_isEnabled ? "IntelliArt PowerSynth" : "IntelliArt PowerSynth(Disabled)";
}

void CPsPortSoftSynth::Reset()
{
	m_pSynth->Reset();
	for (int i = 0; i < 16; ++i)
	{
		m_pSynth->SetCoarseVolume(i, 100 * m_channelCompression[i] / 127);
	}
}

void CPsPortSoftSynth::AllNoteOff()
{
	for(int i = 0; i < CPsSynthesizer::TOTAL_INSTRUMENT; i++)
		m_pSynth->AllNoteOff(i);
}

void CPsPortSoftSynth::ReduceVoice()
{
	m_pSynth->ReduceVoice();
}

int CPsPortSoftSynth::GetActiveCount()
{
	return m_pSynth->GetActiveCount();
}

void CPsPortSoftSynth::GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList)
{
	m_pSynth->GMSystemOn(pProgList, nProg, pDrumList);
}
