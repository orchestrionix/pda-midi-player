#include "PsFilter.h"
#include "win32/PsSys.h"
#include <MATH.H>

void CPsFilterLowPass::InitLowPass()
{
	m_fGain = F_ONE;
	m_ul = m_ur = 0;
	m_xl = m_xr = 0;
}

void CPsFilterLowPass::SetParameters(CPsMath* pMath, int cutOff, F32 fGain, int sampleRate)
{
	F32 a = PSFLOAT2F(PI) * cutOff / sampleRate;
	a = pMath->Tan(a);
	a = FDIV32(a * 4, F_ONE + fGain);
	m_fGain = fGain;
	m_k = FDIV32(F_ONE - a, F_ONE + a);
}

void CPsFilterLowPass::ProcessStereo(int *pS, int count)
{
	if(m_fGain == F_ONE)
		return;
	register int ul = m_ul;
	register int ur = m_ur;
	register int xl = m_xl;
	register int xr = m_xr;
	register const F16 a = (F_ONE - m_k) / 2;
	register int s;
	register const F32 g = m_fGain - F_ONE;
	while(count > 0)
	{
		s = *pS >> MIX_SHIFT;
		ul = F2I((s + xl) * a + m_k * ul);
		xl = s;
		*pS++ += (g * ul) >> (F_SHIFT - MIX_SHIFT);
		s = *pS >> MIX_SHIFT;
		ur = F2I((s + xr) * a + m_k * ur);
		xr = s;
		*pS++ += (g * ur) >> (F_SHIFT - MIX_SHIFT);
		--count;
	}

	m_ul = ul;
	m_xl = xl;
	m_ur = ur;
	m_xr = xr;
}

CPsFilterLHPass::CPsFilterLHPass()
{
}

CPsFilterLHPass::~CPsFilterLHPass()
{
}

void CPsFilterLHPass::InitLowPass(CPsMath* pMath, int cutOff, F32 rez, int sampleRate)
{
	m_c = FRECIPROCAL(pMath->Tan(FMULDIV(PSFLOAT2F(PI), cutOff, sampleRate)));
	m_a[0] = FRECIPROCAL(F_ONE + FMUL32(rez, m_c) + FMUL32(m_c, m_c));
	m_a[1] = 2 * m_a[0];
	m_a[2] = m_a[0];
	m_b[0] = 2 * (F_ONE - FMUL32(m_c, m_c));
	m_b[0] = FMUL32(m_b[0], m_a[0]);
	m_b[1] = FMUL32((F_ONE - FMUL32(rez, m_c) + FMUL32(m_c, m_c)), m_a[0]);
	m_in[0] = m_in[1] = 0;
	m_out[0] = m_out[1] = 0;
}

void CPsFilterLHPass::Process(int *pS, int *pD, int count, int step)
{
	register int i1, i2, o1, o2;
	i1 = m_in[0]; i2 = m_in[1];
	o1 = m_out[0]; o2 = m_out[1];
	
	while(count > 0)
	{
		*pD = FMUL32(m_a[0], *pS) + FMUL32(m_a[1], i1) + FMUL32(m_a[2], i2) -
			FMUL32(m_b[0], o1) - FMUL32(m_b[1], o2);
		i2 = i1;
		i1 = *pS;
		o2 = o1;
		o1 = *pD;
		pD += step;
		pS += step;
		--count;
	}

	m_in[0] = i1; m_in[1] = i2;
	m_out[0] = o1; m_out[1] = o2;
}

void CPsFilterLHPass::InitHiPass(CPsMath *pMath, int cutOff, F32 rez, int sampleRate)
{
	m_c = pMath->Tan(FMULDIV(PSFLOAT2F(PI), cutOff, sampleRate));
	m_a[0] = FRECIPROCAL(F_ONE + FMUL32(rez, m_c) + FMUL32(m_c, m_c));
	m_a[1] = -2 * m_a[0];
	m_a[2] = m_a[0];
	m_b[0] = 2 * (FMUL32(m_c, m_c) - F_ONE);
	m_b[0] = FMUL32(m_b[0], m_a[0]);
	m_b[1] = FMUL32((F_ONE - FMUL32(rez, m_c) + FMUL32(m_c, m_c)), m_a[0]);
	m_in[0] = m_in[1] = 0;
	m_out[0] = m_out[1] = 0;
}

