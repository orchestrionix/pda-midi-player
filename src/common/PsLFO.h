// PsLFO.h: interface for the CPsLFO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSLFO_H__1A73A897_7F56_40AF_B644_B617F10C9755__INCLUDED_)
#define AFX_PSLFO_H__1A73A897_7F56_40AF_B644_B617F10C9755__INCLUDED_

#include "PsMath.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PsObject.h"
#include "PsPatch.h"
#include "PsMath.h"

class CPsLFO : public CPsObject  
{
public:
	void UpdateModWheel();
	void Reset();
	F32 GetPitchScale();
	F32 GetVolumeScale();
	void Update();
	void TriggerOn();
	void SetParameters(CPsPatch::PSLFO *pLFO, CPsMath *pMath, F32 fUpdateFreq);
	CPsLFO();
	virtual ~CPsLFO();

	void SetModWheel(F32 mw) {
		m_fModulation = mw;
	}

protected:
	CPsPatch::PSLFO *m_pLFO;
	F32 m_fStep, m_fCurPos, m_fInitPos;
	F32 m_fCurValue;
	F32 m_fVolumeScale, m_fPitchScale;
	F32 m_fModulation;
	CPsMath* m_pMath;
};

#endif // !defined(AFX_PSLFO_H__1A73A897_7F56_40AF_B644_B617F10C9755__INCLUDED_)
