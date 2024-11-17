// Utils.cpp: implementation of the CUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TCHAR CUtils::m_exepath[MAX_PATH];

CUtils::CUtils()
{

}

CUtils::~CUtils()
{

}

LPTSTR CUtils::MapToInstalledPath(LPCTSTR path, LPTSTR buff)
{
	if(!m_exepath[0])
	{
		LPTSTR p;
		p = m_exepath + GetModuleFileName(NULL, m_exepath, MAX_PATH);
		while(p > m_exepath && *p != TEXT('\\'))
			p--;
		p[1] = 0;
	}
	lstrcpy(buff, m_exepath);
	lstrcat(buff, path);
	return buff;
}