inline PSFLOAT Sgn(PSFLOAT x) { return (x >= 0) ? 1.0f : -1.0f; }
void ShelfEQ(long scale,
			 long *outA1, long *outB0, long *outB1,
			 long F_c, long F_s, PSFLOAT gainDC, PSFLOAT gainFT, PSFLOAT gainPI)
{
	PSFLOAT a1, b0, b1;
	PSFLOAT gainFT2, gainDC2, gainPI2;
	PSFLOAT alpha, beta0, beta1, rho;
	PSFLOAT wT, quad;

	wT = PI * F_c / F_s;

	gainPI2 = gainPI * gainPI;
	gainFT2 = gainFT * gainFT;
	gainDC2 = gainDC * gainDC;

	quad = gainPI2 + gainDC2 - (gainFT2*2);

	alpha = 0;
 
	if (quad != 0)
	{
		PSFLOAT lambda = (gainPI2 - gainDC2) / quad;
		alpha  = (PSFLOAT)(lambda - Sgn(lambda)*sqrt(lambda*lambda - 1.0f));
	}
 
	beta0 = 0.5f * ((gainDC + gainPI) + (gainDC - gainPI) * alpha);
	beta1 = 0.5f * ((gainDC - gainPI) + (gainDC + gainPI) * alpha);
	rho   = (PSFLOAT)((sin((wT*0.5f) - (PI/4.0f))) / (sin((wT*0.5f) + (PI/4.0f))));
 
	quad  = 1.0f / (1.0f + rho*alpha);
    
	b0 = ((beta0 + rho*beta1) * quad);
	b1 = ((beta1 + rho*beta0) * quad);
	a1 = - ((rho + alpha) * quad);

	*outA1 = (int)(a1 * scale);
	*outB0 = (int)(b0 * scale);
	*outB1 = (int)(b1 * scale);
}

CPsFilterSurround::CPsFilterSurround()
{
}

CPsFilterSurround::~CPsFilterSurround()
{
}

// [Surround level 1(quiet)-16(heavy)] [delay in ms, usually 5-50ms]
void CPsFilterSurround::Init(CPsMath* pMath, int level, unsigned int nDelay, int sampleRate)
{
	m_nSurroundPos = 0;
	CPsSys::memset(m_SurroundBuffer, 0, sizeof(m_SurroundBuffer));
	if (nDelay < 4) nDelay = 4;
	if (nDelay > 50) nDelay = 50;
	m_nSurroundSize = (sampleRate * nDelay) / 1000;
	if (m_nSurroundSize > SURROUNDBUFFERSIZE) m_nSurroundSize = SURROUNDBUFFERSIZE;
	PsAssert(level > 0 && level <= 16);
	// Setup biquad filters
	ShelfEQ(1024, &m_nDolbyHP_A1, &m_nDolbyHP_B0, &m_nDolbyHP_B1, 200, sampleRate, 0, 0.5f, 1);
	ShelfEQ(1024, &m_nDolbyLP_A1, &m_nDolbyLP_B0, &m_nDolbyLP_B1, 7000, sampleRate, 1, 0.75f, 0);
	m_nDolbyHP_X1 = m_nDolbyHP_Y1 = 0;
	m_nDolbyLP_Y1 = 0;
	// Surround Level
	m_nDolbyHP_B0 = (m_nDolbyHP_B0 * level) >> 5;
	m_nDolbyHP_B1 = (m_nDolbyHP_B1 * level) >> 5;
	// +6dB
	m_nDolbyLP_B0 *= 2;
	m_nDolbyLP_B1 *= 2;
}

void CPsFilterSurround::Process(int *pS, int count)
{
	int *pr = pS, hy1 = m_nDolbyHP_Y1;
	for (int r=count; r; r--)
	{
		// Delay
		int secho = m_SurroundBuffer[m_nSurroundPos];
		m_SurroundBuffer[m_nSurroundPos] = (pr[0]+pr[1]+256) >> 9;
		// High-pass
		int v0 = (m_nDolbyHP_B0 * secho + m_nDolbyHP_B1 * m_nDolbyHP_X1 + m_nDolbyHP_A1 * hy1) >> 10;
		m_nDolbyHP_X1 = secho;
		// Low-pass
		int v = (m_nDolbyLP_B0 * v0 + m_nDolbyLP_B1 * hy1 + m_nDolbyLP_A1 * m_nDolbyLP_Y1) >> (10-8);
		hy1 = v0;
		m_nDolbyLP_Y1 = v >> 8;
		// Add echo
		pr[0] += v;
		pr[1] -= v;
		if (++m_nSurroundPos >= m_nSurroundSize) m_nSurroundPos = 0;
		pr += 2;
	}
	m_nDolbyHP_Y1 = hy1;
}