#ifndef __PSTYPE_H__
#define __PSTYPE_H__

#include "PSConfig.h"

typedef short F16;
typedef int F32;

#if defined COMPILER_VC
	typedef __int64 F64;
	typedef double PSFLOAT;
#elif defined OS_SYMBIAN
	typedef long long F64;
	typedef TReal PSFLOAT;
#elif defined COMPILER_GCC
	typedef long long F64;
	typedef float PSFLOAT;
#endif

typedef short NativeSample;
#define NATIVE_SAMPLE_MAX 0x7fff
#define NATIVE_SAMPLE_MIN (-32768)

#endif
