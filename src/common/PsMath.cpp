#include "PSMath.h"
#include "PsObject.h"
#ifdef OS_SYMBIAN
	#include <e32math.h>
#else
	#include <MATH.H>
#endif

#ifndef FAST_MATH

CPsMath::CPsMath()
{

}

CPsMath::~CPsMath()
{

}

F32 CPsMath::DlsTime2FixedSec(int tc)
{
	return PSFLOAT2F(pow(2.0, tc / (65536*1200.0)));
}

F32 CPsMath::DlsPitch2AbsFreq(int pc)
{
	return Pitch2AbsFreq(pc >> (16 - F_SHIFT));
//	return PSFLOAT2F(440.0 * pow(2.0, pc / (65536.0*1200) - 6900.0 / 1200));
}

F16 CPsMath::Sin(F32 v)
{
	return PSFLOAT2F(sin(F2PSFLOAT(v)));
}

F32 CPsMath::Note2Pitch(int note)
{
	return I2F((note + 12) * 100);
}

F32 CPsMath::Pitch2AbsFreq(F32 pitch)
{
	return PSFLOAT2F(440.0 * pow(2.0, F2PSFLOAT(pitch) / 1200 - 6900.0 / 1200));
}

F32 CPsMath::Pitch2RelativFreq(F32 pitch)
{
	return PSFLOAT2F(pow(2.0, F2PSFLOAT(pitch) / 1200));
}

F32 CPsMath::AbsFreq2FixedPitch(F32 freq)
{
	return PSFLOAT2F(1200.0*(log(F2PSFLOAT(freq)/440)/log(2.0)) + 6900);
}

F32 CPsMath::FixedAttn2Volume(F32 f)
{
	if(f >= I2F(960))
		return 0;
	return PSFLOAT2F(pow(10.0, F2PSFLOAT(-f) / 200));
}

F32 CPsMath::Volume2FixedAttn(F32 v)
{
	if(v == 0)
		return I2F(960);
	return -PSFLOAT2F(200 * log10(F2PSFLOAT(v)));
}

F32 CPsMath::Sqrt(F32 v)
{
	return PSFLOAT2F(sqrt(F2PSFLOAT(v)));
}

#else

CPsMath::CPsMath()
{
	int i;
#ifdef OS_SYMBIAN
	PSFLOAT f;
	for(i = 0; i < SQRT_LEN; i++)
	{
		Math::Sqrt(f, F2PSFLOAT(F_ONE * i / SQRT_LEN));
		m_sqrt[i] = PSFLOAT2F(f);
	}
	for(i = 0; i < EXP2_LEN; i++)
	{
		Math::Pow(f, 2.0, F2PSFLOAT(F_ONE * i / EXP2_LEN));
		m_exp2[i] = PSFLOAT2F(f);
	}
	for(i = 0; i < EXP10_LEN; i++)
	{
		Math::Pow10(f, F2PSFLOAT(F_ONE * i / EXP10_LEN));
		m_exp10[i] = PSFLOAT2F(f);
	}
	for(i = 0; i < SIN_LEN; i++)
	{
		Math::Sin(f, F2PSFLOAT(F_ONE * i) * (2 * PI) / SIN_LEN);
		m_sin[i] = PSFLOAT2F(f);
	}
#else
	for(i = 0; i < SQRT_LEN; i++)
	{
		m_sqrt[i] = PSFLOAT2F(sqrt(F2PSFLOAT(F_ONE * i / SQRT_LEN)));
	}
	for(i = 0; i < EXP2_LEN; i++)
	{
		m_exp2[i] = PSFLOAT2F(pow(2.0, F2PSFLOAT(F_ONE * i / EXP2_LEN)));
	}
	for(i = 0; i < EXP10_LEN; i++)
	{
		m_exp10[i] = PSFLOAT2F(pow(10.0, F2PSFLOAT(F_ONE * i / EXP10_LEN)));
	}
	for(i = 0; i < SIN_LEN; i++)
	{
		m_sin[i] = PSFLOAT2F(sin(F2PSFLOAT(F_ONE * i) * (2 * PI) / SIN_LEN));
	}
#endif
}

