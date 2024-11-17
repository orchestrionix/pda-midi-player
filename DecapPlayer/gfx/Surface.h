// Surface.h: interface for the CSurface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SURFACE_H__69528B5C_624B_4563_BE85_02A39F591220__INCLUDED_)
#define AFX_SURFACE_H__69528B5C_624B_4563_BE85_02A39F591220__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSurface  
{
public:
	unsigned short GetPixel(int x, int y);
	void BlitSliceFast(int desX, int desY, CSurface *pSrc, int idSlice, int flag = 0);
	void BlitSlice(int desX, int desY, CSurface *pSrc, int idSlice, int flag);
	void BlitSurface(int desX, int desY, CSurface *pSrc, int srcX, int srcY, int w, int h, int flag);
	void BlitSurfaceFast(int desX, int desY, CSurface *pSrc, int srcX, int srcY, int w, int h, int flag = 0);

	enum {
		COLOR_PAL8 = 8,
		COLOR_RGB565 = 16
	};
	enum {
		BLIT_XFLIP = 1,
		BLIT_YFLIP = 2,
		BLIT_TRANS = 4,
		BLIT_SHADOW = 8,
		BLIT_SCREEN = 0x4000,

		BLIT_TRANSLEVEL_MASK = 0x3f00,
		BLIT_TRANSLEVEL_SHIFT = 8,

		MAX_TRANSLEVEL = 32
	};
	
	//GFXSURFACE & GFXSURFACEDATA
	enum {
		SURFACE_ALPHA = 1,
		SURFACE_CKSRC = 2,
	};
	
	typedef struct {
		unsigned short flag;//GFX_BLIT_XX
		unsigned short x, y, w, h;
	}SLICE;
	
	typedef struct {
		unsigned short	bpp;
		unsigned short	flag;
		unsigned short	width, height;
		unsigned short	pixelPitch, alphaPitch;
		int				ckSrc, ckShadow;
		unsigned short	nSlices;
		unsigned short	nPaletteEntries;
		unsigned int	sizeSliceData;
		unsigned int	sizePaletteData;
		unsigned int	sizePixelData;
		unsigned int	sizeAlphaData;
	}SURFACEDATA;

	CSurface();
	CSurface(void* pSurfaceData);
	CSurface(unsigned short w, unsigned short h, int pitchInByte, void* pBuffer);
	virtual ~CSurface();

	SLICE *GetSlice(int id){
		return m_pSlices + id;
	}
	
	int GetWidth(){
		return m_width;
	}

	int GetHeight(){
		return m_height;
	}

	unsigned short	m_bpp;
	unsigned short	m_flag;
	unsigned short	m_width;
	unsigned short	m_height;
	int				m_pixelPitch;
	int				m_alphaPitch;
	int				m_ckSrc;//Source color key
	unsigned short	m_nSlices;
	unsigned short	m_nPaletteEntries;
	unsigned short	m_clipRect[4];//left, top, right, bottom (left and top is inclusive, right and bottom is exclusive)
	SLICE			*m_pSlices;
	unsigned char	*m_pPalette;
	unsigned char	*m_pPixel;
	unsigned char	*m_pAlpha;
};

#endif // !defined(AFX_SURFACE_H__69528B5C_624B_4563_BE85_02A39F591220__INCLUDED_)
