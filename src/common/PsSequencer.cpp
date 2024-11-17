#include "PsAudioSystem.h"
#include "PsSys.h"
#include "PsSequencer.h"
#include "PsMixer.h"

#define USE_ACCURATE_ORDER

CPsSequencer::CPsSequencer()
{
	m_pSystem = NULL;
	m_pSequence = NULL;
	m_pOutputPorts = NULL;
	m_eventCounter = 0;
	m_tickCounter = 0;
	m_stopDelayTime = 0;
	m_state = STATE_STOPPED;
}

CPsSequencer::~CPsSequencer()
{
	Close();
}

void CPsSequencer::Open(CPsAudioSystem *pSystem)
{
	m_pSystem = pSystem;
	m_pSequence = NULL;
	m_eventCounter = 0;
	m_tickCounter = 0;
	m_state = STATE_STOPPED;
	m_tempoScale = F_ONE;
	m_isSetEnd = false;
	m_isForwarding = false;
	m_nOutputPorts = pSystem->GetOutputPortNumber();
	m_pOutputPorts = pSystem->GetOutputPorts();
}

void CPsSequencer::Close()
{
	if(m_pSystem)
	{
		Stop();
		m_pSystem = NULL;
		m_pOutputPorts = NULL;
		m_tickCounter = 0;
	}
}

void CPsSequencer::SetSequence(CPsSequence *pSequence)
{
	m_pSequence = pSequence;
	m_eventCounter = 0;
	m_state = STATE_STOPPED;
	m_songTime = 0;
	m_elapsedTime = 0;
	m_tickCounter = 0;
	m_tempo = CPsSequence::DEFAULT_MIDI_TEMPO;//default tempo is 120bpm
//	CPsSys::memset(m_lastTick, 0, sizeof(m_lastTick));
//	CPsSys::memset(m_lastEvent, 0, sizeof(m_lastEvent));

//	DWORD tick = GetTickCount();
	CPsSequence::MIDIINFO *pInfo = m_pSequence->GetInfo();
	for(int i = 0; i < m_nOutputPorts; i++)
	{
		if(pInfo)
			m_pOutputPorts[i]->GMSystemOn(pInfo->programs, pInfo->nProgram, pInfo->drums);
		else
			m_pOutputPorts[i]->GMSystemOn(NULL, 0, NULL);
	}
//	tick = GetTickCount() - tick;
//	PsPrintDebugString("%d", tick, 0, 0);
}

void CPsSequencer::Play()
{
	if(m_pSystem)
	{
		if(m_state == STATE_STOPPED)
		{
			Rewind();
		}
		m_isSetEnd = false;
		UpdateTicksPerBlock();
		m_state = STATE_PLAYING;
		m_pSystem->GetMixer()->AddPlayer(this);
	}
}

void CPsSequencer::TurnOffAllNotes()
{
#if 0
	CPsTrack::EVENT evt;
	evt.deltaTime = m_tickCounter >> TICK_SHIFT;
	evt.eventCode = 0xb0;
	evt.par1 = 123;
	evt.par2 = 0;
	evt.nExtraData = 0;
	evt.pExtraData = NULL;
	for(int i = 0; i < CPsSynthesizer::TOTAL_INSTRUMENT; i++)
	{
		evt.channel = i;
		unsigned char data[7];
		CPsMidiWriter w(data, 7);
		w.WriteVarLen(evt.deltaTime);
		w.WriteInt8(0xb0 | i);
		w.WriteInt8(123);
		w.WriteInt8(0);
		evt.nRawData = w.GetCurrent() - data;
		evt.pRawData = data;
		for(int j = 0; j < m_nOutputPorts; j++)
			m_pOutputPorts[j]->Send(0, &evt);
	}
#else
	for(int j = 0; j < m_nOutputPorts; j++)
		m_pOutputPorts[j]->AllNoteOff();
#endif
}

bool CPsSequencer::IsAnyPortActive()
{
	for(int i = 0; i < m_nOutputPorts; i++)
		if(m_pOutputPorts[i]->GetActiveCount() > 0)
			return true;
	return false;
}

