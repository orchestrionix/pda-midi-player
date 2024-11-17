#ifndef __PSPATCH_H__
#define __PSPATCH_H__

#include "PsObject.h"

class CPsPatch : public CPsObject  
{
public:

	enum {
		INSTRUMENT_DRUMS = 0x80000000,

		//for PSREGION.options
		REGION_SELFNONEXCLUSIVE = 1,
		REGION_OVERWRITESAMPLE = 2,

		//for PSSAMPLEINFO.loopType
		SAMPLE_LOOPNONE = 0,
		SAMPLE_LOOPFORWARD = 1,

		//for PSARTICULATION.options
		ART_LFO = 1,
		ART_EG1 = 2,
		ART_EG2 = 4
	};

	typedef struct
	{
		unsigned int waveLink;
		int pitch;
		int attenuation;
		unsigned short maxLoopAmplitude;
		unsigned short loopType;
		unsigned int loopStart;
		unsigned int loopLength;
	}PSWAVEINFO;
	
	typedef struct
	{
		unsigned char minKey, maxKey;
		unsigned char minVelocity, maxVelocity;
		unsigned char options;
		unsigned char group;
		PSWAVEINFO waveInfo;
	}PSREGION;
	
	typedef struct
	{
		int attack;
		int decay;
		int release;
		int sustain;
		int velocityToAttack;
		int keyToDecay;
	}PSENVELOPE;
	
	typedef struct
	{
		int pitch;
		int	startDelay;
		int volumeScale;
		int pitchScale;
		int MWToAttenuation;
		int MWToPitch;
	}PSLFO;
	
	typedef struct
	{
		unsigned int options;
		int initialPan;
		int EG2ToPitch;
		PSLFO Lfo;
		PSENVELOPE Env[2];
	}PSARTICULATION;
	
	CPsPatch::PSREGION* FindRegion(int note, int &id);
	bool Load(CPsReader *pReader, unsigned int bank_id, unsigned int prog_id);
	void Free();
	PSREGION* GetRegion(int id);
	CPsPatch();
	virtual ~CPsPatch();

	bool IsDrum(){
		return (m_bank & 0x80) != 0;
	}

	int GetRegionNumber(){
		return m_nRegion;
	}

	PSARTICULATION* GetArticulation(int i){
		return (PSARTICULATION*)m_pData + i;
	}

	int GetProgram(){
		return m_prog;
	}

protected:
	unsigned char m_bank, m_subBank;
	unsigned char m_prog;

	unsigned char m_nRegion;
	unsigned char *m_pData;
	char m_szName[48];
};

#endif
