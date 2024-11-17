// DlgBase.cpp: implementation of the CDlgBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgBase.h"
#include <aygshell.h>
#include <sipapi.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgBase::CDlgBase(LPCTSTR pTemplate, LPCTSTR pTemplateLS)
{
	m_hWnd = NULL;
	m_lpszTemplate = pTemplate;
	if(pTemplateLS)
		m_lpszTemplateLS = pTemplateLS;
	else
		m_lpszTemplateLS = m_lpszTemplate;
}

CDlgBase::~CDlgBase()
{

}

LRESULT CALLBACK CDlgBase::DlgCallBack(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CDlgBase *p;
	if(message == WM_INITDIALOG)
	{
		p = (CDlgBase*)lParam;
		p->m_hWnd = hDlg;
		SetWindowLong(hDlg, DWL_USER, lParam);
	}
	else
	{
		p = (CDlgBase*)GetWindowLong(hDlg, DWL_USER);
		if(!p)
			return FALSE;
	}
	return p->WndProc(message, wParam, lParam);
}

LRESULT CDlgBase::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		OnInitDialog();
		return TRUE;
	case WM_COMMAND:
		return OnCommand(LOWORD(wParam), HIWORD(wParam));
	case WM_NOTIFY:
		return OnNotify((int)wParam, (LPNMHDR)lParam);
	}
	return FALSE;
}

LRESULT CDlgBase::OnCommand(int idCtl, int idNotify)
{
	if(idCtl == IDOK)
	{
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	}
	return FALSE;
}

int CDlgBase::ShowModal(HWND hWnd)
{
	return DialogBoxParam(GetModuleHandle(NULL), IsLandScapeMode() ? m_lpszTemplateLS : m_lpszTemplate, hWnd, (DLGPROC)DlgCallBack, (LPARAM)this);
}

void CDlgBase::OnInitDialog()
{
	SHINITDLGINFO shidi;

	// Create a Done button and size it.  
	shidi.dwMask = SHIDIM_FLAGS;
	shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
	shidi.hDlg = m_hWnd;
	SHInitDialog(&shidi);
}

BOOL CDlgBase::OnNotify(int idCtl, LPNMHDR pHdr)
{
	return FALSE;
}
