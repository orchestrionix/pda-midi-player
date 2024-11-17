#ifndef __PSTRACK_H__
#define __PSTRACK_H__

#include "PsObject.h"
#include "PsMidiIO.h"

class CPsTrack : public CPsObject
{
public:
	enum {
		EVENT_EMPTY = 0,
		EVENT_NOTE_OFF = 0x80,
		EVENT_NOTE_ON = 0x90,
		EVENT_AFTERTOUCH = 0xa0,
		EVENT_CONTROL_CHANGE = 0xb0,
		EVENT_PROGRAM_CHANGE = 0xc0,
		EVENT_CHANNEL_PRESSURE = 0xd0,
		EVENT_PITCH_BEND = 0xe0,
		EVENT_SYSEX = 0xf0,
		EVENT_META = 0xff
	};

	enum {
		STATUS_MORE_EVENT,
		STATUS_LAST_EVENT,
		STATUS_NO_EVENT
	};

	enum {
		LEAD_TRACK = 0x80000000
	};
	typedef struct {
		int deltaTime;
		unsigned char command;
		unsigned char eventCode;
		unsigned char channel;
		short par1;
		short par2;
		int nExtraData;
		int nRawData;
		const unsigned char *pExtraData;
		const unsigned char *pRawData;
	}EVENT;

	CPsTrack();
	
	void Rewind();
	int Attach(const void* pTrack, int len, unsigned int chnmask);
	void Detach();
	bool ReadEvent(CPsTrack::EVENT *pEvent);

	int GetNextTick(){
		return m_nextTick;
	}

	bool IsEndOfTrack(){
		return m_trackState == STATUS_NO_EVENT;
	}

	unsigned char GetOutputPort(){
		return m_outputPort;
	}

	void SetOutputPort(unsigned char port){
		m_outputPort = port;
	}

protected:
	CPsMidiReader m_reader;
	unsigned char m_lastEventCode;
	unsigned char m_outputPort;
	unsigned char m_trackState;
	unsigned int m_channelMask;
	unsigned int m_nextTick;
};

#endif
