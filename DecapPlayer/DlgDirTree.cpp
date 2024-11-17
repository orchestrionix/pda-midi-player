// DlgPlEdit.cpp: implementation of the CDlgDirTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DlgDirTree.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//!!!!Warning: this function destroy parameter sz0!!!!
BOOL Dir_ExplorePath(HWND hwndTV, LPTSTR sz0)
{
	LPTSTR		sz1,p;
	HTREEITEM	hti,htic;
	TV_ITEM		tvi;
	BOOL		bMatch;

	sz1 = (LPTSTR) LocalAlloc(LMEM_FIXED,sizeof(TCHAR) * MAX_PATH);
	hti = TreeView_GetRoot(hwndTV);
	p = sz0;
	while(*p)
	{
		if(*p == _T('\\')) *p = 0;
		p++;
	}
	p[1] = 0; //For the situation that no "\\" at the end, like "\\Windows"

	p = sz0 + 1;
	while(*p)
	{
		TreeView_Expand(hwndTV, hti, TVE_EXPAND);
		htic = TreeView_GetChild(hwndTV, hti);
		if(htic == NULL)break;
		bMatch = FALSE;
		do
		{
			tvi.mask = TVIF_HANDLE|TVIF_TEXT;
			tvi.pszText = sz1;
			tvi.cchTextMax = MAX_PATH;
			tvi.hItem = htic;
			if(TreeView_GetItem(hwndTV, &tvi) == FALSE)break;
			if(lstrcmpi(p, sz1) == 0)
			{
				bMatch = TRUE;
				break;
			}
			htic = TreeView_GetNextSibling(hwndTV, htic);
		} while(htic);
		if(bMatch)
		{
			hti = htic;
			p += lstrlen(p) + 1;
		}
		else
			break;
	}
//	TreeView_SelectItem(hwndTV, NULL);
	TreeView_SelectItem(hwndTV, hti);
	TreeView_Expand(hwndTV, hti, TVE_EXPAND);
	LocalFree(sz1);
	return *p == 0;
}

BOOL Dir_ExplorePathC(HWND hwndTV, LPCTSTR path)
{
	LPTSTR sz0 = (LPTSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * MAX_PATH);
	lstrcpy(sz0, path);
	BOOL ret = Dir_ExplorePath(hwndTV, sz0);
	LocalFree(sz0);
	return ret;
}

HTREEITEM Dir_AddTreeItem(HWND hwndTV, LPTSTR lpszItem, HTREEITEM htiParent)
{
	TV_ITEM          tvi;
	TV_INSERTSTRUCT  tvins;
	HTREEITEM        hti;

    // Start by initializing the structures
	memset(&tvi, 0, sizeof(TV_ITEM));
	memset(&tvins,0, sizeof(TV_INSERTSTRUCT));
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	tvi.cChildren = 1;

	// Set the text of the item
	tvi.pszText = lpszItem;
	tvi.cchTextMax = lstrlen(lpszItem);

	// Set the parent item based on the specified level
	if (!htiParent)
	{
		tvi.iImage = 2;
		tvi.iSelectedImage = 2;
		tvins.hParent = TVI_ROOT;
	}
	else
	{
		tvi.iImage = 0;
		tvi.iSelectedImage = 1;
		tvins.hParent = htiParent;
	}

	tvins.item = tvi;
	tvins.hInsertAfter = TVI_SORT;

	// Add the item to the tree view control
	hti = (HTREEITEM) SendMessage(hwndTV, TVM_INSERTITEM, 0,
								  (LPARAM)(LPTV_INSERTSTRUCT) &tvins);

    // Return the handle to the item
	return hti;
}

BOOL Dir_BuildDirectory(HWND hwndTV, TV_ITEM tvi, LPTSTR lpszDir)
{
    HTREEITEM hti;
	LPTSTR sz0, sz1;

	// Allocate some memory for the temp strings
	sz0 = (LPTSTR) LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * MAX_PATH);
	sz1 = (LPTSTR) LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * MAX_PATH);
	sz0[0]=sz1[0]=0;
    // Get the text for the first item
    tvi.mask |= TVIF_TEXT;
    tvi.pszText = sz0;
    tvi.cchTextMax =  MAX_PATH;

    if (!TreeView_GetItem(hwndTV, &tvi))
	{
		LocalFree(sz0);
		LocalFree(sz1);
        return FALSE;
	}

    // Create the initial string
    wsprintf(sz1, TEXT("%s"), tvi.pszText);
	lstrcpy(lpszDir, sz1);

	hti = tvi.hItem;

    // Now add the parent directories if any
    while (hti = TreeView_GetParent(hwndTV, hti))
    {
        tvi.mask = TVIF_TEXT;
        tvi.hItem = hti;
        if (!TreeView_GetItem(hwndTV, &tvi))
		{
			LocalFree(sz0);
			LocalFree(sz1);
			return FALSE;
		}

		lstrcpy(sz1, lpszDir);
		if (wcscmp(tvi.pszText,TEXT("\\")) == 0) //we are at the root.
			wsprintf(lpszDir, TEXT("%s%s"), tvi.pszText, sz1);
		else
			wsprintf(lpszDir, TEXT("%s\\%s"), tvi.pszText, sz1);
    }

	if (wcscmp(lpszDir,TEXT("\\"))) //we are not at the root.
		lstrcat(lpszDir, TEXT("\\"));

	// Free the strings now that we're done
	LocalFree(sz0);
	LocalFree(sz1);

    return TRUE;
}

BOOL Dir_GetSelPath(HWND hwndTV, LPTSTR lpszDir)
{
	TV_ITEM tvi;

	tvi.mask = TVIF_HANDLE;
	tvi.hItem = TreeView_GetSelection(hwndTV);
	if(!tvi.hItem)return FALSE;
	return Dir_BuildDirectory(hwndTV, tvi, lpszDir);
}

void CDlgDirTree::GetDirContent(LPTSTR pszDirectory, HTREEITEM htiParent)
{
    WIN32_FIND_DATA	findData;
    HANDLE			fileHandle;
    BOOL			bHasSubDir = FALSE;
	TCHAR			fs[MAX_PATH];
	BOOL			fSucc;

	if(!htiParent || !pszDirectory)
		return;
	lstrcpy(fs, pszDirectory);
	lstrcat(fs, TEXT("*.*"));
	// Get the first file in the directory
	fileHandle = FindFirstFile(fs, &findData);
	fSucc = fileHandle != INVALID_HANDLE_VALUE;
	while (fSucc)
	{
		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			bHasSubDir = TRUE;
			if (!Dir_AddTreeItem(m_hTree, findData.cFileName, htiParent))
				break;
		}
		fSucc = FindNextFile(fileHandle, &findData);
	}
	if (fileHandle != INVALID_HANDLE_VALUE )
		FindClose(fileHandle);
	if(!bHasSubDir)
	{
		TV_ITEM         tvi;

		tvi.hItem = htiParent;
		tvi.mask = TVIF_CHILDREN;
		tvi.cChildren = 0;
		TreeView_SetItem(m_hTree, &tvi);
	}
}

CDlgDirTree::CDlgDirTree(LPCTSTR path):CDlgBase((LPCTSTR)IDD_DIR_TREE, (LPCTSTR)IDD_DIR_TREE_LS)
{
	lstrcpy(m_path, path);
}

CDlgDirTree::~CDlgDirTree()
{

}

LRESULT CDlgDirTree::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		Dir_GetSelPath(m_hTree, m_path);
		EndDialog(m_hWnd, idCtl);
		return TRUE;

	case IDCANCEL:
		m_path[0] = 0;
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgDirTree::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_hTree = GetDlgItem(m_hWnd, IDC_TREE);
	Dir_AddTreeItem(m_hTree, TEXT("\\"), NULL);
	Dir_ExplorePathC(m_hTree, m_path);
}

BOOL CDlgDirTree::OnNotify(int idCtl, LPNMHDR pHdr)
{
	switch(idCtl)
	{
	case IDC_TREE:
		switch(pHdr->code)
		{
		case TVN_ITEMEXPANDING:
			{
				LPNM_TREEVIEW pnmtv = (LPNM_TREEVIEW) pHdr;
				if(pnmtv->action == TVE_EXPAND && !(pnmtv->itemNew.state & TVIS_EXPANDEDONCE))
				{
					TCHAR szDir[MAX_PATH];
					Dir_BuildDirectory(pnmtv->hdr.hwndFrom, pnmtv->itemNew, szDir);
					GetDirContent(szDir, (HTREEITEM)pnmtv->itemNew.hItem);
				}
			}
			break;
		case TVN_SELCHANGED:
			break;
		}
	}
	return CDlgBase::OnNotify(idCtl, pHdr);
}
