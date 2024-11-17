#ifndef __BLITA8TO16_H__
#define __BLITA8TO16_H__

typedef void (*GFXBLITALPHAFUNC8TO16)(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);

void Gfx_BlitA8To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);
void Gfx_BlitA8To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);
void Gfx_BlitA8To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);
void Gfx_BlitA8To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);

typedef void (*GFXBLITTRANSALPHAFUNC8TO16)(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA8To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA8To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA8To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA8To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);

typedef void (*GFXBLITTRANSFUNC8TO16)(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT8To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT8To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT8To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT8To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT8To16T(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT8To16TFlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT8To16TFlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT8To16TFlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);


#endif//__BLITA8TO16_H__