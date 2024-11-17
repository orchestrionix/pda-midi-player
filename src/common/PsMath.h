// PSMath.h: interface for the CPsMath class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PSMATH_H__
#define __PSMATH_H__

#include "PsType.h"

#define F_SHIFT 14

#define F_ZERO	0
#define F_ONE	(1 << F_SHIFT)

#define F32_MAX 0x7fffffff
#define F16_MAX 0x7fff

#define F2I(x) ((x) >> F_SHIFT)
#define I2F(x) (((x) << F_SHIFT))
#define I2F64(x) ((((F64)x) << F_SHIFT))

#define FMUL16(x, y) ((x) * (y) >> F_SHIFT)
#define FMUL32(x, y) ((F64)(x) * (y) >> F_SHIFT)
#define FMUL32_FAST(x, y) (((x) >> F_SHIFT/2) * ((y) >> F_SHIFT/2))
#define FMULDIV(x, y, z) ((F32)((F64)(x) * (y) / (z)))

#define FRECIPROCAL(x) (I2F(F_ONE) / (x))
#define FDIV32(x, y) ((F32)(I2F((F64)(x)) / (y)))
#define PSFLOAT2F(x) ((int)((x) * F_ONE))
#define F2PSFLOAT(x) ((x)/(F_ONE*1.0))

#define PI 3.141592653589

#define FAST_MATH

class CPsMath
{
public:
	F32 Cos(F32 f);
	F32 Tan(F32 f);
	F32 Sqrt(F32 v);
	F32 Volume2FixedAttn(F32 v);
	F32 FixedAttn2Volume(F32 f);
	F32 AbsFreq2FixedPitch(F32 freq);
	F32 Pitch2RelativFreq(F32 pitch);
	F32 Pitch2AbsFreq(F32 pitch);
	F32 Note2Pitch(int note);
	F16 Sin(F32 v);
	F32 DlsTime2FixedSec(int tc);
	F32 DlsPitch2AbsFreq(int tc);

	static F32 DlsGain2FixedAttn(int g){
		return -g / (65536 / F_ONE);
	}
	static F32 DlsUnits2Fixed(int n){
		return n / (65536000 / F_ONE);
	}
	static F32 DlsPitch2FixedPitch(int t){
		return t / (65536 / F_ONE);
	}
	CPsMath();
	virtual ~CPsMath();

#ifdef FAST_MATH
	enum {
		SQRT_SHIFT = 8,
		SQRT_LEN = 1 << SQRT_SHIFT,
		EXP2_SHIFT = 8,
		EXP2_LEN = 1 << EXP2_SHIFT,
		EXP10_SHIFT = 8,
		EXP10_LEN = 1 << EXP10_SHIFT,
		SIN_SHIFT = 8,
		SIN_LEN = 1 << SIN_SHIFT
	};
	F32 Pow2(F32 v);
	F32 Pow10(F32 v);
	static F32 log2(F32 f);
	static F32 Log10(F32 f);

	F16 m_sqrt[SQRT_LEN];
	F16 m_exp2[EXP2_LEN];
	F16 m_sin[SIN_LEN];
	F32 m_exp10[EXP10_LEN];
#endif
};

#endif // !defined(AFX_PSMATH_H__C98BE57A_ED01_4BE1_A1AC_D704D0BF278D__INCLUDED_)
