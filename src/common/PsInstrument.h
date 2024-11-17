#ifndef __PSINSTRUMENT_H__
#define __PSINSTRUMENT_H__

#include "PsObject.h"
#include "PsOscillator.h"

class CPsSynthesizer;

class CPsOscPool : public CPsObject
{
public:
	enum {
#ifdef _CONSOLE
		MAX_POLYPHONY = 72
#else
		MAX_POLYPHONY = 72,
#endif
	};

	bool Open(CPsAudioSystem *p, int nPoly);
	void Close(CPsAudioSystem *p);
	void Reset(CPsAudioSystem *p);
	void CheckConsistancy();
	void ReduceVoice();
	void GetChannelVolumes(F16* pVolArray);
	int AllocateOscillator(int channel);

	int GetPolyNumber(){
		return m_nPoly;
	}

	int GetMaxPoly(){
		return MAX_POLYPHONY;
	}

	CPsOscillator* GetOscillator(int n){
		return m_osc + n;
	}

	signed char GetChannel(int n){
		return m_channel[n];
	}

	void SetChannel(int id, int channel){
		m_channel[id] = channel;
	}

	void SetPolyNumber(int n){
		if(n > MAX_POLYPHONY)
			n = MAX_POLYPHONY;
		m_nPoly = n;
	}

	int GetActiveCount(){
		return m_nActive;
	}

protected:
	int m_nPoly;
	int m_nActive;

	CPsOscillator m_osc[MAX_POLYPHONY];
	signed char m_channel[MAX_POLYPHONY];
};

class CPsInstrument : public CPsObject  
{
public:
	enum {
		F_HOLDPEDAL = 1
	};

	void SetHoldPedal(bool bHold);
	void SetPitchBand(F32 f);
	void AllNoteOff();
	void NoteOff(unsigned char note, F32 velocity, bool bForce);
	bool NoteOn(unsigned char note, F32 velocity);
	void Close();
	bool Open(CPsAudioSystem *p, CPsOscPool *pOscPool, int channel);
	void SetPatch(CPsPatch *p);
	void SetPan(F32 f);
	void SetVolume(F32 vol);
	void SetExpression(F32 f);
	void SetModulation(F32 f);

	CPsInstrument();
	virtual ~CPsInstrument();


	void SetPitchBandRange(F32 f){
		m_fPitchBandRange = f;
	}

	F32 GetPitchBandRange(){
		return m_fPitchBandRange;
	}

	F32 GetMasterTuning(){
		return m_par.m_fMasterTuning;
	}

	void SetReverb(F16 fReverb){
		m_par.m_fReverb = fReverb;
	}

	void SetChorus(F16 fChorus){
		m_par.m_fChorus = fChorus;
	}

	void SetKeyTranspose(signed char transpose){
		m_par.m_keyTranspose = transpose;
	}
	
	void SetBalance(F32 f)
	{
		m_par.m_fBalance = f;
	}
	
	void SetMasterTuning(F32 f)
	{
		m_par.m_fMasterTuning = f;
	}
	
	bool IsMute(){
		return m_par.m_isMute != 0;
	}

	void SetMute(bool isMute){
		m_par.m_isMute = isMute? 1: 0;
	}

protected:
	CPsPatch *m_pPatch;
	CPsAudioSystem *m_pSystem;

	signed char m_channel;
	unsigned char m_flag;

	CPsOscillator::PARAMETERS m_par;
	F16 m_fVolume, m_fExpression;
	F32 m_fPitchBand, m_fPitchBandRange;
	CPsOscPool *m_pOscPool;
};

#endif
