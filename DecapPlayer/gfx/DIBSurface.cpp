// DIBSurface.cpp: implementation of the CDIBSurface class.
//
//////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DIBSurface.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDIBSurface::CDIBSurface(int w, int h)
{
	void*pV;
	HDC hdc;
	HBITMAP hbmp;
	BYTE buffer[sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD)];

	m_hdc = NULL;
	m_hbmp = NULL;

	ASSERT((w & 3) == 0);
	// Handy pointers
	BITMAPINFO*       pBMI    = (BITMAPINFO*) buffer;
	BITMAPINFOHEADER* pHeader = &pBMI->bmiHeader;
	DWORD*            pColors = (DWORD*)&pBMI->bmiColors;   

	// DIB Header
	pHeader->biSize            = sizeof(BITMAPINFOHEADER);
	pHeader->biWidth           = w;
	pHeader->biHeight          = -h;
	pHeader->biPlanes          = 1;
	pHeader->biBitCount        = 16;
	pHeader->biCompression     = BI_BITFIELDS;
	pHeader->biSizeImage       = (w * h * 16) / 8;
	pHeader->biXPelsPerMeter   = 0;
	pHeader->biYPelsPerMeter   = 0;
	pHeader->biClrUsed         = 0;
	pHeader->biClrImportant    = 0;

	pColors[0] = 0x1F << 11;
	pColors[1] = 0x3F << 5;
	pColors[2] = 0x1F;

	// Create the DIB
	hdc = CreateCompatibleDC(NULL);
	hbmp = CreateDIBSection(hdc, pBMI, DIB_RGB_COLORS, &pV, 0, 0);

	SelectObject(hdc, hbmp);
	m_bpp = COLOR_RGB565;
	m_hdc = hdc;
	m_hbmp = hbmp;
	m_pPixel = (unsigned char*)pV;
	m_alphaPitch = NULL;
	m_ckSrc = 0;
	m_clipRect[0] = m_clipRect[1] = 0;
	m_clipRect[2] = w;
	m_clipRect[3] = h;
	m_flag = 0;
	m_width = w;
	m_height = h;
	m_nPaletteEntries = 0;
	m_nSlices = 0;
	m_pAlpha = NULL;
	m_pixelPitch = w * 2;
	m_pPalette = NULL;
	m_pSlices = NULL;
}

CDIBSurface::~CDIBSurface()
{
	if(m_hdc)
		DeleteDC(m_hdc);
	if(m_hbmp)
		DeleteObject(m_hbmp);
	m_pPixel = NULL;
}
