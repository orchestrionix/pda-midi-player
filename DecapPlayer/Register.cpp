#include "stdafx.h"
#include "resource.h"
#include "RegisterCore.h"
#include "DlgBase.h"
#include "register.h"
#include "registercalckey.h"

static DWORD gs_serial;
static bool gs_indlg = false;

LRESULT CALLBACK RegistrationProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SHInputDialog(hDlg,message,wParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SHINITDLGINFO shidi;
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hDlg;
		SHInitDialog(&shidi);
		{
			LPTSTR sz0 = (LPTSTR) LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * 80);
			int n = 15 - Regist_GetDaysUsed();

			if(n < 0) n = 0;
			wsprintf(sz0, TEXT("You have %d day(s) left for evaluation."), n);
			SetDlgItemText(hDlg, IDC_TRIAL_LEFT, sz0);
			LocalFree(sz0);
		}
		SetDlgItemInt(hDlg, IDC_USERNAME, gs_serial, FALSE);
		return TRUE;
		
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
//			if(!Regist_IsRegistered())
			{
				DWORD code1, code2;
				BOOL b;

				code1 = GetDlgItemInt(hDlg,IDC_EDIT1,&b,FALSE);
				if(b)
					code2 = GetDlgItemInt(hDlg,IDC_EDIT2,&b,FALSE);
				if(b)
				{
					if(Regist_CheckPWD(code1, code2, gs_serial))
					{
						MessageBox(hDlg,TEXT("Thank you for your registration!"), TEXT("Registration"), MB_OK|MB_ICONINFORMATION);
						Regist_SetRegCode(code1, code2);
						EndDialog(hDlg, LOWORD(wParam));
						return TRUE;
					}
				}
				MessageBox(hDlg,TEXT("Incorrect user information or registration code!\nPlease conform and re-enter."), NULL, MB_OK|MB_ICONSTOP);
				return TRUE;
			}
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		case IDC_REMIND:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			PostQuitMessage(0);
			return TRUE;
		}
		break;
	}
    return FALSE;
}

BOOL RegisterDlg(HWND hWnd)
{
	if(gs_indlg)
		return true;
	if(Regist_GetUserSerial(&gs_serial))
	{
		gs_indlg = true;
		int ret = DialogBox(GetModuleHandle(NULL), (LPCTSTR)(IsLandScapeMode() ? IDD_REGISTER_LS : IDD_REGISTER),
			hWnd, (DLGPROC)RegistrationProc) != -1;
		gs_indlg = false;
		return ret;
	}
	MessageBox(hWnd, TEXT("Please fill the \"Name\" of \"Owner Information\" in \"Settings\" before registration!"),
		TEXT("Error"), MB_ICONSTOP | MB_OK);
	return FALSE;
}

BOOL TestAndPromptRegister(HWND hWnd)
{
	if(Regist_IsRegistered())
		return TRUE;

	RegisterDlg(hWnd);
	if(!Regist_IsRegistered())
	{
		if(Regist_IsTrialExpired(15))
		{
			MessageBox(hWnd, TEXT("This program is expired!\nPlease register at http://www.intelliart.com to continue using this program!"), TEXT("IntelliArt Midi Player"), MB_OK | MB_ICONSTOP);
			PostQuitMessage(0);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL TestAndPromptRegister2(HWND hWnd)
{
	if(Regist_IsRegistered())
		return TRUE;

	if(Regist_IsTrialExpired(15))
	{
		RegisterDlg(hWnd);
		if(!Regist_IsRegistered())
		{
			MessageBox(hWnd, TEXT("This program is expired!\nPlease register at http://www.intelliart.com to continue using this program!"), TEXT("IntelliArt Midi Player"), MB_OK | MB_ICONSTOP);
			PostQuitMessage(0);
			return FALSE;
		}
	}
	return TRUE;
}
