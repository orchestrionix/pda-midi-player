#include <windows.h>
#include "RegisterCore.h"
#include "registercalckey.h"

static BOOL g_bRegistered=FALSE;

#define ENCRYPT_MAGIC_NUMBER 1139

#define REGKEY_REGIST	TEXT("Software\\DECAP\\MidiPlayer")
#define REGVAL_USER		TEXT("UserName")
#define REGVAL_REGCODE0	TEXT("RegCode0")
#define REGVAL_REGCODE1	TEXT("RegCode1")
#define REGIST_TIMEKEY	TEXT("SYSTEM\\CurrentControlSet\\Control")
#define REGIST_TIMEVAL	TEXT("Setting104")

static struct REGINFO{
	__int64 InstallTime, LastUseTime;
}g_stRegInfo;

static DWORD g_userId = 0xff;

typedef HRESULT (APIENTRY *TGetDeviceUniqueID)(
  LPBYTE pbApplicationData,
  DWORD cbApplictionData,
  DWORD dwDeviceIDVersion,
  LPBYTE pbDeviceIDOutput,
  DWORD* pcbDeviceIDOutput
);

HKEY Regist_OpenRegKey()
{
	HKEY hkey;
	DWORD d;

	if(RegCreateKeyEx(HKEY_CURRENT_USER, REGKEY_REGIST, 0, NULL, 0, NULL, NULL, &hkey, &d)
		==ERROR_SUCCESS)
		return hkey;
	return NULL;
}

BOOL Regist_GetUserName(LPTSTR pszUserName,DWORD cSize)
{
	HKEY hkey;
	if(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("ControlPanel\\Owner"), 0, 0, &hkey) != ERROR_SUCCESS)
		return FALSE;

	TCHAR s[0x280];
	DWORD dws = sizeof(s);

	s[0] = 0;
	if(RegQueryValueEx(hkey, TEXT("Owner"), NULL, NULL, (BYTE*)s, &dws) == ERROR_SUCCESS)
	{
		int len = lstrlen(s);
		if(len > 0 && len < cSize)
		{
			lstrcpy(pszUserName, s);
			RegCloseKey(hkey);
			return TRUE;
		}
	}
	RegCloseKey(hkey);
	return FALSE;
}

#define APPLICATION_DATA            "DECAPMIDIPLAYER"
#define APPLICATION_DATA_LENGTH     15

TCHAR dbgstr[1000];

BOOL Regist_GetUserSerial(DWORD *pdwSerial)
{
	TCHAR s[64];
	TGetDeviceUniqueID pGetDeviceUniqueID = NULL;

	HMODULE hModule = LoadLibrary(TEXT("coredll.dll"));
	if(hModule)
	{
		pGetDeviceUniqueID = (TGetDeviceUniqueID)GetProcAddress(hModule, TEXT("GetDeviceUniqueID"));
		if(pGetDeviceUniqueID)
		{
			BYTE devid[64];
			DWORD cbDeviceId = sizeof(devid);
			if(pGetDeviceUniqueID(reinterpret_cast<PBYTE>(APPLICATION_DATA),
				APPLICATION_DATA_LENGTH,
				1,
				devid,
				&cbDeviceId) == S_OK)
			{
//				wsprintf(s, TEXT("%d, %d, %d, %d"), cbDeviceId, devid[0], devid[1], devid[2]);
//				MessageBox(NULL, s, TEXT("AAB"), MB_OK);
				for(DWORD i = 0; i < cbDeviceId; ++i)
				{
					s[i] = devid[i];
				}
				s[cbDeviceId] = 0;
				lstrcat(dbgstr, TEXT("uid 1:"));
				lstrcat(dbgstr, s);
				lstrcat(dbgstr, TEXT(", "));
			}
			else {
				lstrcat(dbgstr, TEXT("uid 0, "));
				pGetDeviceUniqueID = NULL;
			}
		}
		FreeLibrary(hModule);
	}

	if(!pGetDeviceUniqueID)
	{
		if(!Regist_GetUserName(s, 64))
			return FALSE;
		CharUpper(s);
	}

	lstrcat(dbgstr, TEXT("s:"));
	lstrcat(dbgstr, s);
	lstrcat(dbgstr, TEXT(", "));

	DWORD d = 0x756951;
	for(int i = 0; s[i]; i++)
	{
		if(s[i] != TEXT(' '))
		{
			d = (d >> 1) * s[i] + d % 0x696557 ^ d / 0x6e694d;
		}
	}
	*pdwSerial = d;
	return TRUE;
}

BOOL Regist_GetRegCode(LPCTSTR pValueName, DWORD *pdwPWD)
{
	HKEY hkey = Regist_OpenRegKey();
	DWORD dws = sizeof(DWORD);
	LONG ret;

	if(!hkey)return FALSE;
	ret = RegQueryValueEx(hkey, pValueName, NULL, NULL, (BYTE*)pdwPWD, &dws);
	RegCloseKey(hkey);
	return ret == ERROR_SUCCESS;
}

BOOL Regist_SetRegCode(DWORD dwCode0, DWORD dwCode1)
{
	HKEY hkey=Regist_OpenRegKey();
	LONG ret0, ret1;

	if(!hkey)return FALSE;

	ret0 = RegSetValueEx(hkey, REGVAL_REGCODE0, 0, REG_DWORD, (BYTE*)&dwCode0, sizeof(DWORD));
	ret1 = RegSetValueEx(hkey, REGVAL_REGCODE1, 0, REG_DWORD, (BYTE*)&dwCode1, sizeof(DWORD));
	RegCloseKey(hkey);
	return ret0 == ERROR_SUCCESS && ret1 == ERROR_SUCCESS;
}

