// DlgPlEdit.cpp: implementation of the CDlgPlEdit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DlgPlEdit.h"
#include "Player.h"
#include "utils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgPlEdit::CDlgPlEdit(CPlayList *pList):CDlgBase((LPCTSTR)IDD_EDIT_PLAYLIST, (LPCTSTR)IDD_EDIT_PLAYLIST_LS)
{
	m_pList = pList;
}

CDlgPlEdit::~CDlgPlEdit()
{

}

LRESULT CDlgPlEdit::OnCommand(int idCtl, int idNotify)
{
	int i, n, c;
	switch(idCtl)
	{
	case IDC_PLAYLIST:
		if(idNotify == LBN_SELCHANGE)
			UpdateButton();
		else if(idNotify == LBN_DBLCLK)
		{
			n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSELCOUNT, 0, 0);
			if(n != 1) return TRUE;
			if(SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSELITEMS, 1, (LPARAM)&i) == 1)
			{
				if(g_player.Open(i))
				{
					g_player.Play();
					EndDialog(m_hWnd, IDC_PLAYLIST);
				}
			}
		}
		return TRUE;
	case IDC_DELETE:
		n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCOUNT, 0, 0);
		for(i = n - 1; i >= 0; i--)
		{
			if(SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, i, 0) > 0)
			{
				m_pList->Delete(i);
				SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_DELETESTRING, i, 0);
			}
		}
		UpdateButton();
		return TRUE;
	case IDC_MOVE_UP:
		n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCOUNT, 0, 0);
		if(n < 2)return TRUE;
		if(SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, 0, 0) > 0)
			return TRUE;
		for(i = 1; i < n; i++)
		{
			if(SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, i, 0) > 0)
			{
				c = i;
				while(i < n && SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, i, 0) > 0)
				{
					m_pList->Swap(i - 1, i);
					i++;
				}
				SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_DELETESTRING, c - 1, 0);
				SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_INSERTSTRING, i - 1, (LPARAM)m_pList->GetItemName(i - 1));
			}
		}
		UpdateButton();
		return TRUE;
	case IDC_MOVE_DOWN:
		n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCOUNT, 0, 0);
		if(n < 2)return TRUE;
		if(SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, n - 1, 0) > 0)
			return TRUE;
		for(i = n - 2; i >= 0; i--)
		{
			if(SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, i, 0) > 0)
			{
				c = i;
				while(i >= 0 && SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, i, 0) > 0)
				{
					m_pList->Swap(i + 1, i);
					i--;
				}
				SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_DELETESTRING, c + 1, 0);
				SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_INSERTSTRING, i + 1, (LPARAM)m_pList->GetItemName(i + 1));
			}
		}
		UpdateButton();
		return TRUE;
/*	case IDC_PLAY:
		n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSELCOUNT, 0, 0);
		if(n != 1) return TRUE;
		if(SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSELITEMS, 1, (LPARAM)&i) == 1)
		{
			if(g_player.Open(i))
			{
				g_player.Play();
				EndDialog(m_hWnd, IDC_PLAY);
			}
		}
		return TRUE;
*/	case IDC_SORT:
		g_playlist.Sort(true);
		SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_RESETCONTENT, 0, 0);
		for(i = 0; i < m_pList->GetItemCount(); i++)
		{
			SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_ADDSTRING, 0, (LPARAM)m_pList->GetItemName(i));
		}
		break;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgPlEdit::OnInitDialog()
{
	int i;

	CDlgBase::OnInitDialog();
	for(i = 0; i < m_pList->GetItemCount(); i++)
	{
		SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_ADDSTRING, 0, (LPARAM)m_pList->GetItemName(i));
	}
	SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_SETHORIZONTALEXTENT, 400, 0);
}

void CDlgPlEdit::UpdateButton()
{
	int ns = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSELCOUNT, 0, 0);
	int n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCOUNT, 0, 0);
	
//	EnableWindow(GetDlgItem(m_hWnd, IDC_SORT), n > 0);
	if(ns > 0 && n > 1)
	{
		EnableWindow(GetDlgItem(m_hWnd, IDC_MOVE_UP), SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, 0, 0) <= 0);
		EnableWindow(GetDlgItem(m_hWnd, IDC_MOVE_DOWN), SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETSEL, n - 1, 0) <= 0);
	}
	else
	{
		EnableWindow(GetDlgItem(m_hWnd, IDC_MOVE_UP), FALSE);
		EnableWindow(GetDlgItem(m_hWnd, IDC_MOVE_DOWN), FALSE);
	}
	EnableWindow(GetDlgItem(m_hWnd, IDC_DELETE), ns > 0);
}

extern bool g_showPathInPlaylist;

CDlgPlOpen::CDlgPlOpen():CDlgBase((LPCTSTR)IDD_OPEN_PLAYLIST, (LPCTSTR)IDD_OPEN_PLAYLIST_LS)
{
	m_selected[0] = 0;
}

CDlgPlOpen::~CDlgPlOpen()
{

}

