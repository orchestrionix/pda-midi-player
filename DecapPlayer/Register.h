#ifndef __REGISTER_H__
#define __REGISTER_H__

BOOL RegisterDlg(HWND hWnd);
BOOL TestAndPromptRegister(HWND hWnd);
BOOL TestAndPromptRegister2(HWND hWnd);
LRESULT CALLBACK RegistrationProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif