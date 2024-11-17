 // Skin.cpp: implementation of the CSkin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Skin.h"
#include "skindata.h"
#include "playlist.h"
#include "player.h"
#include "resmgr/resmgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define PLAYLIST_ITEM_HEIGHT 16

CSkin g_skin;

static CResMgr g_resmgr;

CSkin::CSkin()
{
	m_pBg = NULL;
	m_pCtls = NULL;
	m_pBkBuf = NULL;
	m_rcUpdate[0] = m_rcUpdate[1] = m_rcUpdate[2] = m_rcUpdate[3] = 0;
	m_lastTimer = -1;
	m_lastPosition = -1;
	m_skinfile[0] = 0;
	m_needUpdate = true;
	m_plTopItem = 0;
	m_plHotItem = -1;
	m_plNextItem = -1;
}

CSkin::~CSkin()
{
	ReleaseAll();
}

bool CSkin::OpenSkin(LPCTSTR fn)
{
	g_resmgr.CloseFile(0);
	if(!g_resmgr.OpenFile(fn, 0))
		return false;
	ReleaseAll();
	lstrcpy(m_skinfile, fn);
	m_pBg = new CSurface(g_resmgr.OpenRecord(DECAP_BG_SUF));
	m_pCtls = new CSurface(g_resmgr.OpenRecord(DECAP_CTLS_SUF));
	m_pBkBuf = new CDIBSurface(m_pBg->GetWidth(), m_pBg->GetHeight());
	UpdateBG(0, 0, m_pBkBuf->GetWidth(), m_pBkBuf->GetHeight());
	m_rcUpdate[0] = m_rcUpdate[1] = 0;
	m_rcUpdate[2] = m_pBkBuf->GetWidth();
	m_rcUpdate[3] = m_pBkBuf->GetHeight();

	CSurface::SLICE *pSlice = m_pCtls->GetSlice(CTLS_XML_wave);
	m_waveClr = m_pCtls->GetPixel(pSlice->x, pSlice->y);
	pSlice = m_pCtls->GetSlice(CTLS_XML_titleclr);
	m_titleClr = m_pCtls->GetPixel(pSlice->x, pSlice->y);

	pSlice = m_pCtls->GetSlice(CTLS_XML_list_text);
	m_plTextClr = m_pCtls->GetPixel(pSlice->x, pSlice->y);
	pSlice = m_pCtls->GetSlice(CTLS_XML_list_sel);
	m_plSelClr = m_pCtls->GetPixel(pSlice->x, pSlice->y);
	m_nVisuablePlItem = m_pBg->GetSlice(BG_XML_list)->h / PLAYLIST_ITEM_HEIGHT;

	DrawPlayList();
	return true;
}

void CSkin::ReleaseAll()
{
	if(m_pBkBuf)
	{
		delete m_pBkBuf;
		m_pBkBuf = NULL;
	}
	if(m_pBg)
	{
		delete m_pBg;
		m_pBg = NULL;
	}
	if(m_pCtls)
	{
		delete m_pCtls;
		m_pCtls = NULL;
	}
}

void CSkin::UpdateBG(int x, int y, int w, int h)
{
	m_pBkBuf->BlitSurfaceFast(x, y, m_pBg, x, y, w, h);
	Invalidate(x, y, w+x, y+h);
}

void CSkin::UpdateBG(int idSlice)
{
	CSurface::SLICE *pSlice = m_pBg->GetSlice(idSlice);
	m_pBkBuf->BlitSurfaceFast(pSlice->x, pSlice->y, m_pBg, pSlice->x, pSlice->y, pSlice->w, pSlice->h);
	Invalidate(pSlice->x, pSlice->y, pSlice->w+pSlice->x, pSlice->y+pSlice->h);
}

void CSkin::BlitToScreen(HDC hdc, int x, int y, int w, int h)
{
	if(m_pBkBuf)
		BitBlt(hdc, x, y, w, h, m_pBkBuf->GetDC(), x, y, SRCCOPY);
}

void CSkin::BlitToScreen(HDC hdc)
{
	int w, h;

	w = m_rcUpdate[2] - m_rcUpdate[0];
	h = m_rcUpdate[3] - m_rcUpdate[1];
	if(w > 0 && h > 0)
	{
		BitBlt(hdc, m_rcUpdate[0], m_rcUpdate[1], w, h, m_pBkBuf->GetDC(), m_rcUpdate[0], m_rcUpdate[1], SRCCOPY);
		m_rcUpdate[1] = m_rcUpdate[0] = 1000;
		m_rcUpdate[2] = m_rcUpdate[3] = 0;
	}
}

