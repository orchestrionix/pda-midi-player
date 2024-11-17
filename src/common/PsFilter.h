#ifndef __PSFILTER_H__
#define __PSFILTER_H__

#include "PsObject.h"
#include "PsMath.h"

class CPsFilterLowPass : public CPsObject
{
public:
	void ProcessStereo(int *pS, int count);
	void InitLowPass();
	void SetParameters(CPsMath* pMath, int cutOff, F32 fGain, int sampleRate);
	
protected:
	F16 m_k;
	F32 m_fGain;
	int m_ul, m_xl, m_ur, m_xr;
};

class CPsFilterLHPass : public CPsObject
{
public:
	void InitHiPass(CPsMath* pMath, int cutOff, F32 rez, int sampleRate);
	void Process(int *pS, int *pD, int count, int step);
	void InitLowPass(CPsMath* pMath, int cutOff, F32 rez, int sampleRate);
	CPsFilterLHPass();
	virtual ~CPsFilterLHPass();
	
protected:
	int m_in[2], m_out[2];
	F32 m_c, m_a[3], m_b[2];
};

class CPsFilterSurround : public CPsObject
{
public:
	enum {
		SURROUNDBUFFERSIZE = 2048
	};
	void Init(CPsMath* pMath, int level, unsigned int nDelay, int sampleRate);
	void Process(int *pS, int count);
	CPsFilterSurround();
	virtual ~CPsFilterSurround();
	
protected:
	int m_SurroundBuffer[SURROUNDBUFFERSIZE];
	int m_nSurroundSize;
	int m_nSurroundPos;
	long m_nDolbyHP_A1, m_nDolbyHP_B0, m_nDolbyHP_B1, m_nDolbyHP_X1, m_nDolbyHP_Y1;
	long m_nDolbyLP_A1, m_nDolbyLP_B0, m_nDolbyLP_B1, m_nDolbyLP_X1, m_nDolbyLP_Y1;
};

#endif
