#ifndef __PSOBJECT_H__
#define __PSOBJECT_H__

#include "PSType.h"

#define ENABLE_PAN_DELAY

#define MIX_SHIFT	8

#define GET_ELEMENT_COUNT(x) (sizeof(x) / sizeof(x[0]))

#define NATIVE_SAMPLE_SAT(x) (((x) > NATIVE_SAMPLE_MAX)?NATIVE_SAMPLE_MAX:(((x) < NATIVE_SAMPLE_MIN)?NATIVE_SAMPLE_MIN:NativeSample(x)))

#define MAX_OUTPUT_PORT 2
//#define DECAP_PLAYER

class CPsObject  
{
public:
	enum {
		PSERR_SUCCESS,
		PSERR_INVALID_PARAM,
		PSERR_INVALID_FORMAT,
		PSERR_INVALID_DATA,
		PSERR_BUFFER_SIZE
	};
#ifdef NDEBUG
	static void PsAssert(int f)
	{
	}
#else
	static void PsAssert(int f)
	{
#ifdef OS_WIN32
		if(!f)DebugBreak();
#endif
	}
#endif
	static void PsPrintDebugString(const char *str, int a = 0, int b = 0, int c = 0, int d = 0, int e = 0)
	{
#ifdef OS_WIN32
		TCHAR s[100];
		TCHAR wstr[100];
		HDC dc = GetDC(NULL);
		RECT rc = {
			0, 0, 200, 30
		};
		wsprintf(wstr, TEXT("%S   "), str);
		wsprintf(s, wstr, a, b, c, d, e);
		DrawText(dc, s, -1, &rc, DT_LEFT | DT_TOP);
		ReleaseDC(NULL, dc);
//		OutputDebugString(s);
#endif
	}
};

class CPsAudioSystem;
class CPsMath;

class CPsAudioConfig : public CPsObject
{
public:
	void SetMixFrequency(CPsMath *pMath, int freq, int nFramePerUpdate = 0);
	
	int GetFramesPerUpdate(){
		return m_nFramePerUpdate;
	}
	
	F32 GetUpdateFrequency() {
		return m_fUpdateFrequency;
	}

	unsigned int GetMixFrequency() {
		return m_nMixFrequency;
	}
	
	F32 GetMixPitch(){
		return m_fMixPitch;
	}

protected:
	unsigned int m_nMixFrequency;
	F32 m_fMixPitch;
	unsigned int m_nFramePerUpdate;
	F32 m_fUpdateFrequency;
};

class CPsPlayer
{
public:
	virtual void Update(bool isAfterMix) = 0;
};

class CPsSampleData
{
public:
	unsigned char	*pData;
	unsigned char	nBitPerSample;
	unsigned char	nChannel;
	unsigned short	loopType;
	F64				loopStart;
	F64				loopLength;
};

class CPsChannel
{
public:
	enum {
		STATE_STOPPED,
		STATE_PLAYING,
	};
	virtual F32 GetMixVolume() = 0;
	virtual F32 GetMixFrequency() = 0;
	virtual const CPsSampleData* GetSampleData() = 0;
	virtual F32 GetPan() = 0;//return pan in [-0.5, 0.5]
	virtual F32 GetBalance() = 0;
	virtual void Update() = 0;
	virtual int GetState() = 0;
	virtual void SetState(int state) = 0;
	virtual F16 GetReverb() = 0;
	virtual F16 GetChorus() = 0;

	F64 m_fMixPosition;
	F16 m_fLastLVol, m_fLastRVol;
};

class CPsReader : public CPsObject
{
public:
	enum {
		REF_BEGIN,
		REF_CURRENT,
		REF_END
	};
	virtual int ReadInt() = 0;
	virtual short ReadShort() = 0;
	virtual signed char ReadChar() = 0;
	virtual unsigned int ReadBlock(void* pBuf, unsigned int len) = 0;
	virtual unsigned int SetPosition(unsigned int pos, int ref) = 0;
	virtual unsigned int GetPosition() = 0;
	virtual unsigned int GetLength() = 0;
	virtual bool IsEof() = 0;
};

class CPsGenerator : public CPsObject
{
public:
	virtual unsigned int Write(NativeSample *pOut) = 0; //return the number of frames written
};

class CPsDriver : public CPsObject
{
public:
	virtual bool Open(int freq, int nBuffers, int nFramesPerBuffer, bool isStereo) = 0;
	virtual void Close() = 0;
	virtual bool Lock() = 0;
	virtual void Unlock() = 0;
	virtual bool Pause() = 0;
	virtual bool Resume() = 0;
	virtual void Update(CPsGenerator *pMixer) = 0;
	virtual unsigned int GetPlayingFramePosition() = 0;
	virtual unsigned int GetBufferedFramePosition() = 0;
};

class CPsOutputPort : public CPsObject
{
public:
	CPsOutputPort();
	virtual const char* GetName() = 0;
	virtual void Reset() = 0;
	virtual void AllNoteOff() = 0;
	virtual void ReduceVoice() = 0;
	virtual int GetActiveCount() = 0;
	virtual void Send(unsigned int framePosition, void *pEvent) = 0;
	virtual void GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList) = 0;
	short* GetChannelDelays() {
		return m_channelDelays;
	}

	char* GetChannelCompression() {
		return m_channelCompression;
	}

protected:
	//todo: implement in PSPortSoftSynth, init/load m_channelCompression
	char m_channelCompression[16];
private:
	short m_channelDelays[16];
};

#endif

