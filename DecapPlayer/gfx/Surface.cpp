// Surface.cpp: implementation of the CSurface class.
//
//////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "Surface.h"
#include "blit8to16.h"
#include "blit16to16.h"
#include "blita8to16.h"
#include "blita16to16.h"

static const GFXBLITFUNC8TO16 g_blitFunc8To16[] = {
	Gfx_Blit8To16,
	Gfx_Blit8To16FlipX,
	Gfx_Blit8To16FlipY,
	Gfx_Blit8To16FlipXY,
	Gfx_Blit8To16T,
	Gfx_Blit8To16TFlipX,
	Gfx_Blit8To16TFlipY,
	Gfx_Blit8To16TFlipXY,
};

static const GFXBLITALPHAFUNC8TO16 g_blitAlphaFunc8To16[] = {
	Gfx_BlitA8To16,
	Gfx_BlitA8To16FlipX,
	Gfx_BlitA8To16FlipY,
	Gfx_BlitA8To16FlipXY,
	Gfx_BlitA8To16,
	Gfx_BlitA8To16FlipX,
	Gfx_BlitA8To16FlipY,
	Gfx_BlitA8To16FlipXY,
};

static const GFXBLITTRANSFUNC8TO16 g_blitTransFunc8To16[] = {
	Gfx_BlitT8To16,
	Gfx_BlitT8To16FlipX,
	Gfx_BlitT8To16FlipY,
	Gfx_BlitT8To16FlipXY,
	Gfx_BlitT8To16T,
	Gfx_BlitT8To16TFlipX,
	Gfx_BlitT8To16TFlipY,
	Gfx_BlitT8To16TFlipXY,
};

static const GFXBLITTRANSALPHAFUNC8TO16 g_blitTransAlphaFunc8To16[] = {
	Gfx_BlitTA8To16,
	Gfx_BlitTA8To16FlipX,
	Gfx_BlitTA8To16FlipY,
	Gfx_BlitTA8To16FlipXY,
	Gfx_BlitTA8To16,
	Gfx_BlitTA8To16FlipX,
	Gfx_BlitTA8To16FlipY,
	Gfx_BlitTA8To16FlipXY,
};

static const GFXBLITFUNC16TO16 g_blitFunc16To16[] = {
	Gfx_Blit16To16,
	Gfx_Blit16To16FlipX,
	Gfx_Blit16To16FlipY,
	Gfx_Blit16To16FlipXY,
	Gfx_Blit16To16T,
	Gfx_Blit16To16TFlipX,
	Gfx_Blit16To16TFlipY,
	Gfx_Blit16To16TFlipXY,
};

static const GFXBLITALPHAFUNC16TO16 g_blitAlphaFunc16To16[] = {
	Gfx_BlitA16To16,
	Gfx_BlitA16To16FlipX,
	Gfx_BlitA16To16FlipY,
	Gfx_BlitA16To16FlipXY,
	Gfx_BlitA16To16,
	Gfx_BlitA16To16FlipX,
	Gfx_BlitA16To16FlipY,
	Gfx_BlitA16To16FlipXY,
};

static const GFXBLITTRANSFUNC16TO16 g_blitTransFunc16To16[] = {
	Gfx_BlitT16To16,
	Gfx_BlitT16To16FlipX,
	Gfx_BlitT16To16FlipY,
	Gfx_BlitT16To16FlipXY,
	Gfx_BlitT16To16T,
	Gfx_BlitT16To16TFlipX,
	Gfx_BlitT16To16TFlipY,
	Gfx_BlitT16To16TFlipXY,
};

static const GFXBLITTRANSALPHAFUNC16TO16 g_blitTransAlphaFunc16To16[] = {
	Gfx_BlitTA16To16,
	Gfx_BlitTA16To16FlipX,
	Gfx_BlitTA16To16FlipY,
	Gfx_BlitTA16To16FlipXY,
	Gfx_BlitTA16To16,
	Gfx_BlitTA16To16FlipX,
	Gfx_BlitTA16To16FlipY,
	Gfx_BlitTA16To16FlipXY,
};
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSurface::CSurface()
{
	m_pSlices = NULL;
	m_pPalette = NULL;
	m_pPixel = NULL;
	m_pAlpha = NULL;
}

