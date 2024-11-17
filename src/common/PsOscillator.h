#ifndef __PSOSCILLATOR_H__
#define __PSOSCILLATOR_H__

#include "PsObject.h"
#include "PsPatchWave.h"
#include "PsEG.h"
#include "PsVolEG.h"
#include "PsLFO.h"
#include "PsMath.h"

class CPsOscillator : public CPsObject, public CPsChannel
{
public:
	enum {
		F_HOLDPADEL = 1,
		F_SUSTAINING = 2,
		F_ENABLE_LFO = 4,
		F_NOTE_OFF = 8
	};
	enum {
		CUTDOWN_SUSTAIN_VOLUME = F_ONE / 64
	};
	
	typedef struct {
		F32 m_fModulation;
		F32 m_fAttenuation;
		F32 m_fBalance;
		F32 m_fPitchBand;
		F32 m_fMasterTuning;
		F16 m_fChorus, m_fReverb;
		F16 m_fPan;
		char m_keyTranspose;
		char m_isMute;
	}PARAMETERS;

	void SetState(int state);
	int GetState();
	void SetHoldPedal(bool bHold);
	F32 GetBalance();
	F32 GetPan();
	F32 GetMixFrequency();
	F32 GetMixVolume();
	const CPsSampleData* GetSampleData();

	void NoteOn(int key, F16 velocity);
	bool NoteOff(int key, F16 velocity, bool bForce);
	void Update();
	void SetWave(CPsPatch::PSWAVEINFO *pInfo, CPsPatchWave *pWave);
	void Open(CPsAudioSystem *pSystem);
	void SetArticulation(CPsPatch::PSARTICULATION *pArt);
	
	CPsOscillator();
	virtual ~CPsOscillator();

	void CutOff(){
		m_volEG.CutOff();
	}

	int GetVolEGState(){
		return m_volEG.GetState();
	}

	void SetPatch(CPsPatch *p){
		m_pPatch = p;
	}

	int GetCurrentKey(){
		return m_key;
	}

	F32 GetMaxVolume(){
		return m_fMaxVolume;
	}

	virtual F16 GetReverb(){
		return m_pPar->m_fReverb;
	}

	virtual F16 GetChorus(){
		return m_pPar->m_fChorus;
	}

	bool IsNoteOff(){
		return (m_flag & F_NOTE_OFF) != 0;
	}

	void SetParameters(PARAMETERS *p){
		m_pPar = p;
		m_lfo.SetModWheel(m_pPar->m_fModulation);
		m_fPan = m_pPar->m_fPan;
		PsAssert(m_fPan >= -F_ONE/2 && m_fPan <= F_ONE/2);
	}

	void UpdateModWheel(){
		m_lfo.SetModWheel(m_pPar->m_fModulation);
		m_lfo.UpdateModWheel();
	}
protected:
	CPsMath *m_pMath;
	CPsAudioConfig *m_pConfig;

	CPsSampleData m_sampleData;

	CPsPatch *m_pPatch;
	CPsPatch::PSARTICULATION *m_pArt;
	CPsPatch::PSWAVEINFO *m_pWaveInfo;
	CPsPatchWave *m_pWaveData;
	CPsLFO m_lfo;
	CPsEG m_pitchEG;
	CPsVolEG m_volEG;

	F32 m_wavePitch;
	F32 m_notePitch;

	F32 m_fWaveSamplePitch;
	F32 m_fEG2Pitch;

	PARAMETERS *m_pPar;

	unsigned char m_state;
	unsigned char m_flag;
	signed char m_key;

	F16 m_fPan;
	F16 m_keyOnVel;
	F32 m_fRelativeFreq;
	F32 m_fMixVolume;
	F32 m_fMaxVolume;
};

#endif