bool CPsSequencer::ProcessEvent()
{
#ifdef USE_ACCURATE_ORDER
	int nEnd = 0;
	int nTrack = m_pSequence->GetTrackNumber();
	while(true)
	{
		int trk = -1;
		int mintick = 0x7fffffff;
		for(int i = 0; i < nTrack; i++)
		{
			if(m_lastTick[i] + m_delayInTick[i] < mintick)
			{
				mintick = m_lastTick[i] + m_delayInTick[i];
				trk = i;
			}
		}
		if(trk < 0)
			return false;
		if(mintick > m_tickCounter)
			break;
		CPsTrack *pTrack = m_pSequence->GetTrack(trk);
//		printf("trk: %d chn:%d evt: %d(%x) par1: %d\n", trk, m_lastEvent[trk].channel, m_lastEvent[trk].eventCode, m_lastEvent[trk].eventCode, m_lastEvent[trk].par1);
		ParseEvent(m_elapsedTime + ((mintick + m_fTickPerUpdate - m_tickCounter) * m_MSPertick >> TICK_SHIFT), pTrack, m_lastEvent + trk);
		m_eventCounter++;
		if(pTrack->ReadEvent(m_lastEvent + trk))
		{
			if(m_lastEvent[trk].command < 0xf0)
			{
				if(m_pSequence->GetTrack(trk)->GetOutputPort() < m_nOutputPorts)
				{
					short *pDelays = m_pOutputPorts[m_pSequence->GetTrack(trk)->GetOutputPort()]->GetChannelDelays();
					m_delayInTick[trk] = (unsigned int)((pDelays[m_lastEvent[trk].channel] * (F64)m_tickPerSomeSecond) / 256);
				}
			}
			m_lastTick[trk] += m_lastEvent[trk].deltaTime << TICK_SHIFT;
		}
		else
			m_lastTick[trk] = 0x7fffffff;//no more process this track
	}
	return true;
#else
	int nEnd = 0;
	for(int i = 0; i < m_pSequence->GetTrackNumber(); i++)
	{
		CPsTrack *pTrack = m_pSequence->GetTrack(i);
		if(pTrack->IsEndOfTrack())
			++nEnd;
		else while(m_lastTick[i] + m_delayInTick[i] <= m_tickCounter)
		{
//			printf("trk: %d chn:%d evt: %d(%x) par1: %d\n", i, m_lastEvent[i].channel, m_lastEvent[i].eventCode, m_lastEvent[i].eventCode, m_lastEvent[i].par1);
			ParseEvent(m_elapsedTime + ((m_lastTick[i] + m_delayInTick[i] + m_fTickPerUpdate - m_tickCounter) * m_MSPertick >> TICK_SHIFT), pTrack, m_lastEvent + i);
			m_eventCounter++;
			if(pTrack->ReadEvent(m_lastEvent + i))
			{
				if(m_lastEvent[trk].command < 0xf0)
				{
					if(m_pSequence->GetTrack(i)->GetOutputPort() < m_nOutputPorts)
					{
						short *pDelays = m_pOutputPorts[m_pSequence->GetTrack(i)->GetOutputPort()]->GetChannelDelays();
						m_delayInTick[i] = (unsigned int)((pDelays[m_lastEvent[i].channel] * (F64)m_tickPerSomeSecond) / 65536);
					}
				}
				m_lastTick[i] += m_lastEvent[i].deltaTime << TICK_SHIFT;
			}
			else
			{
				m_lastTick[i] = 0x7fffffff;
				break;
			}
		}
	}
	return m_pSequence->GetTrackNumber() > nEnd;
#endif
}

void CPsSequencer::Update(bool isAfterMix)
{
	if(!m_isForwarding && isAfterMix)
	{
		for(int i = 0; i < m_nOutputPorts; i++)
		{
			m_pOutputPorts[i]->ReduceVoice();
		}
		return;
	}
	if(m_state == STATE_PLAYING)
	{
//		printf("%d\n", m_pSystem->GetDriver()->GetBufferedSamplePosition() - m_pSystem->GetDriver()->GetPlayingSamplePosition());
		if(m_isSetEnd || !ProcessEvent())
		{
			m_isForwarding = false;
			m_state = STATE_STOPPING1;
			m_stopCounter = 0;
			TurnOffAllNotes();
		}
		m_songTime += m_songMSPerUpdate;
		m_tickCounter += m_fTickPerUpdate;
	}
	else if(m_state == STATE_STOPPING1 || m_state == STATE_STOPPING2)
	{
//		m_tickCounter += m_fTickPerUpdate;
		m_stopCounter += m_MSPerUpdate;
		if(((m_stopCounter > 500 && !IsAnyPortActive()) || m_stopCounter > 1500000) && (m_isSetEnd || m_stopCounter > m_stopDelayTime))
		{
			Stop();
		}
		else if (m_state == STATE_STOPPING1 && m_stopCounter > 1500000)
		{
			Rewind();
			m_state = STATE_STOPPING2;
		}
	}
	if(m_state == STATE_PLAYING || m_state == STATE_PAUSED)
	{
		if(!m_isForwarding)
			m_elapsedTime += m_MSPerUpdate;
	}
}

