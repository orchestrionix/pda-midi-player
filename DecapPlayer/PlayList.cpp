// PlayList.cpp: implementation of the CPlayList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PlayList.h"
#include "player.h"
#include "dlgdirtree.h"
#include "DlgPlEdit.h"
#include "skin.h"
#include "win32\PsFileReader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayList g_playlist;

CPlayList::CPlayList()
{
	memset(m_list, 0, sizeof(m_list));
	m_nlist = 0;
	m_listfn[0] = 0;
	SetShuffle(false);
}

CPlayList::~CPlayList()
{
	RemoveAll();
}

void CPlayList::RemoveAll()
{
	int i;
	for(i = 0; i < MAX_LIST; i++)
	{
		if(m_list[i])
		{
			delete[] m_list[i];
			m_list[i] = NULL;
		}
	}
	m_nlist = 0;
}

bool CPlayList::ShowDlgAdd(HWND hwnd)
{
	TCHAR path[MAX_PATH];

	SHGetSpecialFolderPath(hwnd, path, CSIDL_PERSONAL, false);
	CDlgDirTree dlg(path);
	if(dlg.ShowModal(hwnd) == IDCANCEL)
		return true;
	wsprintf(path, TEXT("%s*.*"), dlg.m_path);

    WIN32_FIND_DATA	findData;
    HANDLE			fileHandle;
	BOOL			fSucc;
	int				nAdd = 0;

	fileHandle = FindFirstFile(path, &findData);
	fSucc = fileHandle != INVALID_HANDLE_VALUE;
	while (fSucc)
	{
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			int len = lstrlen(findData.cFileName);
			if(len > 4)
			{
				LPCTSTR p = findData.cFileName + len - 4;
				if(lstrcmpi(TEXT(".mid"), p) == 0 || lstrcmpi(TEXT(".rmi"), p) == 0 || lstrcmpi(TEXT(".dmi"), p) == 0)
				{
					TCHAR file[MAX_PATH];
					wsprintf(file, TEXT("%s%s"), dlg.m_path, findData.cFileName);
					Add(file);
					nAdd++;
				}
				else if(lstrcmpi(TEXT(".mp3"), p) == 0)
				{
					TCHAR file[MAX_PATH];
					wsprintf(file, TEXT("%s%s"), dlg.m_path, findData.cFileName);
					int suffix = lstrlen(file) - 3;
					bool isMidiExist = false;
					CPsFileReader r;
					lstrcpy(file + suffix, TEXT("mid"));
					if(r.Open(file, CPsFileReader::F_READ))
					{
						isMidiExist = true;
						r.Close();
					}
					if(!isMidiExist)
					{
						lstrcpy(file + suffix, TEXT("rmi"));
						if(r.Open(file, CPsFileReader::F_READ))
						{
							isMidiExist = true;
							r.Close();
						}
					}
					if(!isMidiExist)
					{
						lstrcpy(file + suffix, TEXT("dmi"));
						if(r.Open(file, CPsFileReader::F_READ))
						{
							isMidiExist = true;
							r.Close();
						}
					}
					if(!isMidiExist)
					{
						wsprintf(file, TEXT("%s%s"), dlg.m_path, findData.cFileName);
						Add(file);
						nAdd++;
					}
				}
			}
		}
		fSucc = FindNextFile(fileHandle, &findData);
	}
	if (fileHandle != INVALID_HANDLE_VALUE )
		FindClose(fileHandle);
	if(nAdd == 0)
	{
		MessageBox(hwnd, TEXT("Can't find any MIDI/MP3 files(*.mid;*.rmi;*.dmi;*.mp3) in this folder!"), TEXT("Add MIDI files"), MB_OK | MB_ICONEXCLAMATION);
	}
	return true;
}

void CPlayList::Delete(int id)
{
	int i;

	ASSERT(id < GetItemCount());
	delete[] m_list[id];
	for(i = id; i < m_nlist - 1; i++)
	{
		m_list[i] = m_list[i + 1];
	}
	m_list[i] = NULL;
	m_nlist--;
	SetShuffle(m_shuffled);
}

