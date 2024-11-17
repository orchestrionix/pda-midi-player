// DlgPlEdit.h: interface for the CDlgPlEdit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGPLEDIT_H__6D174D41_9424_4912_8A4B_051456CDAB91__INCLUDED_)
#define AFX_DLGPLEDIT_H__6D174D41_9424_4912_8A4B_051456CDAB91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DlgBase.h"
#include "playlist.h"

class CDlgPlEdit : public CDlgBase  
{
public:
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgPlEdit(CPlayList *pList);
	virtual ~CDlgPlEdit();

protected:
	void UpdateButton();
	CPlayList *m_pList;
};

class CDlgPlOpen : public CDlgBase  
{
public:
	CDlgPlOpen();
	virtual ~CDlgPlOpen();
	LRESULT OnCommand(int idCtl, int idNotify);
	void OnInitDialog();
	void FreeListData();
	void FindPlaylist(LPCTSTR path);
	LPTSTR GetSelected() {
		return m_selected;
	}

protected:
	void UpdateButton();
	TCHAR m_selected[MAX_PATH];
};

class CDlgSkin : public CDlgBase  
{
public:
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgSkin(LPCTSTR fn);
	virtual ~CDlgSkin();

	TCHAR m_file[MAX_PATH];
protected:
};
#endif // !defined(AFX_DLGPLEDIT_H__6D174D41_9424_4912_8A4B_051456CDAB91__INCLUDED_)