void CPsSequencer::Pause()
{
	if(m_pSystem)
	{
		if(m_state == STATE_PLAYING)
		{
			m_state = STATE_PAUSED;
			TurnOffAllNotes();
		}
	}
}

void CPsSequencer::Stop()
{
	if(m_pSystem && m_pSequence && m_state != STATE_STOPPED)
	{
		m_pSystem->GetMixer()->RemovePlayer(this);
		Rewind();
		m_state = STATE_STOPPED;
	}
}

void CPsSequencer::Rewind()
{
	m_eventCounter = 0;
	m_songTime = 0;
	if(!m_isForwarding)
		m_elapsedTime = 0;
	m_tickCounter = 0;
	m_tempo = CPsSequence::DEFAULT_MIDI_TEMPO;//default tempo is 120bpm
	CPsSys::memset(m_lastTick, 0, sizeof(m_lastTick));
	CPsSys::memset(m_lastEvent, 0, sizeof(m_lastEvent));
	m_pSequence->Rewind();
	for(int i = 0; i < m_nOutputPorts; i++)
		m_pOutputPorts[i]->Reset();
	CPsSys::memset(m_delayInTick, 0, sizeof(m_delayInTick));
}

void CPsSequencer::ParseEvent(unsigned int framePosition, CPsTrack *pTrack, CPsTrack::EVENT *pEvent)
{
//	printf("clock: %d, song pos: %d, stamp: %d, delta: %d\n", m_songTime, m_elapsedTime, timeStamp, timeStamp - m_elapsedTime);
	switch(pEvent->eventCode) {
	case CPsTrack::EVENT_EMPTY:
		return;
	case CPsTrack::EVENT_META:
		switch(pEvent->par1)
		{
		case 0x51:
			m_tempo = (pEvent->pExtraData[0] << 16) | (pEvent->pExtraData[1] << 8) | pEvent->pExtraData[2];
			UpdateTicksPerBlock();
			return;
		case 0x21:
			pTrack->SetOutputPort(pEvent->pExtraData[0]);
			return;
		}
		break;

	case CPsTrack::EVENT_NOTE_ON:
	case CPsTrack::EVENT_NOTE_OFF:
		if(m_isForwarding)
			return;
	}
	unsigned char port = pTrack->GetOutputPort();
//	printf("port:%d, evt:%d\n", port, pEvent->eventCode);
	if(port < m_nOutputPorts)
		m_pOutputPorts[port]->Send(framePosition, pEvent);
}

void CPsSequencer::UpdateTicksPerBlock()
{
	//m_fTickPerBlock = BUFFER_SIZE * 1000 * m_pSequence->GetDivision() * 1000 / (pEngine->GetMixFreq() * m_tempo);
	PsAssert(m_pSystem->GetConfig()->GetFramesPerUpdate() < 0x7fffffff / (1000 * 1000));
	m_MSPerUpdate = m_pSystem->GetConfig()->GetFramesPerUpdate() * 1000 * 1000 / m_pSystem->GetConfig()->GetMixFrequency();
	m_songMSPerUpdate = (unsigned int)FMUL32(m_MSPerUpdate, m_tempoScale);
	m_fTickPerUpdate = (int)(((F64)(m_songMSPerUpdate * m_pSequence->GetDivision()) << TICK_SHIFT) / m_tempo);
	m_MSPertick = (int)(I2F((F64)m_tempo) / (m_pSequence->GetDivision() * m_tempoScale));
	m_framePerTick = (unsigned int)((F64)m_MSPertick * m_pSystem->GetConfig()->GetMixFrequency() / 1000000);
	m_tickPerSomeSecond = (256000 << TICK_SHIFT) / m_MSPertick;
}

unsigned int CPsSequencer::SetTickPosition(unsigned int t)
{
	bool isPause;

	if(m_state == STATE_PLAYING)
	{
		isPause = false;
		TurnOffAllNotes();
	}
	else if(m_state == STATE_PAUSED)
		isPause = true;
	else
		return m_tickCounter;

	m_isForwarding = true;
	if(m_tickCounter > t)
	{
		Stop();
		Play();
	}
	while(m_isForwarding && (m_tickCounter >> TICK_SHIFT) < t)
	{
		Update(false);
		Update(true);
	}
	m_isForwarding = false;
	if(isPause)
		m_state = STATE_PAUSED;
	return m_tickCounter;
}

//return time in ms
int CPsSequencer::GetTimePosition()
{
	return m_songTime / 1000;
}
