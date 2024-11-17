#ifndef __PSFILTERREVERBSCHROEDER_H__
#define __PSFILTERREVERBSCHROEDER_H__

#include "PsObject.h"
#include "PsMath.h"

#define F8_SHIFT 8
#define F8_ONE (1 << F8_SHIFT)
#define F2F8(x) ((x) >> (F_SHIFT - F8_SHIFT))

#define FAST_REVERB

class CPsReverbSchroeder : public CPsObject
{
public:
	enum {
		NUM_COMB = 4,//max 8
		NUM_ALLPASS = 2,//max 4
		STEREOSPREAD = 32,
		BUFFER_SIZE = 0x10000,
	};

	typedef struct {
		int* buf;
		unsigned short len, pos;
	}CIRCLEBUFFER;

	void Mute();
	void SetSampleRate(int rate);
#ifndef FAST_REVERB
	void SetDamp(F32 f);
	void SetWet(F32 fWet);
	void SetRoomSize(F32 f);
	void SetWidth(F32 fWidth);
#endif
	bool Init(CPsAudioConfig *pConfig);
	void Shutdown();
	void Process(int *sourceP, int *destP, int nSampleFrames);

	CPsReverbSchroeder();
	virtual ~CPsReverbSchroeder();
	
protected:
	void Update();

	int m_fWet, m_fWet1, m_fWet2;
	int m_fWidth, m_fRoomSize;
	int m_fDamp;
	int m_buffer[BUFFER_SIZE];

	CIRCLEBUFFER m_comb[NUM_COMB][2];
	int m_combStored[NUM_COMB][2];

	CIRCLEBUFFER m_allpass[NUM_ALLPASS][2];
	
	static const unsigned short m_nCombLen[NUM_COMB];
	static const unsigned short m_nAllPassLen[NUM_ALLPASS];
};

#endif
