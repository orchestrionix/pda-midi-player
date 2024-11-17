#ifndef __BLIT8TO16_H__
#define __BLIT8TO16_H__

typedef void (*GFXBLITFUNC8TO16)(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);

void Gfx_Blit8To16(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit8To16FlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit8To16FlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit8To16FlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);

void Gfx_Blit8To16T(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit8To16TFlipX(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit8To16TFlipY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);
void Gfx_Blit8To16TFlipXY(CSurface *pDes, void *pDesStart, CSurface *pSrc, void *pSrcStart, int w, int h);

#endif//__BLIT8TO16_H__