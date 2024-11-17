#ifndef __PSWINDRIVER_H__
#define __PSWINDRIVER_H__

#include "common/PsObject.h"
#include "common/PsMixer.h"

class CPsWinDriver : public CPsDriver
{
public:
	bool Open(int freq, int nBuffers, int nFramesPerBuffer, bool isStereo);
	void Close();
	bool Lock();
	void Unlock();
	void Update(CPsGenerator *pMixer);
	virtual unsigned int GetPlayingFramePosition();
	virtual unsigned int GetBufferedFramePosition();
	WAVEHDR * GetPlayingBuffer();
	CPsWinDriver();
	virtual ~CPsWinDriver();

	int GetChannelCount() {
		return m_nChannel;
	}

	int GetTrueFrequency(){
		int n = GetTickCount() - m_tick;
		if(n < 1)
			n = 1;
		return GetPlayingFramePosition() * 1000 / n;
	}

	bool Pause() {
		if(m_hwaveout)
		{
			return waveOutPause(m_hwaveout) == MMSYSERR_NOERROR;
		}
		return false;
	}

	bool Resume() {
		if(m_hwaveout)
		{
			return waveOutRestart(m_hwaveout) == MMSYSERR_NOERROR;
		}
		return false;
	}

protected:
	static void CALLBACK WIN_CallBack(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);

	int m_nBuffers; /* number of buffers */
	int m_nChannel;
	int m_tick;

	bool m_initialized;

	WAVEFORMATEX	m_wfe;
	HWAVEOUT	m_hwaveout;
	WAVEHDR		*m_header;
	NativeSample		**m_buffer;		/* pointers to buffers */
	short		m_buffersout;				/* number of buffers playing/about to be played */
	short		m_nextbuffer;				/* next buffer to be mixed */
	int			m_nFramesPerBuffer;	/* buffer size in frame */
	unsigned int m_bufferedFramePosition;
};

#endif
