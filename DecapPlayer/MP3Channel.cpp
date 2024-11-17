#include "common/PsAudioSystem.h"
#include "common/PsPatch.h"
#include "Mp3Channel.h"
#include <win32/PsSys.h>

CMP3Channel::CMP3Channel():
	m_state(STATE_STOPPED)
{
	CPsSys::memset(&m_stream, 0, sizeof(m_stream));
	CPsSys::memset(&m_frame, 0, sizeof(m_frame));
	CPsSys::memset(&m_synth, 0, sizeof(m_synth));
	m_fMixVolume = F_ONE - 1;
}

CMP3Channel::~CMP3Channel()
{
	Close();
}

bool CMP3Channel::Open(CPsAudioSystem *pSystem, LPCTSTR pMP3File)
{
	if(!m_file.Open(pMP3File, CPsFileReader::F_READ))
		return false;
	m_pConfig = pSystem->GetConfig();
	mad_stream_init(&m_stream);
	mad_frame_init(&m_frame);
	mad_synth_init(&m_synth);
	m_sampleData.loopLength = 0;
	m_sampleData.loopStart = 0;
	m_sampleData.loopType = CPsPatch::SAMPLE_LOOPNONE;
	m_sampleData.nBitPerSample = sizeof(m_sample[0]) * 8;
	m_sampleData.nChannel = 2;
	m_sampleData.pData = (unsigned char*)m_sample;
	m_fMixPosition = 0;
	m_lastSamplePos = 0;
	m_fLastLVol = 0;
	m_fLastRVol = 0;
	m_totalDecoded = 0;
	m_fResamplePos = 0;
	m_lastSample[0] = m_lastSample[1] = 0;
	m_state = STATE_PLAYING;

	return true;
}