void CPlayList::Swap(int id1, int id2)
{
	LPTSTR p;

	p = m_list[id1];
	m_list[id1] = m_list[id2];
	m_list[id2] = p;
}

void CPlayList::Insert(int id, LPTSTR fn)
{
	int i;
	ASSERT(id >= 0 && id <= GetItemCount());

	for(i = GetItemCount(); i > id; i--)
		m_list[i] = m_list[i - 1];
	m_list[id] = fn;
	SetShuffle(m_shuffled);
}

void CPlayList::NewList()
{
	RemoveAll();
	m_listfn[0] = 0;
}

bool CPlayList::Save(LPCTSTR fn)
{
	LPTSTR pD = new TCHAR[(MAX_PATH + 1) * m_nlist + 3];
	LPTSTR p = pD;
	int i;

	*p++ = 0x4c50;
	*p++ = 1;

	*p++ = m_nlist;

	for(i = 0; i < m_nlist; i++)
	{
		int len = lstrlen(m_list[i]);
		*p++ = len;
		lstrcpy(p, m_list[i]);
		p += len;
	}

	HANDLE handle = CreateFile(fn, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if(handle == INVALID_HANDLE_VALUE)
	{
		delete p;
		return false;
	}
	
	DWORD w = 0;
	WriteFile(handle, pD, (p - pD) * sizeof(TCHAR), &w, NULL);
	CloseHandle(handle);
	delete pD;
	return true;
}

bool CPlayList::SaveAs(HWND hWnd)
{
	TCHAR path[MAX_PATH];
	OPENFILENAME ofn;

	lstrcpy(path, m_listfn);
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = TEXT("Playlist (.mdl)\0*.mdl\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = TEXT("Save Playlist As");
	ofn.Flags = 0;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if(GetSaveFileName(&ofn))
	{
		return Save(path);
	}
	return true;
}

bool CPlayList::Save(HWND hWnd)
{
	if(m_listfn[0])
		return Save(m_listfn);
	else
		return SaveAs(hWnd);
}

int __cdecl compare( const void *arg1, const void *arg2 )
{
   /* Compare all of both strings: */
   return lstrcmp(*(LPCTSTR*)arg1, *(LPCTSTR*)arg2);
}

bool CPlayList::Open(HWND hwnd, int id)
{
	if(id >= 128)
		return false;

	TCHAR path[MAX_PATH];

	SHGetSpecialFolderPath(hwnd, path, CSIDL_PERSONAL, false);

	TCHAR findpath[MAX_PATH];
	lstrcpy(findpath, path);
	lstrcat(findpath, TEXT("\\*.*"));

	LPTSTR fn[128];

    WIN32_FIND_DATA	findData;
    HANDLE			fileHandle;
	BOOL			fSucc;
	int				nAdd = 0;

	fileHandle = FindFirstFile(findpath, &findData);
	fSucc = fileHandle != INVALID_HANDLE_VALUE;
	while (fSucc)
	{
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			int len = lstrlen(findData.cFileName);
			if(len > 4)
			{
				LPCTSTR p = findData.cFileName + len - 4;
				if(lstrcmpi(TEXT(".mdl"), p) == 0)
				{
					TCHAR file[MAX_PATH];
					wsprintf(file, TEXT("%s\\%s"), path, findData.cFileName);
					fn[nAdd] = new TCHAR[lstrlen(file) + 1];
					lstrcpy(fn[nAdd], file);
					nAdd++;
					if(nAdd >= 128)
						break;
				}
			}
		}
		fSucc = FindNextFile(fileHandle, &findData);
	}
	if (fileHandle != INVALID_HANDLE_VALUE )
		FindClose(fileHandle);
	if(id >= nAdd)
	{
		while(nAdd > 0)
		{
			--nAdd;
			delete[] fn[nAdd];
		}
		return false;
	}

	qsort(fn, (size_t)nAdd, sizeof(LPTSTR), compare);
	lstrcpy(path, fn[id]);
	while(nAdd > 0)
	{
		--nAdd;
		delete[] fn[nAdd];
	}
	return Open(path);
}

bool CPlayList::Open(LPCTSTR fn)
{
	CPsFileReader r;
	int nTry = 10;
	//if file is on CF card, it may wait some moment to open it when power on
	while(nTry > 0 && !r.Open(fn, CPsFileReader::F_READ))
	{
		--nTry;
		Sleep(300);
	}
	if(nTry <= 0)
		return false;

	if(r.ReadShort() != 0x4c50 || r.ReadShort() != 1)
	{
		r.Close();
		return false;
	}
	RemoveAll();

	int n = r.ReadShort();

	while(n > 0)
	{
		int len = r.ReadShort();
		m_list[m_nlist] = new TCHAR[len + 1];
		r.ReadBlock(m_list[m_nlist], len * sizeof(TCHAR));
		m_list[m_nlist][len] = 0;
		++m_nlist;
		--n;
	}

	r.Close();

	lstrcpy(m_listfn, fn);
	SetShuffle(m_shuffled);

	g_skin.PlaylistReset();

	if(m_nlist > 0 && g_player.GetStatus() == CPlayer::STATUS_READY)
	{
		g_player.Open(0);
	}
	return true;
}

bool CPlayList::Open(HWND hWnd)
{
/*
	TCHAR path[MAX_PATH];
	OPENFILENAME ofn;

	lstrcpy(path, m_listfn);
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = TEXT("Playlist (.mdl)\0*.mdl\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = TEXT("Open Playlist");
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if(GetOpenFileName(&ofn))
	{
		return Open(path);
	}
	return true;
*/
	CDlgPlOpen *dlg = new CDlgPlOpen;
	if(!dlg)
		return true;
	dlg->ShowModal(hWnd);
	LPTSTR fn = dlg->GetSelected();
	if(fn[0])
		return Open(fn);
	else
		return true;
}

void CPlayList::SetShuffle(bool shuffled)
{
	int i, j, n;

	m_shuffled = shuffled;
	for(i = 0; i < m_nlist; i++)
	{
		m_order[i] = i;
	}
	if(shuffled && m_nlist > 1)
	{
		for(i = 0; i < m_nlist; i++)
		{
			n = rand() % m_nlist;
			j = m_order[i];
			m_order[i] = m_order[n];
			m_order[n] = j;
		}
	}
}

LPCTSTR CPlayList::GetListFile()
{
	return m_listfn;
}

void CPlayList::Add(LPCTSTR fn)
{
	if(m_nlist >= MAX_LIST)return;
	m_list[m_nlist] = new TCHAR[lstrlen(fn) + 1];
	lstrcpy(m_list[m_nlist], fn);
	m_nlist++;
	SetShuffle(m_shuffled);
	if(m_nlist == 1 && g_player.GetStatus() == CPlayer::STATUS_READY)
	{
		g_player.Open(0);
	}
}

LPCTSTR CPlayList::GetDisplayName(int id)
{
	LPCTSTR fn = GetShuffledItemName(id);
	LPCTSTR p = fn + lstrlen(fn) - 1;
	while(p >= fn && *p != TEXT('\\')) p--;
	p++;
	return p;
}

void CPlayList::Sort(bool isAscend)
{
	int i, j;

	for(i = 0; i < m_nlist - 1; i++)
	{
		for(j = m_nlist - 2; j >= i; j--)
		{
			int c = lstrcmp(m_list[j], m_list[j + 1]);
			if((c > 0) && isAscend || (c < 0) && !isAscend)
			{
				LPTSTR p = m_list[j];
				m_list[j] = m_list[j + 1];
				m_list[j + 1] = p;
			}
		}
	}
}