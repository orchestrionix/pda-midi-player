     // ModPlayer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MidiPlayer.h"
#include "skin.h"
#include "utils.h"
#include "playlist.h"
#include "player.h"
#include "dlgpledit.h"
#include "registercore.h"
#include "register.h"
#include "common\PsObject.h"
#include "registercalckey.h"
#include "dlgchannel.h"
#include "DlgOption.h"
#include "DlgDecapSetting.h"
#include "psportrs232.h"
#include "win32/PsWinDriver.h"
#include <Notify.h>

#define MAX_LOADSTRING 100
//#define COUNT_TICK

// Global Variables:
HINSTANCE			hInst;					// The current instance
HWND				hwndMain, hwndCB;					// The command bar handle

static SHACTIVATEINFO s_sai;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About			(HWND, UINT, WPARAM, LPARAM);
HWND				CreateRpCommandBar(HWND);

TCHAR g_cmdFile[MAX_PATH];
int g_regtick = 10 * 60 * 2;

int g_screenWidth, g_screenHeight;
bool g_showPathInPlaylist;
TCHAR g_currentSkin[MAX_PATH];

void Decrypt(char* pStart, char* pEnd)
{
	DWORD a;
	VirtualProtect(pStart, pEnd - pStart, PAGE_EXECUTE_READWRITE, &a);
	while(pStart < pEnd)
		*pStart++ ^= 0x5f;
}

static DWORD gs_ActivePid, g_SysPid;
static BOOL CALLBACK StopWndProc(HWND hwnd, LPARAM lParam )
{
	TCHAR		s[128];
	DWORD		pid;

	if(GetParent(hwnd)==NULL&&IsWindowVisible(hwnd))
	{
		GetWindowThreadProcessId(hwnd, &pid);
		if(pid != g_SysPid && pid != GetCurrentProcessId())
		{
			if(!(lParam && pid == gs_ActivePid))
			{
				if(GetWindowText(hwnd, s, 128))
				{
					SetForegroundWindow(hwnd);
					SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
			}
		}
	}
	return TRUE;
}

void CloseOtherApps(HWND hwnd)
{
	GetWindowThreadProcessId(hwnd, &gs_ActivePid);
	HWND gs_hwndSysTB = FindWindow(TEXT("HHTaskBar"), NULL);
	if(gs_hwndSysTB)
		GetWindowThreadProcessId(gs_hwndSysTB,&g_SysPid);
	else
		g_SysPid = 0;
	EnumWindows(StopWndProc, 1);
}

void SetAutoStart(bool isEnabled)
{
	TCHAR	szExeFile[MAX_PATH];			// The title bar text
	DWORD	val;
	GetModuleFileName(NULL, szExeFile, MAX_PATH);

	//remove previous autostart
	CeRunAppAtEvent(szExeFile, NOTIFICATION_EVENT_NONE);
	CeRunAppAtEvent(szExeFile, NOTIFICATION_EVENT_NONE);
	val = 0;
	if(isEnabled)
	{
		CeRunAppAtEvent(szExeFile, NOTIFICATION_EVENT_RS232_DETECTED);
		CeRunAppAtEvent(szExeFile, NOTIFICATION_EVENT_WAKEUP);
		val = 1;
	}
	HKEY hkey;
	DWORD s;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, PLAYER_REG_KEY, 0, NULL, 0, NULL, NULL, &hkey, &s) == ERROR_SUCCESS)
	{
		RegSetValueEx(hkey, TEXT("AutoStart"), 0, REG_BINARY, (LPBYTE)&val, sizeof(val));
		RegCloseKey(hkey);
	}
}

TCHAR g_lastPlaylist[MAX_PATH];

