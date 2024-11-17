#ifndef __PSSEQUENCE_H__
#define __PSSEQUENCE_H__

#include "PsTrack.h"

class CPsSequence : public CPsObject
{
public:
	enum {
		DEFAULT_MIDI_TEMPO = 500000, //default midi tempo by standard midi specification

		DRUM_CHANNEL = 9,
		MAX_TRACK = 10 * 16
	};

	typedef struct {
		int nDivision;
		unsigned short nMidiTrack;
		unsigned short format;
		int nTick;

		int length;//midi length in ms
		int trackStart[MAX_TRACK];
		int trackLength[MAX_TRACK];
		unsigned short trackChannel[MAX_TRACK];
		unsigned char hasNote[MAX_TRACK];
		int nProgram;
		unsigned int programs[64];
		unsigned char drums[128];
	}MIDIINFO;

	typedef struct {
		int bank;
		int program;
		int totalNote;
		unsigned short notes[128];
	}FINDNOTEINFO;

	CPsSequence();
	
	void Rewind();
	int Attach(const void* pMidi, int len);
	void Detach();

	static int CollectMidiInfo(const void *pSrc, int nSrc, MIDIINFO *pInfo);
	static int FindNote(const void *pSrc, int nSrc, FINDNOTEINFO *pInfo);

	CPsTrack* GetTrack(int i){
		PsAssert(i < MAX_TRACK);
		return m_track + i;
	}

	int GetDivision(){
		return m_info.nDivision;
	}

	MIDIINFO* GetInfo(){
		return &m_info;
	}

	int GetTrackNumber() const {
		return m_nTrack;
	}
	
protected:
	MIDIINFO m_info;
	const unsigned char *m_pMidi;
	int m_nTrack;
	CPsTrack m_track[MAX_TRACK];
};

#endif
