#ifndef __PSSEQUENCER_H__
#define __PSSEQUENCER_H__

#include "PsObject.h"
#include "PsSequence.h"
#include "PsSynthesizer.h"

class CPsSequencer : public CPsObject, public CPsPlayer
{
public:
	enum {
		TICK_SHIFT = 8,
	};

	enum {
		STATE_STOPPED,
		STATE_PLAYING,
		STATE_PAUSED,
		STATE_STOPPING1,
		STATE_STOPPING2
	};

	CPsSequencer();
	virtual ~CPsSequencer();
	unsigned int SetTickPosition(unsigned int t);
	void Open(CPsAudioSystem *pSystem);
	void Close();
	void UpdateTicksPerBlock();
	void Pause();
	void Stop();
	void Play();
	void Rewind();
	void SetSequence(CPsSequence *pSequence);
	void Update(bool isAfterMix);
	int GetTimePosition();

	int GetState(){
		return m_state;
	}

	unsigned int GetTickPosition(){
		return m_tickCounter >> TICK_SHIFT;
	}

	int GetTempoScale(){
		return m_tempoScale;
	}

	void SetTempoScale(F32 scale){
		m_tempoScale = scale;
		UpdateTicksPerBlock();
	}

	void EndPlay(){
		if(m_state == STATE_PLAYING)
			m_isSetEnd = true;
		else if(m_state == STATE_PAUSED)
		{
			m_state = STATE_PLAYING;
			m_isSetEnd = true;
		}
	}
	
	void SetStopDelayTime(int nDelay) {m_stopDelayTime = nDelay;}
	int GetStopDelayTime() const { return m_stopDelayTime; }
	int GetStopCount() const {return m_isSetEnd ? m_stopDelayTime : m_stopCounter; }

protected:
	void ParseEvent(unsigned int framePosition, CPsTrack *pTrack, CPsTrack::EVENT *pEvent);
	void TurnOffAllNotes();
	bool ProcessEvent();
	bool IsAnyPortActive();
	CPsAudioSystem *m_pSystem;

	int m_eventCounter;
	int m_state;
	bool m_isForwarding;
	int m_stopCounter;
	int m_stopDelayTime; //in microsecond

	unsigned int m_lastTick[CPsSequence::MAX_TRACK];
	unsigned int m_delayInTick[CPsSequence::MAX_TRACK];
	CPsTrack::EVENT m_lastEvent[CPsSequence::MAX_TRACK];

	unsigned int m_songTime;//song position time in microsecond
	unsigned int m_elapsedTime;//true time in microsecond
	int m_tickCounter;
	int m_fTickPerUpdate;
	unsigned int m_MSPerUpdate;//true time per update in microsecond
	unsigned int m_songMSPerUpdate;//song position time per update in microsecond
	F32 m_tempoScale;
	int m_tempo;//microseconds per quarter note
	int m_MSPertick;//microseconds per tick
	unsigned int m_tickPerSomeSecond;
	unsigned int m_framePerTick;

	bool m_isSetEnd;
	CPsSequence *m_pSequence;
	int m_nOutputPorts;
	CPsOutputPort **m_pOutputPorts;
};

#endif