bool LoadConfig()
{
	HKEY hkey;
	DWORD val, s;

	g_player.SetVolume(40 * 0xffff / 100);//default volume
	if(RegOpenKeyEx(HKEY_CURRENT_USER, PLAYER_REG_KEY, 0, 0, &hkey) == ERROR_SUCCESS)
	{
		int oport[CPlayer::NUM_OUTPUT_PORT];
		s = sizeof(oport);
		if(RegQueryValueEx(hkey, TEXT("OPortScheme"), NULL, NULL, (LPBYTE)oport, &s) == ERROR_SUCCESS)
		{
			g_player.SetOutputPortsScheme(oport, CPlayer::NUM_OUTPUT_PORT);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("MixVol"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetMixVolume((F16)val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("SongDelay"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetSongDelay(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("Mp3Vol"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetMp3Volume((F16)val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("SysVol"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetVolume(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("SSVolThreshold"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetSoftSynthVolThreshold(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("BassCutoff"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetBassCutoff(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("BassVol"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetBassVolume((F16)val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("COMDelay"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetCOMDelay(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("COMOutId"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.GetRS232()->SetOutPortId(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("COMInId"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.GetRS232()->SetInPortId(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("COMBaudRate"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.GetRS232()->SetBaudRate(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("Shuffle"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetShuffle(val != 0);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("Repeat"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetRepeat(val != 0);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("Single"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetSingle(val != 0);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("AutoAdvance"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetAutoAdvance(val != 0);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("ReverseStereo"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetReverse(val != 0);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("Surround"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetSurround(val != 0);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("Reverb"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetReverb(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("PanSep"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetPanSep(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("TimeMode"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetTimeMode(val);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("CloseOtherApps"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetCloseAppOnActive(val != 0);
		}
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("RespondIR"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetEnableDecapCommand(val != 0);
		}
		s = sizeof(val);
		val = 40;
		if(RegQueryValueEx(hkey, TEXT("MaxPoly"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_player.SetMaxPoly(val);
		}
		short delayTable[16];
		g_player.GetChannelDelayTable(delayTable);
		s = sizeof(delayTable);
		if(RegQueryValueEx(hkey, TEXT("ChnDelays"), NULL, NULL, (LPBYTE)delayTable, &s) == ERROR_SUCCESS)
		{
			g_player.SetChannelDelayTable(delayTable);
		}

		char volCompression[16];
		s = sizeof(volCompression);
		if(RegQueryValueEx(hkey, TEXT("SoftSynthVolCompression"), NULL, NULL, (LPBYTE)volCompression, &s) == ERROR_SUCCESS)
		{
			memcpy(g_player.GetChannelCompressionTable(0), volCompression, sizeof(volCompression));
		}
		s = sizeof(volCompression);
		if(RegQueryValueEx(hkey, TEXT("ComPortVolCompression"), NULL, NULL, (LPBYTE)volCompression, &s) == ERROR_SUCCESS)
		{
			memcpy(g_player.GetChannelCompressionTable(1), volCompression, sizeof(volCompression));
		}

		BOOL isUseCompression;
		int ch1CompMinVelo, ch1MinThreshVol;
		g_player.GetRS232()->GetPianoCH1Settings(isUseCompression, ch1CompMinVelo, ch1MinThreshVol);
		s = 4;
		RegQueryValueEx(hkey, TEXT("PianoCH1UseCompression"), NULL, NULL, (LPBYTE)&isUseCompression, &s);
		s = 4;
		RegQueryValueEx(hkey, TEXT("PianoCH1CompMinVelo"), NULL, NULL, (LPBYTE)&ch1CompMinVelo, &s);
		s = 4;
		RegQueryValueEx(hkey, TEXT("PianoCH1MinThreshVol"), NULL, NULL, (LPBYTE)&ch1MinThreshVol, &s);
		g_player.GetRS232()->SetPianoCH1Settings(isUseCompression, ch1CompMinVelo, ch1MinThreshVol);

		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("ShowPath"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			g_showPathInPlaylist = val ? true : false;
		}
		s = sizeof(g_currentSkin);
		if(RegQueryValueEx(hkey, TEXT("Skin"), NULL, NULL, (LPBYTE)g_currentSkin, &s) != ERROR_SUCCESS)
		{
			lstrcpy(g_currentSkin, TEXT("decap2.msk"));
		}

//		if(!g_cmdFile[0])
		{
			s = sizeof(g_lastPlaylist);
			if(RegQueryValueEx(hkey, TEXT("Playlist"), NULL, NULL, (LPBYTE)g_lastPlaylist, &s) == ERROR_SUCCESS)
			{
//				if(fn[0])
//					g_playlist.Open(fn);
			}
		}
		RegCloseKey(hkey);
		return true;
	}
	else
	{
		//run first time, add autostart
		SetAutoStart(true);
	}
	return false;
}

bool SaveConfig()
{
	HKEY hkey;
	DWORD val, s;
	LPCTSTR p;

	if(RegCreateKeyEx(HKEY_CURRENT_USER, PLAYER_REG_KEY, 0, NULL, 0, NULL, NULL, &hkey, &s) == ERROR_SUCCESS)
	{
		int oport[CPlayer::NUM_OUTPUT_PORT];
		g_player.GetOutputPortsScheme(oport);
		RegSetValueEx(hkey, TEXT("OPortScheme"), 0, REG_BINARY, (LPBYTE)oport, sizeof(oport));
		val = g_player.GetSongDelay();
		RegSetValueEx(hkey, TEXT("SongDelay"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetMixVolume();
		RegSetValueEx(hkey, TEXT("MixVol"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetMp3Volume();
		RegSetValueEx(hkey, TEXT("Mp3Vol"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetVolume();
		RegSetValueEx(hkey, TEXT("SysVol"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetSoftSynthVolThreshold();
		RegSetValueEx(hkey, TEXT("SSVolThreshold"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetBassCutoff();
		RegSetValueEx(hkey, TEXT("BassCutoff"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetBassVolume();
		RegSetValueEx(hkey, TEXT("BassVol"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetCOMDelay();
		RegSetValueEx(hkey, TEXT("COMDelay"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetRS232()->GetOutPortId();
		RegSetValueEx(hkey, TEXT("COMOutId"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetRS232()->GetInPortId();
		RegSetValueEx(hkey, TEXT("COMInId"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetRS232()->GetBaudRate();
		RegSetValueEx(hkey, TEXT("COMBaudRate"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetShuffle();
		RegSetValueEx(hkey, TEXT("Shuffle"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetRepeat();
		RegSetValueEx(hkey, TEXT("Repeat"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetSingle();
		RegSetValueEx(hkey, TEXT("Single"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetAutoAdvance();
		RegSetValueEx(hkey, TEXT("AutoAdvance"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetReverse();
		RegSetValueEx(hkey, TEXT("ReverseStereo"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetSurround();
		RegSetValueEx(hkey, TEXT("Surround"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetReverb();
		RegSetValueEx(hkey, TEXT("Reverb"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetPanSep();
		RegSetValueEx(hkey, TEXT("PanSep"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetTimeMode();
		RegSetValueEx(hkey, TEXT("TimeMode"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetMaxPoly();
		RegSetValueEx(hkey, TEXT("MaxPoly"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetCloseAppOnActive();
		RegSetValueEx(hkey, TEXT("CloseOtherApps"), 0, REG_DWORD, (LPBYTE)&val, 4);
		val = g_player.GetEnableDecapCommand();
		RegSetValueEx(hkey, TEXT("RespondIR"), 0, REG_DWORD, (LPBYTE)&val, 4);
		p = g_playlist.GetListFile();
		RegSetValueEx(hkey, TEXT("Playlist"), 0, REG_SZ, (LPBYTE)p, (lstrlen(p) + 1)*sizeof(TCHAR));
		short delayTable[16];
		g_player.GetChannelDelayTable(delayTable);
		RegSetValueEx(hkey, TEXT("ChnDelays"), 0, REG_BINARY, (LPBYTE)delayTable, sizeof(delayTable));
		RegSetValueEx(hkey, TEXT("SoftSynthVolCompression"), 0, REG_BINARY, (LPBYTE)g_player.GetChannelCompressionTable(0), 16);
		RegSetValueEx(hkey, TEXT("ComPortVolCompression"), 0, REG_BINARY, (LPBYTE)g_player.GetChannelCompressionTable(1), 16);
		val = g_showPathInPlaylist ? 1 : 0;
		RegSetValueEx(hkey, TEXT("ShowPath"), 0, REG_DWORD, (LPBYTE)&val, 4);
		RegSetValueEx(hkey, TEXT("Skin"), 0, REG_SZ, (LPBYTE)g_currentSkin, (lstrlen(g_currentSkin) + 1)*sizeof(TCHAR));

		BOOL isUseCompression;
		int ch1CompMinVelo, ch1MinThreshVol;
		g_player.GetRS232()->GetPianoCH1Settings(isUseCompression, ch1CompMinVelo, ch1MinThreshVol);
		RegSetValueEx(hkey, TEXT("PianoCH1UseCompression"), 0, REG_DWORD, (LPBYTE)&isUseCompression, 4);
		RegSetValueEx(hkey, TEXT("PianoCH1CompMinVelo"), 0, REG_DWORD, (LPBYTE)&ch1CompMinVelo, 4);
		RegSetValueEx(hkey, TEXT("PianoCH1MinThreshVol"), 0, REG_DWORD, (LPBYTE)&ch1MinThreshVol, 4);

		RegCloseKey(hkey);
		return true;
	}
	return false;
}

bool OpenSkin()
{
	TCHAR fn[MAX_PATH];

	CUtils::MapToInstalledPath(g_currentSkin, fn);

	if(IsLandScapeMode())
		lstrcat(fn, TEXT(".l"));
	if(g_skin.OpenSkin(fn))
		return true;

	lstrcpy(g_currentSkin, TEXT("decap2.msk"));
	CUtils::MapToInstalledPath(g_currentSkin, fn);

	if(IsLandScapeMode())
		lstrcat(fn, TEXT(".l"));
	if(g_skin.OpenSkin(fn))
		return true;

/*
	CUtils::MapToInstalledPath(TEXT("*.msk"), fn);

	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(fn, &fd);
	if(h != INVALID_HANDLE_VALUE)
	{
		return g_skin.OpenSkin(CUtils::MapToInstalledPath(fd.cFileName, fn));
	}
*/
	return false;
}

void InitSkinValues()
{
	g_skin.SetBar(CSkin::CTL_POSITION, g_player.GetPosition(), g_player.GetMaxPostion() - 1);
	g_skin.SetBar(CSkin::CTL_VOLUME, g_player.GetVolume(), 0xffff);
	g_skin.DrawChannelNumbers();
}

BOOL AdaptToScreenMode()
{
	g_screenWidth = GetSystemMetrics(SM_CXSCREEN);
	g_screenHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT rc;
	GetWindowRect(hwndMain, &rc);
	rc.bottom -= MENU_HEIGHT;
	if (hwndCB)
		MoveWindow(hwndMain, rc.left, rc.top, rc.right, rc.bottom, FALSE);

	if(!OpenSkin())
	{
		MessageBox(NULL, TEXT("Open skin file failed!"), TEXT("Midi Player"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	InitSkinValues();
	g_player.ShowTitle();
	return TRUE;
}

bool InitPlayer()
{
	MEMORYSTATUS s;
	GlobalMemoryStatus(&s);
	if(s.dwAvailPhys < 2 * 1024 * 1024)
	{
		MessageBox(NULL, TEXT("Not enough memory!"), TEXT("Midi Player"), MB_OK | MB_ICONERROR);
		return false;
	}

	srand(GetTickCount());
	if(!g_player.Init())
	{
		MessageBox(NULL, TEXT("Open audio device failed!\nPlease close the program using audio device and try again."), TEXT("Midi Player"), MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

void ProcessCmd()
{
	LPTSTR p = GetCommandLine();
	LPTSTR pf;

	g_cmdFile[0] = 0;
#ifdef _X86_
//	MessageBox(NULL, p, NULL, MB_OK);
	if(*p == TEXT('\"'))
	{
		//find end of ""
		p++;
		while(*p && *p != TEXT('\"'))p++;
		if(!*p)return;
		p++;
	}
	while(*p && *p != TEXT(' ')) p++;
	if(!*p)return;
	while(*p && *p == TEXT(' '))p++;
	if(!*p)return;
#endif
	pf = g_cmdFile;
	if(*p == TEXT('\"'))
	{
		//find end of ""
		p++;
		while(*p && *p != TEXT('\"'))
			*pf++ = *p++;
		*pf = 0;
	}
	else
	{
		while(*p)
			*pf++ = *p++;
		*pf = 0;
	}
}

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	ProcessCmd();

//	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_MODPLAYER);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application 
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS	wc;

    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) WndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MODPLAYER));
    wc.hCursor			= 0;
    wc.hbrBackground	= NULL;
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= szWindowClass;

	return RegisterClass(&wc);
}

HMENU hMenuOptions;
HMENU hMenuMain;

static void checkReverbMenu(int id)
{
	CheckMenuItem(hMenuOptions, ID_REVERB_OFF, MF_BYCOMMAND | ((id == ID_REVERB_OFF)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenuOptions, ID_REVERB_LOW, MF_BYCOMMAND | ((id == ID_REVERB_LOW)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenuOptions, ID_REVERB_MIDDLE, MF_BYCOMMAND | ((id == ID_REVERB_MIDDLE)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenuOptions, ID_REVERB_HIGH, MF_BYCOMMAND | ((id == ID_REVERB_HIGH)?MF_CHECKED:MF_UNCHECKED));
}

static void UpdateReverbMenu()
{
	int reverb = g_player.GetReverb();
	if(reverb <= 0)
		checkReverbMenu(ID_REVERB_OFF);
	else if(reverb <= 10)
		checkReverbMenu(ID_REVERB_LOW);
	else if(reverb <= 20)
		checkReverbMenu(ID_REVERB_MIDDLE);
	else if(reverb <= 32)
		checkReverbMenu(ID_REVERB_HIGH);
}

static void checkPanSepMenu(int id)
{
	CheckMenuItem(hMenuOptions, ID_PAN_SEP_0, MF_BYCOMMAND | ((id == ID_PAN_SEP_0)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenuOptions, ID_PAN_SEP_50, MF_BYCOMMAND | ((id == ID_PAN_SEP_50)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenuOptions, ID_PAN_SEP_100, MF_BYCOMMAND | ((id == ID_PAN_SEP_100)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenuOptions, ID_PAN_SEP_150, MF_BYCOMMAND | ((id == ID_PAN_SEP_150)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenuOptions, ID_PAN_SEP_200, MF_BYCOMMAND | ((id == ID_PAN_SEP_200)?MF_CHECKED:MF_UNCHECKED));
}

static void checkKeyMenu(int id)
{
	int i;
	for(i = ID_TRANSPOSE0; i <= ID_TRANSPOSE12; i++)
	{
		CheckMenuItem(hMenuOptions, i, MF_BYCOMMAND | ((id == i) ? MF_CHECKED:MF_UNCHECKED));
	}
}

static void checkTempoMenu(int id)
{
	int i;
	for(i = ID_TEMPO0; i <= ID_TEMPO8; i++)
	{
		CheckMenuItem(hMenuOptions, i, MF_BYCOMMAND | ((id == i) ? MF_CHECKED:MF_UNCHECKED));
	}
}

static void UpdatePanSepMenu()
{
	int pansep = g_player.GetPanSep();
	if(pansep <= 0)
		checkPanSepMenu(ID_PAN_SEP_0);
	else if(pansep <= 50)
		checkPanSepMenu(ID_PAN_SEP_50);
	else if(pansep <= 100)
		checkPanSepMenu(ID_PAN_SEP_100);
	else if(pansep <= 150)
		checkPanSepMenu(ID_PAN_SEP_150);
	else if(pansep <= 200)
		checkPanSepMenu(ID_PAN_SEP_200);
}

static void UpdateSwapStereoMenu()
{
	CheckMenuItem(hMenuOptions, ID_SWAP_STEREO, MF_BYCOMMAND | (g_player.GetReverse()?MF_CHECKED:MF_UNCHECKED));
}

static void UpdateRandomMenu()
{
	CheckMenuItem(hMenuOptions, ID_RANDOM, MF_BYCOMMAND | (g_player.GetShuffle() ? MF_CHECKED:MF_UNCHECKED));
}

static void UpdateRepeatMenu()
{
	CheckMenuItem(hMenuOptions, ID_REPEAT, MF_BYCOMMAND | (g_player.GetRepeat() ? MF_CHECKED:MF_UNCHECKED));
}

static void UpdateSingleMenu()
{
	CheckMenuItem(hMenuOptions, ID_SINGLE, MF_BYCOMMAND | (g_player.GetSingle() ? MF_CHECKED:MF_UNCHECKED));
}

static void UpdateAutoAdvanceMenu()
{
	CheckMenuItem(hMenuOptions, ID_AUTOADVANCE, MF_BYCOMMAND | (g_player.GetAutoAdvance() ? MF_CHECKED:MF_UNCHECKED));
}
//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND	hWnd = NULL;
	TCHAR	szTitle[MAX_LOADSTRING];			// The title bar text
	TCHAR	szWindowClass[MAX_LOADSTRING];		// The window class name

	hInst = hInstance;		// Store instance handle in our global variable
	// Initialize global strings
	LoadString(hInstance, IDC_MODPLAYER, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

// 
// 	CPsWinDriver driver;
// 	if(driver.Open(22050, 8, 1000, true))
// 	{
// 		if(driver.Lock())
// 		{
// 			Sleep(50);
// 			driver.Unlock();
// 		}
// 		driver.Close();
// 	}
// 	
	//If it is already running, then focus on the window
	hWnd = FindWindow(szWindowClass, szTitle);	
	if (hWnd) 
	{
		SetForegroundWindow ((HWND) (((DWORD)hWnd) | 0x01));
		return 0;
	}

	if(!InitPlayer())
		return 0;

	MyRegisterClass(hInstance, szWindowClass);
	
	RECT	rect;
	GetClientRect(hWnd, &rect);
	
	hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{	
		return FALSE;
	}
	hwndMain = hWnd;

	g_screenWidth = GetSystemMetrics(SM_CXSCREEN);
	g_screenHeight = GetSystemMetrics(SM_CYSCREEN);
	LoadConfig();
	if(!AdaptToScreenMode())
		return FALSE;
	InitSkinValues();

	ShowWindow(hWnd, nCmdShow);
	if(TestAndPromptRegister2(hWnd))
	{
#ifndef DECAP_PLAYER
		if(g_cmdFile[0])
		{
			int len = lstrlen(g_cmdFile);
			if(len > 3 && lstrcmpi(g_cmdFile + len - 3, TEXT("mdl")) == 0)
				g_playlist.Open(g_cmdFile);
			else
				g_playlist.Add(g_cmdFile);
			g_player.Play();
		}
		else
#endif
		if(g_lastPlaylist[0])
			g_playlist.Open(g_lastPlaylist);
		SetTimer(hWnd, 111, 100, NULL);
	}
	UpdateWindow(hWnd);
	SetForegroundWindow(hWnd);

	hMenuOptions = (HMENU)SendMessage((hwndCB), SHCMBM_GETSUBMENU, (WPARAM)0, 
                              (LPARAM)ID_MENUITEM40031);

	hMenuMain = (HMENU)SendMessage((hwndCB), SHCMBM_GETSUBMENU, (WPARAM)0, 
                              (LPARAM)ID_MENU);
	if(g_player.GetCloseAppOnActive())
		CloseOtherApps(hWnd);
	UpdateReverbMenu();
	UpdateSwapStereoMenu();
	UpdateRandomMenu();
	UpdatePanSepMenu();
	UpdateSingleMenu();
	UpdateAutoAdvanceMenu();
	UpdateRepeatMenu();
	checkTempoMenu(ID_TEMPO4);
	checkKeyMenu(ID_TRANSPOSE6);
//	hMenuOptions = CommandBar_GetMenu(hwndCB, ID_MENUITEM40031);

	return TRUE;
}

void UpdateDisplay(HWND hWnd)
{
	HDC hdc = GetDC(hWnd);
	g_skin.BlitToScreen(hdc);
	ReleaseDC(hWnd, hdc);
}

int g_lastCtl = CSkin::CTL_NONE;

void SetStatus_Volume()
{
	TCHAR s[64];

	wsprintf(s, TEXT("Volume: %d%%"), g_player.GetVolume() * 100 / 0xffff);
	g_skin.SetTitle(s);
}

void SetStatus_Position(int pos)
{
	TCHAR s[64];

	if(g_player.GetMaxPostion() > 0)
	{
		wsprintf(s, TEXT("Position: %d%%"), pos * 100 / g_player.GetMaxPostion());
		g_skin.SetTitle(s);
	}
}

int startX = -1, startY = -1;
bool isDragMode;
//extern int testcmd;

void HandleStylusDown(HWND hWnd, int x, int y)
{
	SetCapture(hWnd);

	startX = x;
	startY = y;
	isDragMode = false;

	g_lastCtl = g_skin.CtlFromPos(x, y);
	switch(g_lastCtl) {
	case CSkin::CTL_PLAY:
		g_skin.SetButton(CSkin::CTL_PLAY, true);
		UpdateDisplay(hWnd);
		g_player.Play();
		break;
	case CSkin::CTL_PAUSE:
		g_skin.SetButton(CSkin::CTL_PAUSE, true);
		g_player.Pause();
		break;
	case CSkin::CTL_STOP:
		g_skin.SetButton(CSkin::CTL_STOP, true);
		g_player.Stop();
		break;
	case CSkin::CTL_PREV:
		g_skin.SetButton(CSkin::CTL_PREV, true);
		UpdateDisplay(hWnd);
		g_player.Prev();
		break;
	case CSkin::CTL_NEXT:
		g_skin.SetButton(CSkin::CTL_NEXT, true);
		UpdateDisplay(hWnd);
		g_player.Next(true);
		break;
	case CSkin::CTL_VOLUME:
		g_player.SetVolume(g_skin.HandleMouseForBar(CSkin::CTL_VOLUME, x, y, 0xffff));
		SetStatus_Volume();
		break;
//	case CSkin::CTL_POSITION:
//		g_player.m_setposbar = false;
//		SetStatus_Position(g_skin.HandleMouseForBar(CSkin::CTL_POSITION, x, y, g_player.GetMaxPostion() - 1));
//		break;
	case CSkin::CTL_PLAYLIST_DOWN:
		g_skin.PlaylistLineDown();
		break;
	case CSkin::CTL_PLAYLIST_UP:
		g_skin.PlaylistLineUp();
		break;
	case CSkin::CTL_PLAYLIST_SCROLL:
		g_skin.PlaylistHitScroll(x, y);
		break;
	case CSkin::CTL_PLAYLIST:
		{
			int sel = g_skin.PlaylistItemFromPos(x, y);
			if(sel >= 0)
			{
				g_player.SelectPlaylistItem(sel);
			}
		}
		break;
	case CSkin::CTL_TIME_PAN:
		g_player.SetTimeMode((g_player.GetTimeMode() + 1) % 3);
//		testcmd  =1;
		break;
	default:
		return;
	}
	UpdateDisplay(hWnd);
}

void HandleStylusMove(HWND hWnd, int x, int y)
{
	int ctl = g_skin.CtlFromPos(x, y);
	if(ctl != g_lastCtl)
	{
		switch(g_lastCtl)
		{
		case CSkin::CTL_PLAY:
		case CSkin::CTL_PAUSE:
		case CSkin::CTL_STOP:
		case CSkin::CTL_PREV:
		case CSkin::CTL_NEXT:
			g_skin.SetButton(g_lastCtl, false);
			UpdateDisplay(hWnd);
			g_lastCtl = CSkin::CTL_NONE;
			return;
		}
	}
	switch(g_lastCtl)
	{
	case CSkin::CTL_VOLUME:
		g_player.SetVolume(g_skin.HandleMouseForBar(CSkin::CTL_VOLUME, x, y, 0xffff));
		SetStatus_Volume();
		UpdateDisplay(hWnd);
		break;
//	case CSkin::CTL_POSITION:
//		g_skin.HandleMouseForBar(CSkin::CTL_POSITION, x, y, g_player.GetMaxPostion() - 1);
//		SetStatus_Position(g_skin.HandleMouseForBar(CSkin::CTL_POSITION, x, y, g_player.GetMaxPostion() - 1));
//		UpdateDisplay(hWnd);
//		break;
	case CSkin::CTL_PLAYLIST_SCROLL:
		if(isDragMode || abs(y - startY) > 5)
		{
			isDragMode = true;
			g_skin.PlaylistHitScroll(x, y);
		}
		break;
	}
}

void HandleStylusUp(HWND hWnd, int x, int y)
{
	startX = -1;
	startY = -1;
	isDragMode = false;
	switch(g_lastCtl)
	{
	case CSkin::CTL_PLAY:
	case CSkin::CTL_PAUSE:
	case CSkin::CTL_STOP:
	case CSkin::CTL_PREV:
	case CSkin::CTL_NEXT:
		g_skin.SetButton(g_lastCtl, false);
		UpdateDisplay(hWnd);
		break;
//	case CSkin::CTL_POSITION:
//		g_player.m_setposbar = true;
//		g_player.SetPosition(g_skin.HandleMouseForBar(CSkin::CTL_POSITION, x, y, g_player.GetMaxPostion() - 1));
	case CSkin::CTL_VOLUME:
		g_player.ShowTitle();
		g_player.SynchronizeCOMVolume();
		break;
	}
	g_lastCtl = CSkin::CTL_NONE;
	ReleaseCapture();
}

int totalPlayerTime;

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int wmId, wmEvent;
	PAINTSTRUCT ps;

	switch (message) 
	{
		case WM_ACTIVATE:
			if(LOWORD(wParam) != WA_INACTIVE)
			{
				if(g_player.GetCloseAppOnActive())
					CloseOtherApps(hWnd);
			}
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{	
			case ID_IS_VOLUME:
				{
					CDlgMasterVolume dlg;
					dlg.ShowModal(hWnd);
				}
				break;
			case ID_IS_MIDICHANNEL:
				{
					CDlgMidiChannel dlg;
					dlg.ShowModal(hWnd);
				}
				break;
			case ID_IS_MIDITHRUMODE:
				{
					CDlgMidiThru dlg;
					dlg.ShowModal(hWnd);
				}
				break;
			case ID_IS_PRESSURE:
				{
					CDlgPressure dlg;
					dlg.ShowModal(hWnd);
				}
				break;
			case ID_CHAN_DELAY:
				{
					CDlgChnDelay dlg;
					dlg.ShowModal(hWnd);
				}
				break;
			case ID_COM_VCOMPRESSION:
				{
					CDlgChnCompression dlg(1);
					dlg.ShowModal(hWnd);
				}
				break;
			case ID_SOFT_VCOMPRESSION:
				{
					CDlgChnCompression dlg(0);
					dlg.ShowModal(hWnd);
				}
				break;
				
			case IDM_HELP_ABOUT:
				DialogBox(hInst, (LPCTSTR)(IsLandScapeMode() ? IDD_ABOUTBOX_LS: IDD_ABOUTBOX), hWnd, (DLGPROC)About);
				break;
			case IDOK:
			case ID_APP_EXIT:
				SendMessage(hWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWnd);
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
			case ID_PL_ADD:
				g_playlist.ShowDlgAdd(hWnd);
				g_skin.DrawPlayList();
				break;
			case ID_PL_EDIT:
				{
					CDlgPlEdit dlg(&g_playlist);
					dlg.ShowModal(hWnd);
					g_skin.DrawPlayList();
				}
				break;
			case ID_PL_NEW:
				g_playlist.NewList();
				g_skin.DrawPlayList();
				break;
			case ID_PL_SAVE:
				g_playlist.Save(hWnd);
				break;
			case ID_PL_OPEN:
				g_playlist.Open(hWnd);
				break;
				
			case ID_SONG_NEXT:
				g_skin.SetButton(CSkin::CTL_NEXT, true);
				UpdateDisplay(hWnd);
				g_player.Next(true);
				g_skin.SetButton(CSkin::CTL_NEXT, false);
				UpdateDisplay(hWnd);
				break;
			case ID_SONG_PREV:
				g_skin.SetButton(CSkin::CTL_PREV, true);
				UpdateDisplay(hWnd);
				g_player.Prev();
				g_skin.SetButton(CSkin::CTL_PREV, false);
				UpdateDisplay(hWnd);
				break;
			case ID_SONG_PLAY:
				if(g_player.GetStatus() == CPlayer::STATUS_PLAYING)
				{
					g_skin.SetButton(CSkin::CTL_PAUSE, true);
					UpdateDisplay(hWnd);
					g_player.Pause();
					g_skin.SetButton(CSkin::CTL_PAUSE, false);
					UpdateDisplay(hWnd);
				}
				else {
					g_skin.SetButton(CSkin::CTL_PLAY, true);
					UpdateDisplay(hWnd);
					g_player.Play();
					g_skin.SetButton(CSkin::CTL_PLAY, false);
					UpdateDisplay(hWnd);
				}
				break;
			case ID_SONG_STOP:
				g_skin.SetButton(CSkin::CTL_STOP, true);
				UpdateDisplay(hWnd);
				g_player.Stop();
				g_skin.SetButton(CSkin::CTL_STOP, false);
				UpdateDisplay(hWnd);
				break;
				
			case ID_SKIN:
				{
					CDlgSkin dlg(g_skin.m_skinfile);
					if(dlg.ShowModal(hWnd) == IDOK && dlg.m_file[0])
					{
						TCHAR f[MAX_PATH];
						CUtils::MapToInstalledPath(dlg.m_file, f);
						if(IsLandScapeMode())
							lstrcat(f, TEXT(".l"));
						if(g_skin.OpenSkin(f))
						{
							InitSkinValues();
							g_player.ShowTitle();
						}
					}
					SaveConfig();
					break;
				}
				break;

			case ID_SKIN_1:
				lstrcpy(g_currentSkin, TEXT("decap.msk"));
				AdaptToScreenMode();
				SaveConfig();
				break;
			case ID_SKIN_2:
				lstrcpy(g_currentSkin, TEXT("decap2.msk"));
				AdaptToScreenMode();
				SaveConfig();
				break;
				
			case ID_REVERB_OFF:
				g_player.SetReverb(0);
				checkReverbMenu(wmId);
				SaveConfig();
				break;
			case ID_REVERB_LOW:
				g_player.SetReverb(10);
				checkReverbMenu(wmId);
				SaveConfig();
				break;
			case ID_REVERB_MIDDLE:
				g_player.SetReverb(20);
				checkReverbMenu(wmId);
				SaveConfig();
				break;
			case ID_REVERB_HIGH:
				g_player.SetReverb(32);
				checkReverbMenu(wmId);
				SaveConfig();
				break;
				
			case ID_PAN_SEP_0:
				g_player.SetPanSep(0);
				checkPanSepMenu(wmId);
				SaveConfig();
				break;
			case ID_PAN_SEP_50:
				g_player.SetPanSep(50);
				checkPanSepMenu(wmId);
				SaveConfig();
				break;
			case ID_PAN_SEP_100:
				g_player.SetPanSep(100);
				checkPanSepMenu(wmId);
				SaveConfig();
				break;
			case ID_PAN_SEP_150:
				g_player.SetPanSep(150);
				checkPanSepMenu(wmId);
				SaveConfig();
				break;
			case ID_PAN_SEP_200:
				g_player.SetPanSep(200);
				checkPanSepMenu(wmId);
				SaveConfig();
				break;
				
			case ID_SWAP_STEREO:
				g_player.SetReverse(!g_player.GetReverse());
				UpdateSwapStereoMenu();
				SaveConfig();
				break;
			case ID_RANDOM:
				g_player.SetShuffle(!g_player.GetShuffle());
				UpdateRandomMenu();
				g_skin.DrawPlayList();
				SaveConfig();
				break;
			case ID_REPEAT:
				g_player.SetRepeat(!g_player.GetRepeat());
				UpdateRepeatMenu();
				SaveConfig();
				break;
			case ID_SINGLE:
				g_player.SetSingle(!g_player.GetSingle());
				UpdateSingleMenu();
				SaveConfig();
				break;
			case ID_AUTOADVANCE:
				g_player.SetAutoAdvance(!g_player.GetAutoAdvance());
				UpdateAutoAdvanceMenu();
				SaveConfig();
				break;
			case ID_OPTIONS:
				{
					CDlgOption dlg;
					dlg.ShowModal(hWnd);
					SaveConfig();
				}
				break;
			case ID_COMPORT:
				{
					CDlgComPort dlg;
					dlg.ShowModal(hWnd);
				}
				break;
			case ID_VOLUMES:
				{
					CDlgVolumes dlg;
					dlg.ShowModal(hWnd);
				}
				break;
				
			case ID_CHANNEL:
				{
					CDlgChannel dlg;
					dlg.ShowModal(hWnd);
					g_skin.DrawChannelNumbers();
					SaveConfig();
				}
				break;

			case ID_PIANO_CH1_SETTINGS:
				{
					CDlgPianoCH1Settings dlg;
					dlg.ShowModal(hWnd);
				}
				
			case ID_ALLCONTROLLEROFF:
				g_player.AllControllerOff();
				break;
				
			default:
				if(wmId >= ID_TEMPO0 && wmId <= ID_TEMPO8)
				{
					checkTempoMenu(wmId);
					g_player.SetTempo(wmId - ID_TEMPO0);
					SaveConfig();
					break;
				}
				else if(wmId >= ID_TRANSPOSE0 && wmId <= ID_TRANSPOSE12)
				{
					checkKeyMenu(wmId);
					g_player.SetKeyTranspose(wmId - ID_TRANSPOSE0 - 6);
					SaveConfig();
					break;
				}
				else
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_CREATE:
			hwndCB = CreateRpCommandBar(hWnd);
#ifdef NDEBUG
//			Decrypt((char*)&RegistrationProc, (char*)&Regist_GetDaysUsed);
#endif
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			g_skin.BlitToScreen(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
			EndPaint(hWnd, &ps);
			break; 
		case WM_DESTROY:
			KillTimer(hWnd, 1);
			SaveConfig();
			CommandBar_Destroy(hwndCB);
			g_player.Destroy();
			PostQuitMessage(0);
			break;
		case WM_SETTINGCHANGE:
			SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
     		break;
		case WM_TIMER:
			if(wParam == 111)
			{
#ifdef COUNT_TICK
				static totaltick = 0, counttick = 0, disptick;
				int tick = GetTickCount();
#endif
				if(g_screenWidth != GetSystemMetrics(SM_CXSCREEN))
					AdaptToScreenMode();
				g_player.Update();
				static int lastStatus = -1;
				if(lastStatus != g_player.GetStatus())
				{
					lastStatus = g_player.GetStatus();
					if(lastStatus == CPlayer::STATUS_READY)
					{
						EnableMenuItem(hMenuMain, 0, MF_BYPOSITION | MF_ENABLED);
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_OPEN, MAKELONG(TRUE, 0));
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_SAVE, MAKELONG(TRUE, 0));
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_ADD, MAKELONG(TRUE, 0));
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_EDIT, MAKELONG(TRUE, 0));
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_NEW, MAKELONG(TRUE, 0));
					}
					else
					{
						EnableMenuItem(hMenuMain, 0, MF_BYPOSITION | MF_GRAYED);
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_OPEN, MAKELONG(FALSE, 0));
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_SAVE, MAKELONG(FALSE, 0));
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_ADD, MAKELONG(FALSE, 0));
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_EDIT, MAKELONG(FALSE, 0));
						SendMessage(hwndCB, TB_ENABLEBUTTON, ID_PL_NEW, MAKELONG(FALSE, 0));
					}
				}
				if(GetForegroundWindow() == hWnd)
					UpdateDisplay(hWnd);
#ifdef COUNT_TICK
				totaltick += GetTickCount() - tick;
				if(++counttick == 10)
				{
					disptick = totaltick;
					totaltick = 0;
					counttick = 0;
				}
				CPsObject::PsPrintDebugString("%d, %x, %d, %d, %d  ", disptick, totalPlayerTime, g_player.GetRS232()->m_readIndex, g_player.GetRS232()->m_writeIndex, g_player.GetRS232()->m_totalReceived);
#endif
				if(--g_regtick <= 0)
				{
					KillTimer(hWnd, 111);
					TestAndPromptRegister(hWnd);
					g_regtick = 10 * 60 + (GetTickCount() & 1023);
					SetTimer(hWnd, 111, 100, NULL);
				}
			}
			break;
		case WM_LBUTTONDOWN:
			HandleStylusDown(hWnd, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_LBUTTONUP:
			HandleStylusUp(hWnd, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MOUSEMOVE:
			HandleStylusMove(hWnd, LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

HWND CreateRpCommandBar(HWND hwnd)
{
	SHMENUBARINFO mbi;

	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize     = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hwnd;
	mbi.nToolBarId = IDM_MENU;
	mbi.hInstRes   = hInst;
	mbi.nBmpId     = IDB_MENUBAR;
	mbi.cBmpImages = 8;

	if (!SHCreateMenuBar(&mbi)) 
		return NULL;

	return mbi.hwndMB;
}

// Mesage handler for the About box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SHINITDLGINFO shidi;

	switch (message)
	{
		case WM_INITDIALOG:
			// Create a Done button and size it.  
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);
			if(Regist_IsRegistered())
			{
				SetDlgItemText(hDlg, IDC_REGISTER, TEXT("Upgrade"));
				ShowWindow(GetDlgItem(hDlg, IDC_LICENSED), SW_SHOW);
			}
			return TRUE; 

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDOK:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			case IDC_REGISTER:
				RegisterDlg(hDlg);
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