CSurface::~CSurface()
{
	if(m_pSlices)
		delete[] m_pSlices;
	if(m_pPalette)
		delete[] m_pPalette;
	if(m_pPixel)
		delete[] m_pPixel;
	if(m_pAlpha)
		delete[] m_pAlpha;
}

CSurface::CSurface(void *pSurfaceData)
{
	SURFACEDATA *pData = (SURFACEDATA*)pSurfaceData;
	char *p;

	ASSERT(pData != NULL);
	
	m_bpp = pData->bpp;
	m_flag = pData->flag;
	m_width = pData->width;
	m_height = pData->height;
	m_pixelPitch = pData->pixelPitch;
	m_alphaPitch = pData->alphaPitch;
	m_ckSrc = pData->ckSrc;
	m_nSlices = pData->nSlices;
	m_nPaletteEntries = pData->nPaletteEntries;
	m_clipRect[0] = m_clipRect[1] = 0;
	m_clipRect[2] = pData->width;
	m_clipRect[3] = pData->height;
	p = (char*)pData + sizeof(SURFACEDATA);
	if(pData->sizeSliceData)
	{
		m_pSlices = (SLICE*)new char[pData->sizeSliceData];
		memcpy(m_pSlices, p, pData->sizeSliceData);
		p += pData->sizeSliceData;
	}
	else
		m_pSlices = NULL;
	if(pData->sizePaletteData)
	{
		m_pPalette = new unsigned char[pData->sizePaletteData];
		memcpy(m_pPalette, p, pData->sizePaletteData);
		p += pData->sizePaletteData;
	}
	else
		m_pPalette = NULL;
	if(pData->sizePixelData)
	{
		m_pPixel = new unsigned char[pData->sizePixelData];
		memcpy(m_pPixel, p, pData->sizePixelData);
		p += pData->sizePixelData;
	}
	else
		m_pPixel = NULL;
	if(pData->sizeAlphaData)
	{
		m_pAlpha = new unsigned char[pData->sizeAlphaData];
		memcpy(m_pAlpha, p, pData->sizeAlphaData);
		p += pData->sizeAlphaData;
	}
	else
		m_pAlpha = NULL;
}

CSurface::CSurface(unsigned short w, unsigned short h, int pitchInByte, void *pBuffer)
{
	CSurface *pSurface = new CSurface();

	ASSERT(pBuffer != NULL);

	m_bpp = COLOR_RGB565;
	m_flag = 0;
	m_width = w;
	m_height = h;
	m_pixelPitch = pitchInByte;
	m_alphaPitch = 0;
	m_ckSrc = 0;
	m_nPaletteEntries = 0;
	m_clipRect[0] = m_clipRect[1] = 0;
	m_clipRect[2] = w;
	m_clipRect[3] = h;
	m_pPixel = (unsigned char*)pBuffer;
	m_pAlpha = NULL;
	m_pPalette = NULL;
}

