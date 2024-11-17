#include "surface.h"
#include "blita8to16.h"

#if defined(USE_ASM) && defined(__MARM__)
void Gfx_BlitA8To16_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned short *m_pPalette);
void Gfx_BlitA8To16FlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned short *m_pPalette);
#else
static void Gfx_BlitA8To16_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned short *m_pPalette)
{
	register unsigned int s, d, t0 = 0x7e0f81f, a;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		if(a = *pAlphaData++)
		{
			s = m_pPalette[*pStart];
			d = *pDes;

			s = (s | (s << 16)) & t0;
			d = (d | (d << 16)) & t0;
			d = (d + (((s - d) * a) >> 5)) & t0;
			*pDes = (unsigned short)(d | (d >> 16));
		}
		pDes++;pStart++;
	}
}

static void Gfx_BlitA8To16FlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned short *m_pPalette)
{
	register unsigned int s, d, t0 = 0x7e0f81f, a;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		if(a = *pAlphaData--)
		{
			s = m_pPalette[*pStart];
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
void Gfx_BlitA8To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData)
{
	while(h--)
	{
		Gfx_BlitA8To16_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, (unsigned short*)pSrc->m_pPalette);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pAlphaData += pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitA8To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	pAlphaData += w - 1;
	while(h--)
	{
		Gfx_BlitA8To16FlipX_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, (unsigned short*)pSrc->m_pPalette);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pAlphaData += pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitA8To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	pAlphaData += pSrc->m_alphaPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitA8To16_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, (unsigned short*)pSrc->m_pPalette);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pAlphaData -= pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitA8To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	pAlphaData += pSrc->m_alphaPitch * (h - 1);
	pSrcStart = (char*)pSrcStart + w - 1;
	pAlphaData += w - 1;
	while(h--)
	{
		Gfx_BlitA8To16FlipX_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, (unsigned short*)pSrc->m_pPalette);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pAlphaData -= pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

/****************************** For nOpacity *****************************/

#if defined(USE_ASM) && defined(__MARM__)
void Gfx_BlitTA8To16_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned short *m_pPalette, unsigned int nOpacity);
void Gfx_BlitTA8To16FlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned short *m_pPalette, unsigned int nOpacity);
#else
static void Gfx_BlitTA8To16_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned short *m_pPalette, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = m_pPalette[*pStart++];
		d = *pDes;
		s = (s | (s << 16)) & t0;
		d = (d | (d << 16)) & t0;
		d = (d + (((s - d) * ((nOpacity * *pAlphaData++) >> 5)) >> 5)) & t0;
		*pDes++ = (unsigned short)(d | (d >> 16));
	}
}

static void Gfx_BlitTA8To16FlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned char *pAlphaData, unsigned short *m_pPalette, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = m_pPalette[*pStart--];
		d = *pDes;
		s = (s | (s << 16)) & t0;
		d = (d | (d << 16)) & t0;
		d = (d + (((s - d) * ((nOpacity *  *pAlphaData--) >> 5)) >> 5)) & t0;
		*pDes++ = (unsigned short)(d | (d >> 16));
	}
}
#endif
void Gfx_BlitTA8To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity)
{
	while(h--)
	{
		Gfx_BlitTA8To16_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, (unsigned short*)pSrc->m_pPalette, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pAlphaData += pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitTA8To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	pAlphaData += w - 1;
	while(h--)
	{
		Gfx_BlitTA8To16FlipX_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, (unsigned short*)pSrc->m_pPalette, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pAlphaData += pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitTA8To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	pAlphaData += pSrc->m_alphaPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitTA8To16_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, (unsigned short*)pSrc->m_pPalette, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pAlphaData -= pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitTA8To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	pAlphaData += pSrc->m_alphaPitch * (h - 1);
	pSrcStart = (char*)pSrcStart + w - 1;
	pAlphaData += w - 1;
	while(h--)
	{
		Gfx_BlitTA8To16FlipX_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, pAlphaData, (unsigned short*)pSrc->m_pPalette, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pAlphaData -= pSrc->m_alphaPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

#if defined(USE_ASM) && defined(__MARM__)
void Gfx_BlitT8To16_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, unsigned int nOpacity);
void Gfx_BlitT8To16FlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, unsigned int nOpacity);
#else
static void Gfx_BlitT8To16_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = m_pPalette[*pStart++];
		d = *pDes;
		s = (s | (s << 16)) & t0;
		d = (d | (d << 16)) & t0;
		d = (d + (((s - d) * nOpacity) >> 5)) & t0;
		*pDes++ = (unsigned short)(d | (d >> 16));
	}
}

static void Gfx_BlitT8To16FlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = m_pPalette[*pStart--];
		d = *pDes;
		s = (s | (s << 16)) & t0;
		d = (d | (d << 16)) & t0;
		d = (d + (((s - d) * nOpacity) >> 5)) & t0;
		*pDes++ = (unsigned short)(d | (d >> 16));
	}
}
#endif

void Gfx_BlitT8To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	while(h--)
	{
		Gfx_BlitT8To16_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, (unsigned short*)pSrc->m_pPalette, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT8To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	while(h--)
	{
		Gfx_BlitT8To16FlipX_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, (unsigned short*)pSrc->m_pPalette, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT8To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitT8To16_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, (unsigned short*)pSrc->m_pPalette, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT8To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitT8To16FlipX_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, (unsigned short*)pSrc->m_pPalette, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

#if defined(USE_ASM) && defined(__MARM__)
void Gfx_BlitT8To16T_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, int tc, unsigned int nOpacity);
void Gfx_BlitT8To16TFlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, int tc, unsigned int nOpacity);
#else
static void Gfx_BlitT8To16T_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, int tc, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = *pStart++;
		if((int)s != tc)
		{
			s = m_pPalette[s];
			d = *pDes;
			s = (s | (s << 16)) & t0;
			d = (d | (d << 16)) & t0;
			d = (d + (((s - d) * nOpacity) >> 5)) & t0;
			*pDes = (unsigned short)(d | (d >> 16));
		}
		pDes++;
	}
}

static void Gfx_BlitT8To16TFlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, int tc, unsigned int nOpacity)
{
	register unsigned int s, d, t0 = 0x7e0f81f;//, t1 = 0x7e0;
	register unsigned short *pe = pDes + dwLen;

	while(pDes < pe)
	{
		s = *pStart--;
		if((int)s != tc)
		{
			s = m_pPalette[s];
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

void Gfx_BlitT8To16T(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	while(h--)
	{
		Gfx_BlitT8To16T_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, (unsigned short*)pSrc->m_pPalette, pSrc->m_ckSrc, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT8To16TFlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	while(h--)
	{
		Gfx_BlitT8To16TFlipX_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, (unsigned short*)pSrc->m_pPalette, pSrc->m_ckSrc, nOpacity);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT8To16TFlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitT8To16T_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, (unsigned short*)pSrc->m_pPalette, pSrc->m_ckSrc, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_BlitT8To16TFlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_BlitT8To16TFlipX_OneLine((unsigned char*)pSrcStart, w, (unsigned short*)pDesStart, (unsigned short*)pSrc->m_pPalette, pSrc->m_ckSrc, nOpacity);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}
