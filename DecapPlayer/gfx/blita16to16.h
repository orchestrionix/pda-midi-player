#ifndef __BLITA16TO16_H__
#define __BLITA16TO16_H__

typedef void (*GFXBLITALPHAFUNC16TO16)(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);

void Gfx_BlitA16To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);
void Gfx_BlitA16To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);
void Gfx_BlitA16To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);
void Gfx_BlitA16To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData);

typedef void (*GFXBLITTRANSALPHAFUNC16TO16)(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA16To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA16To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA16To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);
void Gfx_BlitTA16To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned char *pAlphaData, unsigned int nOpacity);

typedef void (*GFXBLITTRANSFUNC16TO16)(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT16To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT16To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT16To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT16To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT16To16T(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT16To16TFlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT16To16TFlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);
void Gfx_BlitT16To16TFlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h, unsigned int nOpacity);

#endif//__BLITA16TO16_H__