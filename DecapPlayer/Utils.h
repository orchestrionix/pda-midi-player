// Utils.h: interface for the CUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILS_H__DA36E392_7F24_4817_9B5F_C5619345FA72__INCLUDED_)
#define AFX_UTILS_H__DA36E392_7F24_4817_9B5F_C5619345FA72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUtils  
{
public:
	static LPTSTR MapToInstalledPath(LPCTSTR path, LPTSTR buff);
	CUtils();
	virtual ~CUtils();

	static TCHAR m_exepath[MAX_PATH];
};

#endif // !defined(AFX_UTILS_H__DA36E392_7F24_4817_9B5F_C5619345FA72__INCLUDED_)
