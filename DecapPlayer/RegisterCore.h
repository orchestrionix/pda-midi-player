#ifndef __REGISTERCORE_H__
#define __REGISTERCORE_H__

#define MAX_REGIST_STR 60

enum
{
	FEATURE_RS232 = 0x01,
	FEATURE_SOFTSYNTH = 0x02,
	FEATURE_MP3 = 0x04
};

BOOL Regist_GetUserName(LPTSTR pszUserName,DWORD cSize);
BOOL Regist_GetRegCode(LPCTSTR pValueName, DWORD *pdwPWD);
BOOL Regist_SetRegCode(DWORD dwCode0, DWORD dwCode1);
DWORD Regist_GetUserId();
DWORD Regist_GetFeatureCode();
BOOL Regist_GetUserSerial(DWORD *pdwSerial);

//DWORD Regist_GeneratePWD(DWORD dwCode0);
//BOOL Regist_CheckPWD(DWORD dwCode0, DWORD dwCode1);
BOOL Regist_IsRegistered();
DWORD Regist_GeneratePWD(LPCTSTR pszUserID);

BOOL Regist_IsTrialExpired(int day);
int Regist_GetDaysUsed();

#endif