CPsMath::~CPsMath()
{

}

#define SQRT2 PSFLOAT2F(0.707106781186547524e0)
#define LN2_RECP PSFLOAT2F(1.44269504088896)
#define P0 PSFLOAT2F(-.240139179559210510e2)
#define P1 PSFLOAT2F(0.309572928215376501e2)
#define P2 PSFLOAT2F(-.963769093368686593e1)
#define P3 PSFLOAT2F(0.421087371217979714e0)
#define Q0 PSFLOAT2F(-.120069589779605255e2)
#define Q1 PSFLOAT2F(0.194809660700889731e2)
#define Q2 PSFLOAT2F(-.891110902798312337e1)

F32 CPsMath::log2(F32 f)
{
	int exp;
	int sign = 0;
	if(f == F_ONE)
		return 0;
	else if(f <= 0)
		return 0;
	else if(f < F_ONE)
	{
		f = FRECIPROCAL(f);
		sign = 1;
	}
	for(exp = 30; exp >= 0; exp--)
	{
		if(f & (1 << exp))
			break;
	}
	exp -= F_SHIFT;
	F32 x = f >> exp;

	F32 z = FDIV32(x-F_ONE, x+F_ONE);
	F64 zsq = FMUL32(z, z);

	F64 temp = FMUL32(FMUL32(FMUL32(P3, zsq) + P2, zsq) + P1, zsq) + P0;
	temp = FDIV32(temp, FMUL32(FMUL32(zsq + Q2, zsq) + Q1, zsq) + Q0);
	temp = FMUL32(FMUL32(temp, z), LN2_RECP);

	F32 ret = (F32)temp + I2F(exp);
	if(sign)
		ret = -ret;
	return ret;
}

#define LOG2_10_FRAC PSFLOAT2F(0.301029995663)

F32 CPsMath::Log10(F32 f)
{
	return (F32)FMUL32(log2(f), LOG2_10_FRAC);
}

F32 CPsMath::DlsTime2FixedSec(int tc)
{
//	printf("%f->%f->%f\n", tc / (65536*1200.0), pow(2.0, tc / (65536*1200.0)), F2PSFLOAT(Pow2(tc / (1200 << (16 - F_SHIFT)))));
	return Pow2(tc / (1200 << (16 - F_SHIFT)));
}

F32 CPsMath::DlsPitch2AbsFreq(int pc)
{
//	printf("%f->%f->%f\n", pc / 65536.0, 440.0 * pow(2.0, pc / (65536.0*1200) - 6900.0 / 1200), F2PSFLOAT(Pitch2AbsFreq(pc >> (16 - F_SHIFT))));
	return Pitch2AbsFreq(pc >> (16 - F_SHIFT));
}

F16 CPsMath::Sin(F32 v)
{
	int n = v * SIN_LEN / PSFLOAT2F(PI * 2);
	CPsObject::PsAssert(n >= 0 && n < SIN_LEN);
//	printf("%f->%f->%f\n", F2PSFLOAT(v), sin(F2PSFLOAT(v)), F2PSFLOAT(m_sin[n]));
	return m_sin[n];
//	return PSFLOAT2F(sin(F2PSFLOAT(v)));
}

F32 CPsMath::Note2Pitch(int note)
{
	return I2F((note + 12) * 100);
}

F32 CPsMath::Pitch2AbsFreq(F32 pitch)
{
//	printf("%f->%f->%f\n", F2PSFLOAT(pitch), 440.0 * pow(2.0, F2PSFLOAT(pitch) / 1200 - 6900.0 / 1200), F2PSFLOAT(Pow2((pitch + PSFLOAT2F(3637.6316562)) / 1200)));
	return Pow2((pitch + PSFLOAT2F(3637.6316562)) / 1200);
}