void CSurface::BlitSurfaceFast(int desX, int desY, CSurface *pSrc, int srcX, int srcY, int w, int h, int flag)
{
	unsigned char *pDesStart, *pSrcStart, *pAlphaStart;
	unsigned int translevel;

	ASSERT(m_bpp == COLOR_RGB565);
	translevel = (flag >> BLIT_TRANSLEVEL_SHIFT) & (BLIT_TRANSLEVEL_MASK >> BLIT_TRANSLEVEL_SHIFT);
	pDesStart = m_pPixel + desY * m_pixelPitch + desX * 2;
	flag &= 0xff;
	if(pSrc->m_bpp == 8)
	{
		pSrcStart = pSrc->m_pPixel + srcY * pSrc->m_pixelPitch + srcX;
		if(pSrc->m_flag & SURFACE_ALPHA)
		{
			pAlphaStart = pSrc->m_pAlpha + srcY * pSrc->m_alphaPitch + srcX;
			if(translevel)
				(*g_blitTransAlphaFunc8To16[flag])(this, pDesStart, pSrc, pSrcStart, w, h, pAlphaStart, 32 - translevel);
			else
				(*g_blitAlphaFunc8To16[flag])(this, pDesStart, pSrc, pSrcStart, w, h, pAlphaStart);
		}
		else
		{
			if(translevel)
				(*g_blitTransFunc8To16[flag])(this, pDesStart, pSrc, pSrcStart, w, h, 32 - translevel);
			else
				(*g_blitFunc8To16[flag])(this, pDesStart, pSrc, pSrcStart, w, h);
		}
	}
	else if(pSrc->m_bpp == 16)
	{
		pSrcStart = pSrc->m_pPixel + srcY * pSrc->m_pixelPitch + srcX * 2;
		if(pSrc->m_flag & SURFACE_ALPHA)
		{
			pAlphaStart = pSrc->m_pAlpha + srcY * pSrc->m_alphaPitch + srcX;
			if(translevel)
				(*g_blitTransAlphaFunc16To16[flag])(this, pDesStart, pSrc, pSrcStart, w, h, pAlphaStart, 32 - translevel);
			else
				(*g_blitAlphaFunc16To16[flag])(this, pDesStart, pSrc, pSrcStart, w, h, pAlphaStart);
		}
		else
		{
			if(translevel)
				(*g_blitTransFunc16To16[flag])(this, pDesStart, pSrc, pSrcStart, w, h, 32 - translevel);
			else
				(*g_blitFunc16To16[flag])(this, pDesStart, pSrc, pSrcStart, w, h);
		}
	}
}

void CSurface::BlitSurface(int desX, int desY, CSurface *pSrc, int srcX, int srcY, int w, int h, int flag)
{
	int d;

	ASSERT((srcX + w) <= pSrc->m_width && (srcY + h) <= pSrc->m_height);

	d = desX - m_clipRect[0];
	if(d < 0)
	{
		if(!(flag & BLIT_XFLIP))
			srcX -= d;
		w += d;
		desX = m_clipRect[0];
	}
	d = desX + w - m_clipRect[2];
	if(d > 0)
	{
		if(flag & BLIT_XFLIP)
			srcX += d;
		w -= d;
	}
	d = desY - m_clipRect[1];
	if(d < 0)
	{
		if(!(flag & BLIT_YFLIP))
			srcY -= d;
		h += d;
		desY = m_clipRect[1];
	}
	d = desY + h - m_clipRect[3];
	if(d > 0)
	{
		if(flag & BLIT_YFLIP)
			srcY += d;
		h -= d;
	}
	if(w > 0 && h > 0)
		BlitSurfaceFast(desX, desY, pSrc, srcX, srcY, w, h, flag);
}

void CSurface::BlitSlice(int desX, int desY, CSurface *pSrc, int idSlice, int flag)
{
	SLICE *pSlice;

	ASSERT(idSlice < pSrc->m_nSlices);
	pSlice = pSrc->m_pSlices + idSlice;
	BlitSurface(desX, desY, pSrc, pSlice->x, pSlice->y, pSlice->w, pSlice->h, pSlice->flag | flag);
}

void CSurface::BlitSliceFast(int desX, int desY, CSurface *pSrc, int idSlice, int flag)
{
	SLICE *pSlice;

	ASSERT(idSlice < pSrc->m_nSlices);
	pSlice = pSrc->m_pSlices + idSlice;
	BlitSurfaceFast(desX, desY, pSrc, pSlice->x, pSlice->y, pSlice->w, pSlice->h, pSlice->flag | flag);
}

unsigned short CSurface::GetPixel(int x, int y)
{
	ASSERT(m_bpp == 16);
	return *(unsigned short*)(m_pPixel + m_pixelPitch * y + x * 2);
}
