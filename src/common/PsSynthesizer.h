#ifndef __PSSYNTHESIZER_H__
#define __PSSYNTHESIZER_H__

#include "PsObject.h"
#include "PsInstrument.h"
#include "PsSoundBank.h"

class CPsSynthesizer : public CPsObject
{
public:
	enum {
		TOTAL_INSTRUMENT = 16,
	};

	enum {
		F_ENABLED = 1,
		F_DRUM = 2
	};

	void SetFineDataEntry(int channel, int d);
	void SetCoarseDataEntry(int channel, int d);
	void SetCoarseChorus(int channel, int chorus);
	void SetCoarseReverb(int channel, int reverb);
	void ReduceVoice();
	void SetHoldPedal(int channel, int n);
	void SetPitchBand(int channel, int n);
	void GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList);
	bool Open(CPsAudioSystem *pSystem, CPsReader *pFile);
	void Close();
	void Reset();
	void ResetRPN(int channel);
	void SetProgram(int channel, int prog);
	void ResetAllControllers(int channel);
	void AllNoteOff(int channel);
	void SetCoarseModulation(int channel, int m);
	void SetCoarseVolume(int channel, int volume);
	void SetCoarseExpression(int channel, int volume);
	void SetCoarsePan(int channel, int pan);
	void SetCoarseBalance(int channel, int balance);
	void NoteOff(int channel, int note, int velocity);
	bool NoteOn(int channel, int note, int velocity);
	void SetTranspose(int nKey);
	CPsSynthesizer();
	virtual ~CPsSynthesizer();

	void SetCoarseBank(int channel, int b){
		m_bank[channel][1] = b;
	}

	void SetFineBank(int channel, int b){
		m_bank[channel][0] = b;
	}

	CPsOscPool* GetOscPool(){
		return &m_oscPool;
	}

	int GetActiveCount(){
		return m_oscPool.GetActiveCount();
	}

	void SetCoarseRPN(int channel, int n)
	{
		m_RPN[channel] = (m_RPN[channel] & 0x7f) | (n << 7);
	}
	
	void SetFineRPN(int channel, int n)
	{
		m_RPN[channel] = (m_RPN[channel] & 0x3f80) | n;
		if(m_RPN[channel] == 0x3fff)
			ResetRPN(channel);
	}

	int GetTranspose() {
		return m_keyTranspose;
	}
	
	void SetMute(int channel, bool isMute){
		m_inst[channel].SetMute(isMute);
	}

	bool IsMute(int channel){
		return m_inst[channel].IsMute();
	}

	int GetProgram(int channel){
		return m_prog[channel];
	}

	const unsigned char *GetNoteState(){
		return m_noteState[0];
	}

	void SetDrumChannel(int channel, bool isDrum){
		if(isDrum)
			m_flags[channel] |= F_DRUM;
		else
			m_flags[channel] &= ~F_DRUM;
	}

protected:
	void LoadProgram(int channel);
	CPsReader *m_pSbFile;
	CPsAudioSystem *m_pSystem;
	CPsInstrument m_inst[TOTAL_INSTRUMENT];
	unsigned char m_flags[TOTAL_INSTRUMENT];
	CPsSoundBank m_sb;
	CPsPatch *m_pPatch[TOTAL_INSTRUMENT];
	unsigned char m_bank[TOTAL_INSTRUMENT][2];
	unsigned char m_prog[TOTAL_INSTRUMENT];
	unsigned char m_noteState[TOTAL_INSTRUMENT][128];

	CPsOscPool m_oscPool;

	unsigned short m_RPN[TOTAL_INSTRUMENT];
	char m_keyTranspose;
};

#endif