#define MAKEFOURCCLE(ch0, ch1, ch2, ch3)                              \
		((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) |   \
		((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))

#define XING_FRAMES     0x01
#define XING_BYTES		0x02
#define XING_TOC		0x04
#define XING_SCALE		0x08

int CMP3Channel::GetTotalTime()
{
/*
	//Get song length
    mad_timer_t timer;
    struct xing xing;

    m_totalTime = 20 * 60 * 1000; //use 20 min when failed to get song length
    xing_init(&xing);
    xing_parse(&xing, m_stream.anc_ptr, m_stream.anc_bitlen);

    if (xing.flags & XING_FRAMES) {
	timer = m_frame.header.duration;
	mad_timer_multiply(&timer, xing.frames);
	m_totalTime = mad_timer_count(timer, MAD_UNITS_MILLISECONDS);
    } else {
		if (m_frame.header.bitrate)
			m_totalTime = m_file.GetLength() * 8 / m_frame.header.bitrate * 1000;
    }
	return m_totalTime;
*/
	unsigned int total_frames = 0;
	m_totalTime = 0;
	for (int i=0; i<2048; ++i)
		if ((unsigned char)m_file.ReadChar() == 0xFF)
		{
			unsigned int Frame;
			int SampleRate,Id,Mode,Layer,SamplePerFrame;
			static const int RateTable[4] = { 44100, 48000, 32000, 99999 };

			i = (unsigned char)m_file.ReadChar();
			if ((i & 0xE0) != 0xE0)
				continue;

			Id = (i >> 3) & 3;
			Layer = (i >> 1) & 3;
			SampleRate = RateTable[((unsigned char)m_file.ReadChar() >> 2) & 3];
			if (Id==2)
				SampleRate >>= 1; // MPEG2
			if (Id==0)
				SampleRate >>= 2; // MPEG2.5
			Mode = ((unsigned char)m_file.ReadChar() >> 6) & 3;

			SamplePerFrame = (Layer == 3)?384:1152;

			Frame = m_file.GetPosition();

			//Xing offset
			if (Id==3)
			{
				// MPEG1
				m_file.SetPosition(Mode==3 ? 17:32, CPsFileReader::REF_CURRENT);
			}
			else
			{
				// MPEG2/2.5
				m_file.SetPosition(Mode==3 ? 9:17, CPsFileReader::REF_CURRENT);
				if (Layer == 1) // layer-3
					SamplePerFrame = 576;
			}

			if (m_file.ReadInt() == MAKEFOURCCLE('X','i','n','g'))
			{
				int Flags = m_file.ReadIntBE();
				if (Flags & XING_FRAMES) 
					total_frames = m_file.ReadIntBE();
			}
			else
			{
				m_file.SetPosition(Frame+32, CPsFileReader::REF_BEGIN);

				if (m_file.ReadInt() == MAKEFOURCCLE('V','B','R','I'))
				{
					m_file.SetPosition(2 + 2 + 2 + 4, CPsFileReader::REF_CURRENT); //Version,Delay,Quality, Bytes
					total_frames = m_file.ReadIntBE();
				}
			}

			if (total_frames > 0)
				m_totalTime = (unsigned int)((F64)total_frames * SamplePerFrame * 1000 / SampleRate);
			break;
		}
	if(m_totalTime == 0)
	{
		m_file.SetPosition(0, CPsFileReader::REF_BEGIN);
		Update();
		if (m_frame.header.bitrate)
			m_totalTime = m_file.GetLength() * 8 / m_frame.header.bitrate * 1000;
	}
	m_file.SetPosition(0, CPsFileReader::REF_BEGIN);
	return m_totalTime;
}

void CMP3Channel::Close()
{
	mad_synth_finish(&m_synth);
	mad_frame_finish(&m_frame);
	mad_stream_finish(&m_stream);
	m_file.Close();
	m_state = STATE_STOPPED;
}

static inline NativeSample MadFixedToNativeSample(mad_fixed_t Fixed)
{
	Fixed >>= MAD_F_FRACBITS - 15;
	return NATIVE_SAMPLE_SAT(Fixed);
}

void CMP3Channel::Update()
{
	const int nChannel = 2;
	if(m_state != STATE_PLAYING)
		return;
	int startSample = F2I(m_fMixPosition);
	if(m_lastSamplePos - startSample <= MAX_OUTPUT_SAMPLE_PER_UPDATE)
	{
		memmove(m_sample, m_sample + startSample * nChannel, (m_lastSamplePos - startSample) * sizeof(m_sample[0]) * nChannel);
		m_lastSamplePos -= startSample;
		startSample = 0;
		m_fMixPosition &= F_ONE - 1;
	}
	while(m_lastSamplePos <= MAX_OUTPUT_SAMPLE_PER_UPDATE)
	{
		if(m_stream.buffer == NULL || m_stream.error == MAD_ERROR_BUFLEN)
		{
			int readSize, remaining;
			unsigned char *readStart;

			if(m_stream.next_frame)
			{
				remaining = m_stream.bufend - m_stream.next_frame;
				memmove(m_inputBuffer, m_stream.next_frame, remaining);
			}
			else
				remaining = 0;
			readSize = sizeof(m_inputBuffer) - remaining;
			readStart = m_inputBuffer + remaining;
			int readCount = m_file.ReadBlock(readStart, readSize);
			if(readCount == 0)
			{
				if(m_lastSamplePos == 0)
					m_state = STATE_STOPPED;
				break;
			}
			if(readCount < readSize)
			{
				memset(readStart + readCount, 0, MAD_BUFFER_GUARD);
				readCount += MAD_BUFFER_GUARD;
			}
			mad_stream_buffer(&m_stream,m_inputBuffer, readCount + remaining);
			m_stream.error = MAD_ERROR_NONE;
		}
		if(mad_frame_decode(&m_frame, &m_stream))
		{
			if(MAD_RECOVERABLE(m_stream.error) || m_stream.error == MAD_ERROR_BUFLEN)
				continue;
			else
			{
				m_state = STATE_STOPPED;
				break;
			}
		}
		m_frame.options = MAD_OPTION_IGNORECRC;
		if(m_frame.header.samplerate >= m_pConfig->GetMixFrequency() * 2)
			m_frame.options |= MAD_OPTION_HALFSAMPLERATE;
		mad_synth_frame(&m_synth,&m_frame);
		F32 fStep = I2F(m_synth.pcm.samplerate) / m_pConfig->GetMixFrequency();
		int writepos = m_lastSamplePos * nChannel;
		if(MAD_NCHANNELS(&m_frame.header) == 2)
		{
			F32 fPos = m_fResamplePos;
			while(true)
			{
				int wholePart = F2I(fPos);
				if(wholePart >= m_synth.pcm.length)
					break;
				int last[2];
				if(wholePart < 1)
				{
					last[0] = m_lastSample[0];
					last[1] = m_lastSample[1];
				}
				else
				{
					last[0] = MadFixedToNativeSample(m_synth.pcm.samples[0][wholePart - 1]);
					last[1] = MadFixedToNativeSample(m_synth.pcm.samples[1][wholePart - 1]);
				}
				m_sample[writepos++] = last[0] + F2I((MadFixedToNativeSample(m_synth.pcm.samples[0][wholePart]) - last[0]) * (fPos & (F_ONE - 1)));
				m_sample[writepos++] = last[1] + F2I((MadFixedToNativeSample(m_synth.pcm.samples[1][wholePart]) - last[1]) * (fPos & (F_ONE - 1)));
				fPos += fStep;
			}
			m_lastSample[0] = MadFixedToNativeSample(m_synth.pcm.samples[0][m_synth.pcm.length - 1]);
			m_lastSample[1] = MadFixedToNativeSample(m_synth.pcm.samples[1][m_synth.pcm.length - 1]);
			m_fResamplePos = fPos - fStep * m_synth.pcm.length;
		}
		else
		{
			F32 fPos = m_fResamplePos;
			while(true)
			{
				int wholePart = F2I(fPos);
				if(wholePart >= m_synth.pcm.length)
					break;
				int last[1];
				if(wholePart < 1)
				{
					last[0] = m_lastSample[0];
				}
				else
				{
					last[0] = MadFixedToNativeSample(m_synth.pcm.samples[0][wholePart - 1]);
				}
				last[0] += F2I((MadFixedToNativeSample(m_synth.pcm.samples[0][wholePart]) - last[0]) * (fPos & (F_ONE - 1)));
				m_sample[writepos++] = last[0];
				m_sample[writepos++] = last[0];
				fPos += fStep;
			}
			m_lastSample[0] = MadFixedToNativeSample(m_synth.pcm.samples[0][m_synth.pcm.length - 1]);
			m_fResamplePos = fPos - fStep * m_synth.pcm.length;
//			for(i = 0; i < m_synth.pcm.length; i++)
//			{
//				NativeSample s = MadFixedToNativeSample(m_synth.pcm.samples[0][i]);
//				m_sample[writepos++] = s;
//				m_sample[writepos++] = s;
//			}
		}
		m_lastSamplePos = writepos / nChannel;
		m_totalDecoded += m_synth.pcm.length;
	}
	m_sampleData.loopStart = m_fMixPosition;
	m_sampleData.loopLength = m_fMixPosition + I2F64(m_lastSamplePos);
}

F32 CMP3Channel::GetMixVolume()
{
	return m_fMixVolume;
}

F32 CMP3Channel::GetMixFrequency()
{
	return F_ONE;
}

F32 CMP3Channel::GetPan()
{
	return 0;
}

F32 CMP3Channel::GetBalance()
{
	return 0;
}

int CMP3Channel::GetState()
{
	return m_state;
}

void CMP3Channel::SetState(int state)
{
	m_state = state;
}

const CPsSampleData* CMP3Channel::GetSampleData()
{
	return &m_sampleData;
}

F16 CMP3Channel::GetReverb(){
	return 0;
}

F16 CMP3Channel::GetChorus(){
	return 0;
}
