#ifndef __PSAUDIOSYSTEM_H__
#define __PSAUDIOSYSTEM_H__

#include "PsObject.h"
#include "PsMath.h"
#include "PsMixer.h"

class CPsOutputPortNULL : public CPsOutputPort
{
public:
	virtual const char* GetName();
	virtual void Reset();
	virtual void AllNoteOff();
	virtual void ReduceVoice();
	virtual int GetActiveCount();
	virtual void Send(unsigned int framePosition, void *pEvent);
	virtual void GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList);
};

class CPsAudioSystem : public CPsObject  
{
public:
	void Close();
	void Update();
	bool Open(CPsDriver *pDriver, int freq, int bufferSize);
	CPsAudioSystem();
	virtual ~CPsAudioSystem();

	CPsMixer* GetMixer(){
		return &m_mixer;
	}

	CPsDriver *GetDriver(){
		return m_pDriver;
	}

	CPsMath* GetMath(){
		return &m_math;
	}

	bool LockWaveOutDevice(){
		return GetDriver()->Lock();
	}

	void UnlockWaveOutDevice(){
		GetDriver()->Unlock();
	}

	CPsAudioConfig* GetConfig(){
		return &m_config;
	}

	CPsOutputPort **GetOutputPorts(){
		return m_outPors;
	}

	void AttachOutputPort(int id, CPsOutputPort *pPort){
		PsAssert(id >= 0 && id < MAX_OUTPUT_PORT);
		if(id >= 0 || id < MAX_OUTPUT_PORT)
			m_outPors[id] = pPort;
	}

	int GetOutputPortNumber(){
		return MAX_OUTPUT_PORT;
	}

	void DetachOutputPort(int id){
		PsAssert(id >= 0 && id < MAX_OUTPUT_PORT);
		if(id >= 0 || id < MAX_OUTPUT_PORT)
			m_outPors[id] = &m_nullOutPort;
	}

protected:
	CPsAudioConfig m_config;
	CPsMixer m_mixer;
	CPsDriver *m_pDriver;
	CPsMath m_math;
	CPsOutputPortNULL m_nullOutPort;
	CPsOutputPort *m_outPors[MAX_OUTPUT_PORT];
};

#endif
