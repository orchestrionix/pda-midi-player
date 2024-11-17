#include "surface.h"
#include "blita16to16.h"

#if defined(USE_ASM) && defined(__MARM__)
void Gfx_BlitA16To16_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData);
void Gfx_BlitA16To16FlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData);
#else
static void Gfx_BlitA16To16_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData)
{
	register unsigned int s, d, t0 = 0x7e0f81f, a;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		if(a = *pAlphaData++)
		{
			s = *pStart;
			d = *pDes;

			s = (s | (s << 16)) & t0;
			d = (d | (d << 16)) & t0;
			d = (d + (((s - d) * a) >> 5)) & t0;
			*pDes = (unsigned short)(d | (d >> 16));
		}
		pDes++;pStart++;
	}
}

static void Gfx_BlitA16To16FlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData)
{
	register unsigned int s, d, t0 = 0x7e0f81f, a;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		if(a = *pAlphaData--)
		{
			s = *pStart;
			d = *pDes;

			s = (s | (s << 16)) & t0;
			d = (d | (d << 16)) & t0;
			d = (d + (((s - d) * a) >> 5)) & t0;
			*pDes = (unsigned short)(d | (d >> 16));
		}
		pDes++;pStart--;
	}
}
#endif

void Gfx_BlitA16To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData)
{
	while(h--)
	{
		Gfx_BlitA16To16_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pAlphaData += pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitA16To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	pAlphaData += w - 1;
	while(h--)
	{
		Gfx_BlitA16To16FlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pAlphaData += pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitA16To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	pAlphaData += pSrc->m_alphaPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitA16To16_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pAlphaData -= pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitA16To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	pAlphaData += pSrc->m_alphaPitch * (h - 1);
	pSrcStart = (short*)pSrcStart + w - 1;
	pAlphaData += w - 1;
	while(h--)
	{
		Gfx_BlitA16To16FlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pAlphaData -= pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

/****************************** For nOpacity *****************************/

#if defined(USE_ASM) && defined(__MARM__)
void Gfx_BlitTA16To16_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA16To16FlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned int nOpacity);
#else
static void Gfx_BlitTA16To16_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = *pStart++;
		d = *pDes;
		s = (s | (s << 16)) & t0;
		d = (d | (d << 16)) & t0;
		d = (d + (((s - d) * ((nOpacity * *pAlphaData++) >> 5)) >> 5)) & t0;
		*pDes++ = (unsigned short)(d | (d >> 16));
	}
}
static void Gfx_BlitTA16To16FlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = *pStart--;
		d = *pDes;
		s = (s | (s << 16)) & t0;
		d = (d | (d << 16)) & t0;
		d = (d + (((s - d) * ((nOpacity * *pAlphaData--) >> 5)) >> 5)) & t0;
		*pDes++ = (unsigned short)(d | (d >> 16));
	}
}
#endif

void Gfx_BlitTA16To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity)
{
	while(h--)
	{
		Gfx_BlitTA16To16_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pAlphaData += pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitTA16To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	pAlphaData += w - 1;
	while(h--)
	{
		Gfx_BlitTA16To16FlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pAlphaData += pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitTA16To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	pAlphaData += pSrc->m_alphaPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitTA16To16_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pAlphaData -= pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitTA16To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	pAlphaData += pSrc->m_alphaPitch * (h - 1);
	pSrcStart = (short*)pSrcStart + w - 1;
	pAlphaData += w - 1;
	while(h--)
	{
		Gfx_BlitTA16To16FlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pAlphaData -= pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

#if defined(USE_ASM) && defined(__MARM__)
void Gfx_BlitT16To16_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned int nOpacity);
void Gfx_BlitT16To16FlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned int nOpacity);
#else
static void Gfx_BlitT16To16_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = *pStart++;
		d = *pDes;
		s = (s | (s << 16)) & t0;
		d = (d | (d << 16)) & t0;
		d = (d + (((s - d) * nOpacity) >> 5)) & t0;
		*pDes++ = (unsigned short)(d | (d >> 16));
	}
}

static void Gfx_BlitT16To16FlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = *pStart--;
		d = *pDes;
		s = (s | (s << 16)) & t0;
		d = (d | (d << 16)) & t0;
		d = (d + (((s - d) * nOpacity) >> 5)) & t0;
		*pDes++ = (unsigned short)(d | (d >> 16));
	}
}
#endif

void Gfx_BlitT16To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	while(h--)
	{
		Gfx_BlitT16To16_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT16To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	while(h--)
	{
		Gfx_BlitT16To16FlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT16To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitT16To16_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT16To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitT16To16FlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

#if defined(USE_ASM) && defined(__MARM__)
void Gfx_BlitT16To16T_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, int tc, unsigned int nOpacity);
void Gfx_BlitT16To16TFlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, int tc, unsigned int nOpacity);
#else
static void Gfx_BlitT16To16T_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, int tc, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = *pStart++;
		if((int)s != tc)
		{
			d = *pDes;
			s = (s | (s << 16)) & t0;
			d = (d | (d << 16)) & t0;
			d = (d + (((s - d) * nOpacity) >> 5)) & t0;
			*pDes = (unsigned short)(d | (d >> 16));
		}
		pDes++;
	}
}
static void Gfx_BlitT16To16TFlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, int tc, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = *pStart--;
		if((int)s != tc)
		{
			d = *pDes;
			s = (s | (s << 16)) & t0;
			d = (d | (d << 16)) & t0;
			d = (d + (((s - d) * nOpacity) >> 5)) & t0;
			*pDes = (unsigned short)(d | (d >> 16));
		}
		pDes++;
	}
}
#endif

void Gfx_BlitT16To16T(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	while(h--)
	{
		Gfx_BlitT16To16T_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pSrc->m_ckSrc, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT16To16TFlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	while(h--)
	{
		Gfx_BlitT16To16TFlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pSrc->m_ckSrc, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT16To16TFlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitT16To16T_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pSrc->m_ckSrc, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT16To16TFlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitT16To16TFlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pSrc->m_ckSrc, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

