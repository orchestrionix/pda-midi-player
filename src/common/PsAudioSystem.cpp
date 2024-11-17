#include "PsAudioSystem.h"

void CPsOutputPortNULL::Send(unsigned int framePosition, void *pEvent)
{
}

const char* CPsOutputPortNULL::GetName()
{
	return "NULL";
}

void CPsOutputPortNULL::Reset()
{
}

void CPsOutputPortNULL::AllNoteOff()
{
}

void CPsOutputPortNULL::ReduceVoice()
{
}

int CPsOutputPortNULL::GetActiveCount()
{
	return 0;
}

void CPsOutputPortNULL::GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList)
{
}

void CPsAudioConfig::SetMixFrequency(CPsMath *pMath, int freq, int nFramePerUpdate){
	m_nMixFrequency = freq;
	m_fMixPitch = pMath->AbsFreq2FixedPitch(I2F(m_nMixFrequency));

	if(nFramePerUpdate == 0)
	{
		m_nFramePerUpdate = freq / 100;
		m_nFramePerUpdate = (m_nFramePerUpdate + 3) & ~3;
	}
	else
	{
		PsAssert(nFramePerUpdate % 4 == 0);
		m_nFramePerUpdate = nFramePerUpdate;
	}
		
	m_fUpdateFrequency = I2F(m_nMixFrequency) / m_nFramePerUpdate;
}

CPsAudioSystem::CPsAudioSystem()
{
	for(int i = 0; i < MAX_OUTPUT_PORT; i++)
		m_outPors[i] = &m_nullOutPort;
}

CPsAudioSystem::~CPsAudioSystem()
{

}

bool CPsAudioSystem::Open(CPsDriver *pDriver, int freq, int bufferSize)
{
	m_pDriver = pDriver;
	m_config.SetMixFrequency(GetMath(), freq, 0);
	int bufferSample = bufferSize * freq / 1000;
	int n = m_config.GetFramesPerUpdate() * 8;

	if(m_pDriver->Open(m_config.GetMixFrequency(), bufferSample / n, n, true))
	{
		if(m_mixer.Open(this, m_config.GetFramesPerUpdate(), true))
		{
			return true;
		}
		m_pDriver->Close();
	}
	return false;
}

void CPsAudioSystem::Update()
{
	m_pDriver->Update(&m_mixer);
}

void CPsAudioSystem::Close()
{
	m_mixer.Close();
	m_pDriver->Close();
	for(int i = 0; i < MAX_OUTPUT_PORT; i++)
		m_outPors[i] = &m_nullOutPort;
}
