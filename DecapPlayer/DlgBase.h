// DlgBase.h: interface for the CDlgBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGBASE_H__D8CB3933_7C2C_48C8_94A5_A3A7CCFF038E__INCLUDED_)
#define AFX_DLGBASE_H__D8CB3933_7C2C_48C8_94A5_A3A7CCFF038E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern int g_screenWidth, g_screenHeight;
inline bool IsLandScapeMode(){
	return g_screenWidth > g_screenHeight;
}

class CDlgBase  
{
public:
	virtual void OnInitDialog();
	int ShowModal(HWND hWnd);
	virtual LRESULT OnCommand(int idCtl, int idNotify);
	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
	CDlgBase(LPCTSTR pTemplate, LPCTSTR pTemplateLS = NULL);
	virtual ~CDlgBase();

	static LRESULT CALLBACK DlgCallBack(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	virtual BOOL OnNotify(int idCtl, LPNMHDR pHdr);
	HWND m_hWnd;
	LPCTSTR m_lpszTemplate, m_lpszTemplateLS;
};

#endif // !defined(AFX_DLGBASE_H__D8CB3933_7C2C_48C8_94A5_A3A7CCFF038E__INCLUDED_)
