#ifndef __PSSOUNDBANK_H__
#define __PSSOUNDBANK_H__

#include "PsObject.h"
#include "PsPatch.h"
#include "PsPatchWave.h"

class CPsSoundBank : public CPsObject  
{
public:
	void FreeAllPatchs();
	CPsPatch* LoadPatch(CPsMath *pMath, unsigned int bank, unsigned int prog, unsigned char *pRgnMap);
	void FreePatch(CPsPatch* p);
	bool Load(CPsReader *pReader);
	void Free();

	CPsSoundBank();
	virtual ~CPsSoundBank();

protected:
	bool UpdateWaveLink(CPsMath *pMath, int instId, unsigned char *pRgnMap);
	int FindPatch(unsigned int bank, unsigned int prog);
	CPsPatchWave* LoadWave(CPsMath *pMath, int id);
	void FreeWave(CPsPatchWave* pSample);

	unsigned int m_nPatch, m_nWave;
	unsigned int *m_pPatchMap;
	CPsPatch **m_pPatchs;
	unsigned short *m_pPatchRefs;
	CPsPatchWave **m_pWaves;
	unsigned short *m_pWaveRefs;
	CPsReader *m_pSBReader;
	unsigned int *m_pOffsets;
};

#endif
