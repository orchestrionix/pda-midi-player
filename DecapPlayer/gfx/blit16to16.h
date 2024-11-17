#ifndef __BLIT16TO16_H__
#define __BLIT16TO16_H__

typedef void (*GFXBLITFUNC16TO16)(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);

void Gfx_Blit16To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit16To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit16To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit16To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);

void Gfx_Blit16To16T(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit16To16TFlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit16To16TFlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit16To16TFlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);

#endif//__BLIT16TO16_H__