LRESULT CDlgPlOpen::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDC_PLAYLIST:
		if(idNotify == LBN_SELCHANGE)
			UpdateButton();
		else if(idNotify == LBN_DBLCLK)
		{
			int n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCURSEL, 0, 0);
			if(n == LB_ERR) return TRUE;
			LPTSTR p = (LPTSTR)SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETITEMDATA, n, 0);
			lstrcpy(m_selected, p);
			FreeListData();
			EndDialog(m_hWnd, IDC_PLAYLIST);
		}
		return TRUE;
	case IDC_DELETE:
		{
			int n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCURSEL, 0, 0);
			if(n != LB_ERR)
			{
				if(MessageBox(m_hWnd, TEXT("Are you sure you want to permanently delete the selected playlist?"), TEXT("Delete Playlist"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					LPTSTR p = (LPTSTR)SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETITEMDATA, n, 0);
					lstrcpy(m_selected, p);
					DeleteFile(m_selected);
					m_selected[0] = 0;
					delete[] p;
					SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_DELETESTRING, n, 0);
					UpdateButton();
				}
			}
		}
		return TRUE;
	case IDC_SHOWPATH:
		{
			FreeListData();
			SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_RESETCONTENT, 0, 0);
			g_showPathInPlaylist = !g_showPathInPlaylist;
			FindPlaylist(TEXT(""));
			UpdateButton();
		}
		return TRUE;
	case IDOK:
		{
			int n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCURSEL, 0, 0);
			if(n != LB_ERR)
			{
				LPTSTR p = (LPTSTR)SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETITEMDATA, n, 0);
				lstrcpy(m_selected, p);
			}
		}
		FreeListData();
		EndDialog(m_hWnd, idCtl);
		break;
	case IDCANCEL:
		FreeListData();
		EndDialog(m_hWnd, idCtl);
		break;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgPlOpen::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_selected[0] = 0;
	FindPlaylist(TEXT(""));
	SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_SETHORIZONTALEXTENT, 400, 0);
}

void CDlgPlOpen::UpdateButton()
{
	int ns = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCURSEL, 0, 0);
	if(ns != LB_ERR)
	{
		EnableWindow(GetDlgItem(m_hWnd, IDOK), TRUE);
		EnableWindow(GetDlgItem(m_hWnd, IDC_DELETE), TRUE);
	}
	else
	{
		EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
		EnableWindow(GetDlgItem(m_hWnd, IDC_DELETE), FALSE);
	}
}

void CDlgPlOpen::FindPlaylist( LPCTSTR path ) 
{
    HANDLE			fileHandle;
	BOOL			fSucc;
    WIN32_FIND_DATA	*findData = new WIN32_FIND_DATA;
	if(!findData)
		return;
	
	LPTSTR findpath = new TCHAR[MAX_PATH];
	LPTSTR found = new TCHAR[MAX_PATH];
	lstrcpy(findpath, path);
	lstrcat(findpath, TEXT("\\*.*"));
	fileHandle = FindFirstFile(findpath, findData);
	fSucc = fileHandle != INVALID_HANDLE_VALUE;
	while (fSucc)
	{
		if(findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			lstrcpy(found, path);
			lstrcat(found, TEXT("\\"));
			lstrcat(found, findData->cFileName);
			FindPlaylist(found);
		}
		else
		{
			int len = lstrlen(findData->cFileName);
			if(len > 4)
			{
				LPCTSTR p = findData->cFileName + len - 4;
				if(lstrcmpi(TEXT(".mdl"), p) == 0)
				{
					lstrcpy(found, path);
					lstrcat(found, TEXT("\\"));
					lstrcat(found, findData->cFileName);
					LONG r = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_ADDSTRING, 0, (LPARAM)(g_showPathInPlaylist ? found : findData->cFileName));
					if(r != LB_ERR)
					{
						LPTSTR full = new TCHAR[lstrlen(found) + 1];
						lstrcpy(full, found);
						SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_SETITEMDATA, (WPARAM)r, (LPARAM)full);
					}
				}
			}
		}
		fSucc = FindNextFile(fileHandle, findData);
	}
	if (fileHandle != INVALID_HANDLE_VALUE )
		FindClose(fileHandle);
	delete found;
	delete findpath;
	delete findData;
}

void CDlgPlOpen::FreeListData()
{
	LONG i, n;
	n = SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCOUNT, 0, 0);

	for(i = 0; i < n; ++i)
	{
		LPTSTR p = (LPTSTR)SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETITEMDATA, i, 0);
		delete[] p;
	}
}

CDlgSkin::CDlgSkin(LPCTSTR fn):CDlgBase((LPCTSTR)IDD_SKIN)
{
	lstrcpy(m_file, fn);
}

CDlgSkin::~CDlgSkin()
{

}

LRESULT CDlgSkin::OnCommand(int idCtl, int idNotify)
{
	int i;
	switch(idCtl)
	{
	case IDOK:
		i = SendDlgItemMessage(m_hWnd, IDC_SKINLIST, LB_GETCURSEL, 0, 0);
		if(i != LB_ERR)
		{
			if(SendDlgItemMessage(m_hWnd, IDC_SKINLIST, LB_GETTEXT, i, (LPARAM)m_file) == LB_ERR)
				m_file[0] = 0;
		}
		else
			m_file[0] = 0;
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgSkin::OnInitDialog()
{
	TCHAR path[MAX_PATH], tmp[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE h;
	BOOL findnext;

	CDlgBase::OnInitDialog();
	CUtils::MapToInstalledPath(TEXT(""), path);
	SetDlgItemText(m_hWnd, IDC_PATH, path);
	lstrcpy(tmp, path);
	lstrcat(tmp, TEXT("*.msk"));
	h = FindFirstFile(tmp, &fd);
	findnext = h != INVALID_HANDLE_VALUE;
	while(findnext)
	{
//		lstrcpy(tmp, path);
//		lstrcat(tmp, fd.cFileName);
		SendDlgItemMessage(m_hWnd, IDC_SKINLIST, LB_ADDSTRING, 0, (LPARAM)fd.cFileName);
		findnext = FindNextFile(h, &fd);
	}
	if(h != INVALID_HANDLE_VALUE)
		FindClose(h);
}