DWORD Regist_GetUserId()
{
	return g_userId >> 8;
}

DWORD Regist_GetFeatureCode()
{
	return g_userId & 0xff;
}

/*
DWORD Regist_GeneratePWD(DWORD dwCode0)
{
	DWORD d;

	d=0x52415453 ^ dwCode0;
	d=(d*ENCRYPT_MAGIC_NUMBER) ^ 0x4d5751;
	d=((d << (d & 0xf)) + 211) * 217 + 0x6d7771;
	return d&0x4ffffff;
}

BOOL Regist_CheckPWD(DWORD dwCode0, DWORD dwCode1)
{
	//make dwCode1 error value if cheat(not generated by author)
//	if(dwCode0 < 16850000 || dwCode0 > 16870000)
//		dwCode1++;
	g_bRegistered = Regist_GeneratePWD(dwCode0) == dwCode1;
	return g_bRegistered;
}
*/

BOOL Regist_IsRegistered()
{
#ifdef _WIN32_WCE_EMULATION
	return true;
#else
	DWORD d0, d1, serial;

	if(g_bRegistered)return TRUE;

	dbgstr[0] = 0;
	if(Regist_GetRegCode(REGVAL_REGCODE0, &d0))
	{
		lstrcat(dbgstr, TEXT("c0 1, "));
		if(Regist_GetRegCode(REGVAL_REGCODE1, &d1))
		{
			lstrcat(dbgstr, TEXT("c1 1, "));
			if(Regist_GetUserSerial(&serial))
			{	
				lstrcat(dbgstr, TEXT("gs 1, "));
				TCHAR dbgs[200];
				wsprintf(dbgs, TEXT("all:%08x %08x %08x, "), d0, d1, serial);
				lstrcat(dbgstr, dbgs);
				if(Regist_CheckPWD(d0, d1, serial))
				{
					g_userId = d0;
					lstrcat(dbgstr, TEXT("cp 1, "));
//					MessageBox(NULL, dbgstr, TEXT("Debug"), MB_OK);
					return TRUE;
				}
				else {
					lstrcat(dbgstr, TEXT("cp 0, "));
				}
			}
			else {
				lstrcat(dbgstr, TEXT("gs 0, "));
			}
		}
		else {
			lstrcat(dbgstr, TEXT("c1 0, "));
		}
	}
	else {
		lstrcat(dbgstr, TEXT("c0 0, "));
	}
//	MessageBox(NULL, dbgstr, TEXT("Debug"), MB_OK);
	return FALSE;
#endif
}

/*******************************************************************************/

__int64 Regist_GetStartTime()
{
	HKEY hkey;
	DWORD dwRead;
	SYSTEMTIME syst;

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGIST_TIMEKEY, 0, NULL, 0, 0, NULL, &hkey, &dwRead) == ERROR_SUCCESS)
	{
		dwRead = sizeof(g_stRegInfo);
		if(RegQueryValueEx(hkey, REGIST_TIMEVAL, NULL, NULL, (LPBYTE)&g_stRegInfo, &dwRead) != ERROR_SUCCESS)
		{
			GetSystemTime(&syst);
			SystemTimeToFileTime(&syst, (FILETIME*)&g_stRegInfo.InstallTime);
			g_stRegInfo.LastUseTime = g_stRegInfo.InstallTime;
			RegSetValueEx(hkey, REGIST_TIMEVAL, 0, REG_BINARY, (BYTE*)&g_stRegInfo, sizeof(g_stRegInfo));
		}
		RegCloseKey(hkey);
		return g_stRegInfo.InstallTime;
	}
	else
		return 0;
}

static void Regist_GetRegInfo()
{
	HKEY hkey;
	DWORD dwRead;
	SYSTEMTIME syst;
	__int64 cur;

	GetSystemTime(&syst);
	SystemTimeToFileTime(&syst, (FILETIME*)&cur);
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGIST_TIMEKEY, 0, NULL, 0, 0, NULL, &hkey, &dwRead) == ERROR_SUCCESS)
	{
		dwRead = sizeof(g_stRegInfo);
		if(RegQueryValueEx(hkey, REGIST_TIMEVAL, NULL, NULL, (LPBYTE)&g_stRegInfo, &dwRead) != ERROR_SUCCESS)
		{
			g_stRegInfo.LastUseTime = g_stRegInfo.InstallTime = cur;
		}
		else
		{
			if(g_stRegInfo.LastUseTime < cur)
				g_stRegInfo.LastUseTime = cur;
		}
		RegSetValueEx(hkey, REGIST_TIMEVAL, 0, REG_BINARY, (BYTE*)&g_stRegInfo, sizeof(g_stRegInfo));
		RegCloseKey(hkey);
	}
	else
	{
		g_stRegInfo.InstallTime = 0;
		g_stRegInfo.LastUseTime = cur;
	}
}

BOOL Regist_IsTrialExpired(int day)
{
#ifdef _WIN32_WCE_EMULATION
	return false;
#else
	__int64 end;

	Regist_GetRegInfo();
	end = g_stRegInfo.InstallTime + ((__int64)day) * 24 * 3600 * 1000 * 1000 * 10;

	return g_stRegInfo.LastUseTime > end;
	
#endif
}

int Regist_GetDaysUsed()
{
	__int64 ft;

	Regist_GetRegInfo();
	ft = g_stRegInfo.LastUseTime - g_stRegInfo.InstallTime;
	return (int)(ft / ((__int64)24 * 3600 * 1000 * 1000 * 10));
}
