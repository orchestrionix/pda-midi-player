// DlgPlEdit.h: interface for the CDlgDirTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__DIRTREE_H__)
#define __DIRTREE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DlgBase.h"

class CDlgDirTree : public CDlgBase  
{
public:
	void GetDirContent(LPTSTR pszDirectory, HTREEITEM htiParent);
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgDirTree(LPCTSTR path);
	virtual ~CDlgDirTree();

	TCHAR m_path[MAX_PATH];
	HWND m_hTree;
protected:
	BOOL OnNotify(int idCtl, LPNMHDR pHdr);
};
#endif // !defined(__DIRTREE_H__)