void CSkin::Invalidate(int l, int t, int r, int b)
{
	if(m_rcUpdate[0] > l) m_rcUpdate[0] = l;
	if(m_rcUpdate[1] > t) m_rcUpdate[1] = t;
	if(m_rcUpdate[2] < r) m_rcUpdate[2] = r;
	if(m_rcUpdate[3] < b) m_rcUpdate[3] = b;
}

void CSkin::DrawProgressBar(int idBar, int idProg, int curValue, int maxValue)
{
	if(!m_needUpdate)
		return;
	int x;
	CSurface::SLICE *pSliceBar = m_pBg->GetSlice(idBar);
	if(pSliceBar->w == 1)
		return;
	CSurface::SLICE *pSliceProg = m_pCtls->GetSlice(idProg);
	if(maxValue == 0)
		x = 0;
	else
		x = curValue * (pSliceBar->w - pSliceProg->w) / maxValue;
	if(x + pSliceProg->w > pSliceBar->w)
		x = pSliceBar->w - pSliceProg->w;
	ASSERT(pSliceBar->h == pSliceProg->h);
	UpdateBG(pSliceBar->x, pSliceBar->y, pSliceBar->w, pSliceBar->h);
	m_pBkBuf->BlitSliceFast(x + pSliceBar->x, pSliceBar->y, m_pCtls, idProg);
}

void CSkin::DrawSwitch(int idSwitch, int idEnabled, bool enabled)
{
	CSurface::SLICE *pSlice = m_pBg->GetSlice(idSwitch);
	if(enabled)
	{
		m_pBkBuf->BlitSliceFast(pSlice->x, pSlice->y, m_pCtls, idEnabled);
		Invalidate(pSlice->x, pSlice->y, pSlice->w+pSlice->x, pSlice->y+pSlice->h);
	}
	else
		UpdateBG(pSlice->x, pSlice->y, pSlice->w, pSlice->h);
}

void CSkin::DrawButton(int idBtn, int idPressed, bool pressed)
{
	CSurface::SLICE *pSlice = m_pBg->GetSlice(idBtn);
	if(pressed)
	{
		m_pBkBuf->BlitSliceFast(pSlice->x, pSlice->y, m_pCtls, idPressed);
		Invalidate(pSlice->x, pSlice->y, pSlice->w+pSlice->x, pSlice->y+pSlice->h);
	}
	else
		UpdateBG(pSlice->x, pSlice->y, pSlice->w, pSlice->h);
}

void CSkin::SetBar(int idCtl, int curValue, int maxValue)
{
	switch(idCtl) {
	case CTL_POSITION:
		if(m_lastPosition != curValue)
		{
			m_lastPosition = curValue;
			DrawProgressBar(BG_XML_position, CTLS_XML_progress, curValue, maxValue);
		}
		break;
	case CTL_VOLUME:
		DrawProgressBar(BG_XML_volume, CTLS_XML_volume, curValue, maxValue);
		break;
	default:
		ASSERT(0);
	}
}

void CSkin::SetButton(int idCtl, bool pressed)
{
	switch(idCtl) {
	case CTL_PLAY:
		DrawButton(BG_XML_play, CTLS_XML_play, pressed);
		break;
	case CTL_PAUSE:
		DrawButton(BG_XML_pause, CTLS_XML_pause, pressed);
		break;
	case CTL_STOP:
		DrawButton(BG_XML_stop, CTLS_XML_stop, pressed);
		break;
	case CTL_PREV:
		DrawButton(BG_XML_prev, CTLS_XML_prev, pressed);
		break;
	case CTL_NEXT:
		DrawButton(BG_XML_next, CTLS_XML_next, pressed);
		break;
	default:
		ASSERT(0);
	}
}

void CSkin::SetTitle(LPCTSTR title)
{
	CSurface::SLICE *pSlice = m_pBg->GetSlice(BG_XML_title);
	RECT rc;

	rc.left = pSlice->x;
	rc.top = pSlice->y;
	rc.right = pSlice->x + pSlice->w;
	rc.bottom = pSlice->y + pSlice->h;

	UpdateBG(BG_XML_title);
	if(title[0])
	{
		SetBkMode(m_pBkBuf->GetDC(), TRANSPARENT);
		SetTextColor(m_pBkBuf->GetDC(), RGB((m_titleClr >> 8) & 0xf8, (m_titleClr >> 3) & 0xf8 , (m_titleClr << 3) & 0xf8));
		DrawText(m_pBkBuf->GetDC(), title, -1, &rc, DT_LEFT | DT_VCENTER);
	}
}

int CSkin::CtlFromPos(int x, int y)
{
	int i;
	CSurface::SLICE *pSlice;
	for(i = BG_XML_position; i <= BG_XML_title; i++)
	{
		pSlice = m_pBg->GetSlice(i);
		if(x >= pSlice->x && x < pSlice->x + pSlice->w && y >= pSlice->y && y < pSlice->y + pSlice->h)
			return CTL_POSITION + i;
	}
	return CTL_NONE;
}

