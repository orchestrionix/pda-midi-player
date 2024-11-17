#include "surface.h"
#include "blit16to16.h"

#ifdef USE_ASM
void Gfx_Blit16To16_OneLineA(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes);
void Gfx_Blit16To16_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes);
void Gfx_Blit16To16FlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes);
#else
static void Gfx_Blit16To16_OneLineA(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes)
{
	register unsigned int *pd = (unsigned int*)pDes, *ps = (unsigned int*)pStart;

	while(dwLen >= 8)
	{
		*(pd++) = *(ps++);
		*(pd++) = *(ps++);
		*(pd++) = *(ps++);
		*(pd++) = *(ps++);
		dwLen -= 8;
	}
	while(dwLen >= 2)
	{
		*(pd++) = *(ps++);
		dwLen -= 2;
	}
	if(dwLen)
	{
		*(short*)pd = *(short*)ps;
	}
}

static void Gfx_Blit16To16_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes)
{
	while(dwLen >= 8)
	{
		*(pDes++) = *(pStart++);
		*(pDes++) = *(pStart++);
		*(pDes++) = *(pStart++);
		*(pDes++) = *(pStart++);
		*(pDes++) = *(pStart++);
		*(pDes++) = *(pStart++);
		*(pDes++) = *(pStart++);
		*(pDes++) = *(pStart++);
		dwLen -= 8;
	}
	while(dwLen--)
	{
		*(pDes++) = *(pStart++);
	}
}

static void Gfx_Blit16To16FlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes)
{
	while(dwLen >= 8)
	{
		*(pDes++) = *(pStart--);
		*(pDes++) = *(pStart--);
		*(pDes++) = *(pStart--);
		*(pDes++) = *(pStart--);
		*(pDes++) = *(pStart--);
		*(pDes++) = *(pStart--);
		*(pDes++) = *(pStart--);
		*(pDes++) = *(pStart--);
		dwLen -= 8;
	}
	while(dwLen--)
	{
		*(pDes++) = *(pStart--);
	}
}
#endif

void Gfx_Blit16To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
/*	if(!((((unsigned int)pDesStart) & 3) ^ (((unsigned int)pSrcStart) & 3)))
	{
		if(!(((unsigned int)pDesStart) & 3))
		{
			while(h--)
			{
				Gfx_Blit16To16_OneLineA((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart);
				pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
				pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
			}

		}
		else
		{
			while(h--)
			{
				*(short*)pDesStart = *(short*)pSrcStart;
				Gfx_Blit16To16_OneLineA((unsigned short*)pSrcStart + 1, w - 1, (unsigned short*)pDesStart + 1);
				pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
				pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
			}
		}
	}
	else*/
	{
		while(h--)
		{
			Gfx_Blit16To16_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart);
			pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
			pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
		}
	}
}

void Gfx_Blit16To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	while(h--)
	{
		Gfx_Blit16To16FlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit16To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	if(!((((unsigned int)pDesStart) & 3) ^ (((unsigned int)pSrcStart) & 3)))
	{
		if(!(((unsigned int)pDesStart) & 3))
		{
			while(h--)
			{
				Gfx_Blit16To16_OneLineA((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart);
				pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
				pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
			}

		}
		else
		{
			while(h--)
			{
				*(short*)pDesStart = *(short*)pSrcStart;
				Gfx_Blit16To16_OneLineA((unsigned short*)pSrcStart + 1, w - 1, (unsigned short*)pDesStart + 1);
				pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
				pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
			}
		}
	}
	else
	{
		while(h--)
		{
			Gfx_Blit16To16_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart);
			pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
			pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
		}
	}
}

void Gfx_Blit16To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_Blit16To16FlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

#ifdef USE_ASM
void Gfx_Blit16To16T_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, int tc);
void Gfx_Blit16To16TFlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, int tc);
#else
static void Gfx_Blit16To16T_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, int tc)
{
	register int c;

	while(dwLen >= 8)
	{
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		dwLen -= 8;
	}
	while(dwLen)
	{
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart ++;
		dwLen--;
	}
}
static void Gfx_Blit16To16TFlipX_OneLine(unsigned short *pStart, unsigned int dwLen, unsigned short *pDes, int tc)
{
	register int c;

	while(dwLen >= 8)
	{
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		dwLen -= 8;
	}
	while(dwLen)
	{
		c = *pStart;
		if(c != tc)	*pDes = (unsigned short)c;
		pDes ++; pStart --;
		dwLen--;
	}
}
#endif

void Gfx_Blit16To16T(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	while(h--)
	{
		Gfx_Blit16To16T_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pSrc->m_ckSrc);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit16To16TFlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	while(h--)
	{
		Gfx_Blit16To16TFlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pSrc->m_ckSrc);
		pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit16To16TFlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_Blit16To16T_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pSrc->m_ckSrc);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}

void Gfx_Blit16To16TFlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h)
{
	pSrcStart = (short*)pSrcStart + w - 1;
	pSrcStart = (char*)pSrcStart + pSrc->m_pixelPitch * (h - 1);
	while(h--)
	{
		Gfx_Blit16To16TFlipX_OneLine((unsigned short*)pSrcStart, w, (unsigned short*)pDesStart, pSrc->m_ckSrc);
		pSrcStart = (char*)pSrcStart - pSrc->m_pixelPitch;
		pDesStart = (char*)pDesStart + pDes->m_pixelPitch;
	}
}
