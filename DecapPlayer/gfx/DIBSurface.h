// DIBSurface.h: interface for the CDIBSurface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBSURFACE_H__4C2704C4_5822_4CCA_BE54_6F4982D92AAF__INCLUDED_)
#define AFX_DIBSURFACE_H__4C2704C4_5822_4CCA_BE54_6F4982D92AAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Surface.h"

class CDIBSurface : public CSurface  
{
public:
	CDIBSurface(int w, int h);
	virtual ~CDIBSurface();
	HDC GetDC()
	{
		return m_hdc;
	}

protected:
	HDC m_hdc;
	HBITMAP m_hbmp;
};

#endif // !defined(AFX_DIBSURFACE_H__4C2704C4_5822_4CCA_BE54_6F4982D92AAF__INCLUDED_)