int CSkin::HandleMouseForBar(int idCtl, int mx, int my, int maxval)
{
	int x, w;
	CSurface::SLICE *pSlice;

	pSlice = m_pBg->GetSlice(idCtl - CTL_POSITION + BG_XML_position);
	w = m_pCtls->GetSlice(CTLS_XML_progress)->w;
	if(mx <= pSlice->x + w/2)
		x = 0;
	else if(mx >= pSlice->x + pSlice->w - w/2)
		x = maxval;
	else
	{
		x = maxval * (mx - pSlice->x - w/2)/(pSlice->w - w);
	}
	SetBar(idCtl, x, maxval);
	return x;
}

void CSkin::DrawTimer(int sec, bool isTimeFormat)
{
	if(!m_needUpdate)
		return;
	if(m_lastTimer != sec)
	{
		m_lastTimer = sec;
		if(sec == 0x10000000)
		{
			DrawDigit(-1, BG_XML_time0);
			DrawDigit(-1, BG_XML_time1);
			DrawDigit(-1, BG_XML_time2);
			DrawDigit(-1, BG_XML_time3);
			DrawDigit(10, BG_XML_time4);
		}
		else if(sec == 0x10000001)
		{
			UpdateBG(BG_XML_time0);
			UpdateBG(BG_XML_time1);
			UpdateBG(BG_XML_time2);
			UpdateBG(BG_XML_time3);
			UpdateBG(BG_XML_time4);
		}
		else
		{
			if(sec < 0)
			{
				DrawDigit(-1, BG_XML_time5);
				sec = -sec;
			}
			else
				UpdateBG(BG_XML_time5);
			if(isTimeFormat)
			{
				int m = sec / 60;
				int s = sec % 60;

				DrawDigit(m / 10, BG_XML_time0);
				DrawDigit(m % 10, BG_XML_time1);
				DrawDigit(s / 10, BG_XML_time2);
				DrawDigit(s % 10, BG_XML_time3);
				DrawDigit(10, BG_XML_time4);
			}
			else
			{
				UpdateBG(BG_XML_time0);
				UpdateBG(BG_XML_time1);
				DrawDigit(sec / 10, BG_XML_time2);
				DrawDigit(sec % 10, BG_XML_time3);
				UpdateBG(BG_XML_time4);
			}
		}
	}
}

void CSkin::DrawDigit(int n, int id)
{
	CSurface::SLICE *pSlice = m_pBg->GetSlice(id);
	UpdateBG(id);
	m_pBkBuf->BlitSliceFast(pSlice->x, pSlice->y, m_pCtls, n + CTLS_XML_dig0);
}

void CSkin::DrawMeter(short *pVals)
{
	if(!m_needUpdate)
		return;
	CSurface::SLICE *pMeter = m_pCtls->GetSlice(CTLS_XML_meter);

	for(int i = 0; i < 16; i++)
	{
		CSurface::SLICE *pBg = m_pBg->GetSlice(BG_XML_cv1 + i);
		int h = pMeter->h * pVals[i] / 8192;
		if(h > pMeter->h)
			h = pMeter->h;
		int y = pMeter->h - h;
		UpdateBG(BG_XML_cv1 + i);
		m_pBkBuf->BlitSurfaceFast(pBg->x, pBg->y + y, m_pCtls, pMeter->x, pMeter->y + y, pMeter->w, h);
	}
}

void CSkin::DrawChannelNumbers()
{
	for(int i = 0; i < 16; i++)
	{
		if(g_player.IsMute(i))
			UpdateBG(BG_XML_cn1 + i);
		else
		{
			CSurface::SLICE *pSlice = m_pBg->GetSlice(BG_XML_cn1 + i);
			m_pBkBuf->BlitSliceFast(pSlice->x, pSlice->y, m_pCtls, i + CTLS_XML_cn1);
		}
	}
}