F32 CPsMath::Pow2(F32 v)
{
	int isNegative = v < 0;
	if(isNegative)
		v = -v;
	F32 partFrac = m_exp2[(v & (F_ONE - 1)) >> (F_SHIFT - EXP2_SHIFT)];
	if(isNegative)
		return FRECIPROCAL(partFrac) >> (v >> F_SHIFT);
	else
		return partFrac << (v >> F_SHIFT);
}

F32 CPsMath::Pow10(F32 v)
{
	int isNegative = v < 0;
	if(isNegative)
		v = -v;
	F32 partFrac = m_exp10[(v & (F_ONE - 1)) >> (F_SHIFT - EXP10_SHIFT)];
	F32 partInt = v >> F_SHIFT;
	for(int i = 0; i < partInt; i++)
		partFrac *= 10;
	if(isNegative)
		return FRECIPROCAL(partFrac);
	else
		return partFrac;
}

F32 CPsMath::Pitch2RelativFreq(F32 pitch)
{
	return Pow2(pitch / 1200);
//	printf("%f->%f->%f\n", F2PSFLOAT(pitch) / 1200, pow(2.0, F2PSFLOAT(pitch) / 1200), F2PSFLOAT(ret));
//	return PSFLOAT2F(pow(2.0, F2PSFLOAT(pitch) / 1200));
}

F32 CPsMath::AbsFreq2FixedPitch(F32 freq)
{
	if(F2I(freq) == 44100)
		return PSFLOAT2F(14876.557586);
	else if(F2I(freq) == 22050)
		return PSFLOAT2F(13676.557586);
//	printf("%d %d %f->%f\n", freq / F_ONE, PSFLOAT2F(1200.0*(log(F2PSFLOAT(freq)/440)/log(2.0)) + 6900), 1200.0*(log(F2PSFLOAT(freq)/440)/log(2.0)) + 6900, F2PSFLOAT(1200 * log2(freq /440) + I2F(6900)));
//	return PSFLOAT2F(1200.0*(log(F2PSFLOAT(freq)/440)/log(2.0)) + 6900);
	return 1200 * log2(freq /440) + I2F(6900);
}

F32 CPsMath::FixedAttn2Volume(F32 f)
{
	if(f >= I2F(960))
		return 0;

	return Pow10(-f/200);
//	printf("%f->%f->%f\n", F2PSFLOAT(f) / 200, pow(10.0, F2PSFLOAT(-f) / 200), F2PSFLOAT(Pow10(-f/200)));
//	return PSFLOAT2F(pow(10.0, F2PSFLOAT(-f) / 200));
}

F32 CPsMath::Volume2FixedAttn(F32 v)
{
	if(v == 0)
		return I2F(960);
//	printf("%f->%f\n", 200 * log10(F2PSFLOAT(v)), F2PSFLOAT(200*Log10(v)));
	return -200*Log10(v);
}

F32 CPsMath::Sqrt(F32 v)
{
	int n = v >> (F_SHIFT - SQRT_SHIFT);
	CPsObject::PsAssert(n <= SQRT_LEN);
	if(n == SQRT_LEN)
		return F_ONE;

	return m_sqrt[n];
}
#endif

F32 CPsMath::Tan(F32 f)
{
#ifdef OS_SYMBIAN
	PSFLOAT ret;
	Math::Tan(ret, F2PSFLOAT(f));
	return PSFLOAT2F(ret);
#else
	return PSFLOAT2F(tan(F2PSFLOAT(f)));
#endif
}

F32 CPsMath::Cos(F32 f)
{
#ifdef OS_SYMBIAN
	PSFLOAT ret;
	Math::Cos(ret, F2PSFLOAT(f));
	return PSFLOAT2F(ret);
#else
	return PSFLOAT2F(cos(F2PSFLOAT(f)));
#endif
}
