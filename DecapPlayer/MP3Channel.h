#ifndef __MP3CHANNEL_H__
#define __MP3CHANNEL_H__

#include "common/PsObject.h"
#include "PsFileReader.h"
#include "libmad/mad.h"
#include "libmad/xing.h"

class CMP3Channel : public CPsObject, public CPsChannel
{
public:
	enum {
		MAX_OUTPUT_SAMPLE_PER_UPDATE = 800 + 4,//mixer will read 3 more byte for interpolation
		MAX_SYNTH_SAMPLE = 1152
	};
	F16 GetReverb();
	F16 GetChorus();
	const CPsSampleData* GetSampleData();
	void SetState(int state);
	int GetState();
	F32 GetBalance();
	F32 GetPan();
	F32 GetMixFrequency();
	F32 GetMixVolume();
	int GetTotalTime();
	void Update();
	bool Open(CPsAudioSystem *pSystem, LPCTSTR pMP3File);
	void Close();
	CMP3Channel();
	virtual ~CMP3Channel();
	void SetMixVolume(F16 vol) { m_fMixVolume = vol; }

	F64 m_totalDecoded;
protected:
	CPsAudioConfig *m_pConfig;
	F32 m_fMixVolume;
	int m_state;
	CPsFileReader m_file;
	unsigned char m_inputBuffer[6*8192];
	NativeSample m_sample[(MAX_OUTPUT_SAMPLE_PER_UPDATE + MAX_SYNTH_SAMPLE) * 2];
	NativeSample m_lastSample[2];
	int m_lastSamplePos;

	CPsSampleData m_sampleData;

	struct mad_stream	m_stream;
	struct mad_frame	m_frame;
	struct mad_synth	m_synth;
	F32 m_fResamplePos;

	int m_totalTime; //in millisecond
};

#endif
