#include "PsAudioSystem.h"
#include "PsReverbLPHP.h"
#include "Win32/PsSys.h"

#define REV_VAL0        234
#define REV_VAL1        463
#define REV_VAL2        1946
#define REV_VAL3        926

#define REV_INP_LEV      PSFLOAT2F(0.55)
#define REV_FBK_LEV      PSFLOAT2F(0.12)

#define REV_NMIX_LEV     PSFLOAT2F(0.7)

#define REV_HPF_LEV      PSFLOAT2F(0.5)
#define REV_LPF_LEV      PSFLOAT2F(0.45)
#define REV_LPF_INP      PSFLOAT2F(0.55)
#define REV_EPF_LEV      PSFLOAT2F(0.4)
#define REV_EPF_INP      PSFLOAT2F(0.48)

#define REV_WIDTH        PSFLOAT2F(0.125)

CPsReverbLPHP::CPsReverbLPHP()
{
}

CPsReverbLPHP::~CPsReverbLPHP()
{
	Shutdown();
}

void CPsReverbLPHP::SetSampleRate(int rate)
{
    rpt0 = rate * REV_VAL0 / 44100;
    rpt1 = rate * REV_VAL1 / 44100;
    rpt2 = rate * REV_VAL2 / 44100;
    rpt3 = rate * REV_VAL3 / 44100;
}

bool CPsReverbLPHP::Init(CPsAudioSystem *pSystem)
{
    ta = 0; tb = 0;
    HPFL = 0; HPFR = 0;
    LPFL = 0; LPFR = 0;
    EPFL = 0; EPFR = 0;
    spt0 = 0; spt1 = 0;
    spt2 = 0; spt3 = 0;

	CPsSys::memset(buf0_L, 0, sizeof(buf0_L));
	CPsSys::memset(buf0_R, 0, sizeof(buf0_R));
	CPsSys::memset(buf1_L, 0, sizeof(buf1_L));
	CPsSys::memset(buf1_R, 0, sizeof(buf1_R));
	CPsSys::memset(buf2_L, 0, sizeof(buf2_L));
	CPsSys::memset(buf2_R, 0, sizeof(buf2_R));
	CPsSys::memset(buf3_L, 0, sizeof(buf3_L));
	CPsSys::memset(buf3_R, 0, sizeof(buf3_R));
	
	SetSampleRate(pSystem->GetConfig()->GetMixFrequency());
	return true;
}

void CPsReverbLPHP::Shutdown()
{
}

void CPsReverbLPHP::Process(TSAMPLE *pBuf, int nSampleFrames)
{
	TSAMPLE fixp;
    F32  s, t, i;

    for(i = 0; i < nSampleFrames * 2; i++)
    {
        /* L */
        fixp = FMUL16(pBuf[i], REV_INP_LEV);

        LPFL = FMUL32(LPFL, REV_LPF_LEV) + FMUL32(buf2_L[spt2]+tb, REV_LPF_INP) + FMUL32(ta, REV_WIDTH);
        ta = buf3_L[spt3];
        s  = buf3_L[spt3] = buf0_L[spt0];
        buf0_L[spt0] = -LPFL;

        t = FMUL32(HPFL + fixp, REV_HPF_LEV);
        HPFL = t - fixp;

        buf2_L[spt2] = FMUL32(s - FMUL16(fixp, REV_FBK_LEV), REV_NMIX_LEV);
        tb = buf1_L[spt1];
        buf1_L[spt1] = t;

        EPFL = FMUL32(EPFL, REV_EPF_LEV) + FMUL32(ta, REV_EPF_INP);
        pBuf[i] = ta + EPFL + fixp;

        /* R */
        fixp = FMUL16(pBuf[++i], REV_INP_LEV);

        LPFR = FMUL32(LPFR, REV_LPF_LEV) + FMUL32(buf2_R[spt2]+tb, REV_LPF_INP) + FMUL32(ta, REV_WIDTH);
        ta = buf3_R[spt3];
        s  = buf3_R[spt3] = buf0_R[spt0];
        buf0_R[spt0] = LPFR;

        t = FMUL32(HPFR + fixp, REV_HPF_LEV);
        HPFR = t - fixp;

        buf2_R[spt2] = FMUL32(s - FMUL16(fixp, REV_FBK_LEV), REV_NMIX_LEV);
        tb = buf1_R[spt1];
        buf1_R[spt1] = t;

        EPFR = FMUL32(EPFR, REV_EPF_LEV) + FMUL32(ta, REV_EPF_INP);
        pBuf[i] = ta + EPFR + fixp;

		spt0++; if(spt0 == rpt0) spt0 = 0;
		spt1++; if(spt1 == rpt1) spt1 = 0;
		spt2++; if(spt2 == rpt2) spt2 = 0;
		spt3++; if(spt3 == rpt3) spt3 = 0;
    }
}