void CSkin::DrawPlayListText()
{
	UpdateBG(BG_XML_list);
	SetBkMode(m_pBkBuf->GetDC(), TRANSPARENT);
	SetTextColor(m_pBkBuf->GetDC(), RGB((m_plTextClr >> 8) & 0xf8, (m_plTextClr >> 3) & 0xf8 , (m_plTextClr << 3) & 0xf8));

	CSurface::SLICE *pSlice = m_pBg->GetSlice(BG_XML_list);
	if(pSlice->h == 1)
		return;
	RECT rc;

	rc.left = pSlice->x;
	rc.top = pSlice->y;
	rc.right = pSlice->x + pSlice->w;
	rc.bottom = pSlice->y + PLAYLIST_ITEM_HEIGHT;
	int ndisp = g_playlist.GetItemCount() - m_plTopItem;
	if(ndisp > m_nVisuablePlItem)
		ndisp = m_nVisuablePlItem;
	int i;
	TCHAR s[0x100];
	HDC dc = m_pBkBuf->GetDC();
	for(i = 0; i < ndisp; i++)
	{
		if(i + m_plTopItem == m_plHotItem)
		{
			HBRUSH br = CreateSolidBrush(RGB((m_plSelClr >> 8) & 0xf8, (m_plSelClr >> 3) & 0xf8 , (m_plSelClr << 3) & 0xf8));
			FillRect(dc, &rc, br);
		}
		else if(i + m_plTopItem == m_plNextItem)
		{
			HBRUSH br = CreateSolidBrush(RGB(0, 120, 40));
			FillRect(dc, &rc, br);
		}
		wsprintf(s, TEXT("%d. %s"), g_playlist.GetShuffledOrder(i + m_plTopItem) + 1, g_playlist.GetDisplayName(i + m_plTopItem));
		DrawText(dc, s, -1, &rc, DT_LEFT | DT_TOP);
		rc.top += PLAYLIST_ITEM_HEIGHT;
		rc.bottom += PLAYLIST_ITEM_HEIGHT;
	}
}

void CSkin::DrawPlayList()
{
	int ndiff = g_playlist.GetItemCount() - m_nVisuablePlItem;
	if(ndiff > 0)
	{
		if(m_plTopItem >= ndiff)
			m_plTopItem = ndiff;
		UpdateBG(BG_XML_list_scroll);
		CSurface::SLICE *pScroll = m_pBg->GetSlice(BG_XML_list_scroll);
		if(pScroll->h == 1)
			return;
		CSurface::SLICE *pThumb = m_pCtls->GetSlice(CTLS_XML_thumb);
		int thumb_gap = ((pScroll->h - pThumb->h) << 12) / ndiff;
		m_pBkBuf->BlitSliceFast(pScroll->x, pScroll->y + ((thumb_gap * m_plTopItem + (1 << 11)) >> 12), m_pCtls, CTLS_XML_thumb);
	}
	DrawPlayListText();
}

void CSkin::PlaylistLineDown()
{
	if(m_plTopItem + m_nVisuablePlItem < g_playlist.GetItemCount())
	{
		m_plTopItem++;
		DrawPlayList();
	}
}

void CSkin::PlaylistLineUp()
{
	if(m_plTopItem > 0)
	{
		m_plTopItem--;
		DrawPlayList();
	}
}

void CSkin::PlaylistHitScroll(int x, int y)
{
	int ndiff = g_playlist.GetItemCount() - m_nVisuablePlItem;
	if(ndiff <= 0)
		return;
	CSurface::SLICE *pScroll = m_pBg->GetSlice(BG_XML_list_scroll);
	CSurface::SLICE *pThumb = m_pCtls->GetSlice(CTLS_XML_thumb);
	y -= pScroll->y;
	if(y < 0)
		return;
	y -= pThumb->h / 2;
	int targetTopItem = ndiff * y / (pScroll->h - pThumb->h);
	if(targetTopItem > m_plTopItem + m_nVisuablePlItem)
		m_plTopItem += m_nVisuablePlItem;
	else if(targetTopItem < m_plTopItem - m_nVisuablePlItem)
		m_plTopItem -= m_nVisuablePlItem;
	else
		m_plTopItem = targetTopItem;
	if(m_plTopItem < 0)
		m_plTopItem = 0;
	else if(m_plTopItem > ndiff)
		m_plTopItem = ndiff;
	DrawPlayList();
}

int CSkin::PlaylistItemFromPos(int x, int y)
{
	CSurface::SLICE *pScroll = m_pBg->GetSlice(BG_XML_list);

	x -= pScroll->x;
	y -= pScroll->y;

	if(x < 0 || x >= pScroll->w || y < 0 || y >= pScroll->h)
		return -1;

	y = y / PLAYLIST_ITEM_HEIGHT + m_plTopItem;

	if(y >= g_playlist.GetItemCount())
		return -1;

	return y;
}

void CSkin::PlaylistSelectItem(int i)
{
	m_plNextItem = -1;
	if(m_plHotItem == i)
	{
		DrawPlayListText();
		return;
	}
	m_plHotItem = i;
	if(m_plHotItem < 0)
	{
		DrawPlayListText();
		return;
	}
	if(m_plHotItem < m_plTopItem)
		m_plTopItem = m_plHotItem;
	else if(m_plHotItem >= m_nVisuablePlItem + m_plTopItem)
		m_plTopItem = m_plHotItem - m_nVisuablePlItem + 1;
	DrawPlayList();
}
