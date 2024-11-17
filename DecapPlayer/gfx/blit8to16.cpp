#include "surface.h"
#include "blit16to16.h"

#ifdef USE_ASM
void Gfx_Blit8To16_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette);
void Gfx_Blit8To16FlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette);
#else
static void Gfx_Blit8To16_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette)
{
	while(dwLen >= 8)
	{
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
		dwLen -= 8;
	}
	while(dwLen--)
	{
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart ++;
	}
}

static void Gfx_Blit8To16FlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette)
{
	while(dwLen >= 8)
	{
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		dwLen -= 8;
	}
	while(dwLen)
	{
		*pDes = m_pPalette[*pStart];
		pDes ++; pStart --;
		dwLen--;
	}
}

#endif
void Gfx_Blit8To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	while(h--)
	{
		Gfx_Blit8To16_OneLine((unsigned char *)pSrcStart, w, (unsigned short *)pDesStart, (unsigned short*)pSrc->m_pPalette);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit8To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	while(h--)
	{
		Gfx_Blit8To16FlipX_OneLine((unsigned char *)pSrcStart, w, (unsigned short *)pDesStart, (unsigned short*)pSrc->m_pPalette);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit8To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_Blit8To16_OneLine((unsigned char *)pSrcStart, w, (unsigned short *)pDesStart, (unsigned short*)pSrc->m_pPalette);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit8To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_Blit8To16FlipX_OneLine((unsigned char *)pSrcStart, w, (unsigned short *)pDesStart, (unsigned short*)pSrc->m_pPalette);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

#ifdef USE_ASM
void Gfx_Blit8To16T_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, int tc);
void Gfx_Blit8To16TFlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, int tc);
#else
static void Gfx_Blit8To16T_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, int tc)
{
	register int c;

	while(dwLen >= 8)
	{
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		dwLen -= 8;
	}
	while(dwLen)
	{
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart ++;
		dwLen--;
	}
}

static void Gfx_Blit8To16TFlipX_OneLine(unsigned char *pStart, unsigned int dwLen, unsigned short *pDes, unsigned short *m_pPalette, int tc)
{
	register int c;

	while(dwLen >= 8)
	{
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		dwLen -= 8;
	}
	while(dwLen)
	{
		c = *pStart;
		if(c != tc)	*pDes = m_pPalette[c];
		pDes ++; pStart --;
		dwLen--;
	}
}
#endif

void Gfx_Blit8To16T(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	while(h--)
	{
		Gfx_Blit8To16T_OneLine((unsigned char *)pSrcStart, w, (unsigned short *)pDesStart, (unsigned short*)pSrc->m_pPalette, pSrc->m_ckSrc);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit8To16TFlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	while(h--)
	{
		Gfx_Blit8To16TFlipX_OneLine((unsigned char *)pSrcStart, w, (unsigned short *)pDesStart, (unsigned short*)pSrc->m_pPalette, pSrc->m_ckSrc);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit8To16TFlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_Blit8To16T_OneLine((unsigned char *)pSrcStart, w, (unsigned short *)pDesStart, (unsigned short*)pSrc->m_pPalette, pSrc->m_ckSrc);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit8To16TFlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (char*)pSrcStart + w - 1;
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_Blit8To16TFlipX_OneLine((unsigned char *)pSrcStart, w, (unsigned short *)pDesStart, (unsigned short*)pSrc->m_pPalette, pSrc->m_ckSrc);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}