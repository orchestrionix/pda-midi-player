#include "PsWinDriver.h"
#include "common/PsAudioSystem.h"
#include "PsSys.h"

CPsWinDriver::CPsWinDriver():
	m_bufferedFramePosition(0)
{
	m_nBuffers = 0;
	m_nFramesPerBuffer = 0;
	m_hwaveout = NULL;
	m_header = NULL;
	m_buffer = NULL;
	m_buffersout = 0;
	m_nextbuffer = 0;
	m_initialized = false;
}

CPsWinDriver::~CPsWinDriver()
{
	Close();
}

bool CPsWinDriver::Open(int freq, int nBuffers, int nFramesPerBuffer, bool isStereo)
{
	int				n;

	m_nChannel = isStereo ? 2 : 1;
	m_nBuffers = nBuffers;
	m_header = new WAVEHDR[m_nBuffers];
	if(!m_header)
		return false;
	m_buffer = new NativeSample*[m_nBuffers];
	if(!m_buffer)
	{
		Close();
		return false;
	}

	CPsSys::memset(m_buffer, 0, sizeof(NativeSample*) * m_nBuffers);

	m_wfe.wFormatTag=WAVE_FORMAT_PCM;
	m_wfe.nChannels= m_nChannel;
	m_wfe.nSamplesPerSec= freq;
	m_wfe.nAvgBytesPerSec= freq*sizeof(NativeSample)*m_nChannel;
	m_wfe.nBlockAlign=sizeof(NativeSample)*m_nChannel;
	m_wfe.wBitsPerSample=sizeof(NativeSample)*8;
	m_wfe.cbSize=sizeof(m_wfe);

//	PsAssert(nSamplesPerBuffer % pSystem->GetSamplesPerUpdate() == 0);
	m_nFramesPerBuffer = nFramesPerBuffer;

	for (n=0;n<m_nBuffers;n++) {
		m_buffer[n]=new NativeSample[m_nFramesPerBuffer * m_nChannel];
		m_header[n].lpData= (char*)m_buffer[n];
		m_header[n].dwBufferLength= m_nFramesPerBuffer * m_nChannel * sizeof(NativeSample);
		m_header[n].dwFlags = 0;
		if (!m_buffer[n]) {
			Close();
			return false;
		}
	}

	m_initialized = true;
/*	if(!Lock())
	{
		m_initialized = false;
		return false;
	}
*/	
	return true;
}

void CALLBACK CPsWinDriver::WIN_CallBack(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg==WOM_DONE) --((CPsWinDriver*)dwInstance)->m_buffersout;
}

void CPsWinDriver::Update(CPsGenerator *pMixer)
{
	while (m_buffersout<m_nBuffers) {
		unsigned int todo = m_nFramesPerBuffer;
		NativeSample *p = m_buffer[m_nextbuffer];
		while(todo > 0)
		{
			unsigned int written = pMixer->Write(p);
			m_bufferedFramePosition += written;
			todo -= written;
			p += written * m_nChannel;
		}
		PsAssert(todo == 0);
		m_header[m_nextbuffer].dwBufferLength = m_nFramesPerBuffer * sizeof(NativeSample) * m_nChannel;
		waveOutWrite(m_hwaveout,&m_header[m_nextbuffer],sizeof(WAVEHDR));
//		fwrite(m_buffer[m_nextbuffer], sizeof(SAMPLE) * m_nChannel, m_nFramesPerBuffer, m_hFileOut);
		if (++m_nextbuffer>=m_nBuffers) m_nextbuffer -=m_nBuffers;
		++m_buffersout;
	}
}

void CPsWinDriver::Close()
{
	int n;
	if(m_hwaveout)
	{
		waveOutReset(m_hwaveout);
		waveOutClose(m_hwaveout);
		m_hwaveout = NULL;
	}
	if(m_buffer)
	{
		for (n=0;n<m_nBuffers;n++) {
				delete[] m_buffer[n];
		}
		delete[] m_buffer;
		m_buffer = NULL;
	}
	delete[] m_header;
	m_header = NULL;

	m_initialized = false;
}

WAVEHDR* CPsWinDriver::GetPlayingBuffer()
{
	if(m_buffersout == 0)return NULL;
	return &m_header[(m_nextbuffer + (m_nBuffers - m_buffersout)) % m_nBuffers];
}

bool CPsWinDriver::Lock()
{
	int n;
	if(!m_initialized)
		return false;
	if(m_hwaveout)
		return true;
	MMRESULT mmr=waveOutOpen(&m_hwaveout, WAVE_MAPPER, &m_wfe,( DWORD)WIN_CallBack, (DWORD)this, CALLBACK_FUNCTION);
	if(mmr != MMSYSERR_NOERROR)
		return false;

	m_tick = GetTickCount();

	for (n = 0; n < m_nBuffers; n++) {
		m_header[n].dwFlags = 0;
		mmr = waveOutPrepareHeader(m_hwaveout, &m_header[n], sizeof(WAVEHDR));
		if (mmr != MMSYSERR_NOERROR) {
			Unlock();
			return false;
		}
	}
	
	m_bufferedFramePosition = 0;
	m_buffersout=m_nextbuffer=0;
	return true;
}

void CPsWinDriver::Unlock()
{
	if(!m_initialized)
		return;
	if(m_hwaveout)
	{
		waveOutReset(m_hwaveout);
#ifndef _WIN32_WCE_EMULATION
		waveOutClose(m_hwaveout);
#endif
		m_hwaveout = NULL;
		for (int n=0;n<m_nBuffers;n++) {
			if(m_buffer[n])
				CPsSys::memset(m_buffer[n], 0, sizeof(NativeSample) * m_nFramesPerBuffer * m_nChannel);
		}
	}
}

unsigned int CPsWinDriver::GetPlayingFramePosition()
{
	if(m_hwaveout)
	{
		MMTIME t;
		t.wType = TIME_SAMPLES;
		waveOutGetPosition(m_hwaveout, &t, sizeof(t));
		return t.u.sample;
	}
	else
		return 0;
}

unsigned int CPsWinDriver::GetBufferedFramePosition()
{
	return m_bufferedFramePosition;
}
