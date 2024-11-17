// Skin.h: interface for the CSkin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKIN_H__8007E19A_8A0C_40A5_9EF5_E72141C6CE11__INCLUDED_)
#define AFX_SKIN_H__8007E19A_8A0C_40A5_9EF5_E72141C6CE11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gfx/DIBSurface.h"

class CSkin  
{
public:
	enum {
		CTL_NONE,

		CTL_POSITION,
		CTL_VOLUME,

		CTL_PLAY,
		CTL_PAUSE,
		CTL_STOP,
		CTL_PREV,
		CTL_NEXT,

		CTL_PLAYLIST,
		CTL_PLAYLIST_UP,
		CTL_PLAYLIST_DOWN,
		CTL_PLAYLIST_SCROLL,

		CTL_TIME_PAN,
		CTL_TITLE
	};
	void PlaylistSelectItem(int i);
	int PlaylistItemFromPos(int x, int y);
	void PlaylistHitScroll(int x, int y);
	void PlaylistLineUp();
	void PlaylistLineDown();
	int PlaylistGetNext() const {
		return m_plNextItem;
	}
	void PlaylistSetNext(int next) {
		m_plNextItem = next;
		DrawPlayList();
	}
	void PlaylistReset() {
		m_plNextItem = -1;
		m_plTopItem = 0;
		m_plHotItem = -1;
		DrawPlayList();
	}
	void DrawPlayList();
	void DrawChannelNumbers();
	void DrawTimer(int sec, bool isTimeFormat);
	int HandleMouseForBar(int idCtl, int mx, int my, int maxval);
	int CtlFromPos(int x, int y);
	void SetTitle(LPCTSTR title);
	void SetButton(int idCtl, bool pressed);
	void SetBar(int idCtl, int curValue, int maxValue);
	void BlitToScreen(HDC hdc);
	void SetPosition(int curValue, int maxValue);
	void BlitToScreen(HDC hdc, int x, int y, int w, int h);
	bool OpenSkin(LPCTSTR fn);
	void DrawMeter(short *pVals);
	void DrawSwitch(int idSwitch, int idEnabled, bool enabled);
	CSkin();
	virtual ~CSkin();

	void SetUpdate(bool needUpdate){
		m_needUpdate = needUpdate;
	}

	TCHAR m_skinfile[MAX_PATH];
protected:
	void DrawPlayListText();
	void DrawDigit(int n, int id);
	void DrawButton(int idBtn, int idPressed, bool pressed);
	void DrawProgressBar(int idBar, int idProg, int curValue, int maxValue);
	void Invalidate(int l, int t, int r, int b);
	void UpdateBG(int idSlice);
	void UpdateBG(int x, int y, int w, int h);
	void ReleaseAll();
	CDIBSurface *m_pBkBuf;
	CSurface* m_pBg, *m_pCtls;
	int m_rcUpdate[4];
	unsigned short m_waveClr, m_titleClr, m_plTextClr, m_plSelClr;
	int m_lastTimer;
	bool m_needUpdate;

	int m_plTopItem;
	int m_plHotItem;
	int m_plNextItem;
	int m_nVisuablePlItem;

	int m_lastPosition;
};

extern CSkin g_skin;
#endif // !defined(AFX_SKIN_H__8007E19A_8A0C_40A5_9EF5_E72141C6CE11__INCLUDED_)
