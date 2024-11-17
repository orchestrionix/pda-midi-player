#include "stdafx.h"
#include "resource.h"
#include "DlgOption.h"
#include "Player.h"
#include "utils.h"
#include "Common/PsMath.h"
#include "PsPortRS232.h"
#include <Notify.h>

CDlgOption::CDlgOption():CDlgBase((LPCTSTR)IDD_OPTIONS, (LPCTSTR)IDD_OPTIONS_LS)
{
}

CDlgOption::~CDlgOption()
{

}

void SetAutoStart(bool isEnabled);

LRESULT CDlgOption::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		{
			int a[2];
			a[0] = SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);
			a[1] = SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_GETCURSEL, 0, 0);
			g_player.SetMaxPoly(GetDlgItemInt(m_hWnd, IDC_MAX_POLY, NULL, FALSE));
			g_player.SetSoftSynthVolThreshold(GetDlgItemInt(m_hWnd, IDC_EDIT1, NULL, TRUE));
			g_player.SetCOMDelay(GetDlgItemInt(m_hWnd, IDC_EDIT2, NULL, TRUE));
			g_player.SetSongDelay(GetDlgItemInt(m_hWnd, IDC_SONG_DELAY, NULL, FALSE));
			g_player.SetOutputPortsScheme(a, 2);
		}
		SaveConfig();
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	case IDC_AUTO_ENABLE:
		SetAutoStart(SendDlgItemMessage(m_hWnd, IDC_AUTO_ENABLE, BM_GETCHECK, 0, 0) == BST_CHECKED);
		return TRUE;
	case IDC_CLOSE_OTHER_APP:
		g_player.SetCloseAppOnActive(SendDlgItemMessage(m_hWnd, IDC_CLOSE_OTHER_APP, BM_GETCHECK, 0, 0) == BST_CHECKED);
		return TRUE;
	case IDC_RESPOND_IR:
		g_player.SetEnableDecapCommand(SendDlgItemMessage(m_hWnd, IDC_RESPOND_IR, BM_GETCHECK, 0, 0) == BST_CHECKED);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgOption::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	Update();
}

void CDlgOption::Update()
{
	int n;
	CPsOutputPort **p = g_player.GetStockedOutputPorts(&n);
	for(int i = 0; i < n; i++)
	{
		TCHAR desc[100];
		MultiByteToWideChar(CP_OEMCP, 0, p[i]->GetName(), -1, desc, 100);
		SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)desc);
		SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)desc);
	}
	int a[2];
	n = g_player.GetOutputPortsScheme(a);
	CPsObject::PsAssert(n == 2);
	SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_SETCURSEL, a[0], 0);
	SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_SETCURSEL, a[1], 0);
	if(g_player.GetStatus() == CPlayer::STATUS_PLAYING)
	{
		EnableWindow(GetDlgItem(m_hWnd, IDC_COMBO1), FALSE);
		EnableWindow(GetDlgItem(m_hWnd, IDC_COMBO2), FALSE);
	}

	SetDlgItemInt(m_hWnd, IDC_EDIT1, g_player.GetSoftSynthVolThreshold(), TRUE);
	SetDlgItemInt(m_hWnd, IDC_EDIT2, g_player.GetCOMDelay(), TRUE);
	SetDlgItemInt(m_hWnd, IDC_SONG_DELAY, g_player.GetSongDelay(), FALSE);
	SetDlgItemInt(m_hWnd, IDC_MAX_POLY, g_player.GetMaxPoly(), FALSE);
	SendDlgItemMessage(m_hWnd, IDC_CLOSE_OTHER_APP, BM_SETCHECK, g_player.GetCloseAppOnActive() ? BST_CHECKED : BST_UNCHECKED, 0);
	WPARAM checked = BST_UNCHECKED;
	HKEY hkey;
	DWORD val, s;
	if(RegOpenKeyEx(HKEY_CURRENT_USER, PLAYER_REG_KEY, 0, 0, &hkey) == ERROR_SUCCESS)
	{
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("AutoStart"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
			checked = val == 0 ? BST_UNCHECKED : BST_CHECKED;
	}
	SendDlgItemMessage(m_hWnd, IDC_AUTO_ENABLE, BM_SETCHECK, checked, 0);
	SendDlgItemMessage(m_hWnd, IDC_RESPOND_IR, BM_SETCHECK, g_player.GetEnableDecapCommand() ? BST_CHECKED : BST_UNCHECKED, 0);
}

static const DWORD gs_availableBaudRates[] = {
	CBR_38400,
	CBR_56000,
	CBR_57600,
	CBR_115200
};

CDlgComPort::CDlgComPort():CDlgBase((LPCTSTR)IDD_COMPORT, (LPCTSTR)IDD_COMPORT_LS)
{
}

CDlgComPort::~CDlgComPort()
{
	
}

LRESULT CDlgComPort::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		{
			int a[3];
			a[0] = SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_GETCURSEL, 0, 0) + 1;
			a[1] = SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_GETCURSEL, 0, 0) + 1;
			a[2] = SendDlgItemMessage(m_hWnd, IDC_COMBO3, CB_GETCURSEL, 0, 0);
			g_player.GetRS232()->SetInPortId(a[0]);
			g_player.GetRS232()->SetOutPortId(a[1]);
			g_player.GetRS232()->SetBaudRate(gs_availableBaudRates[a[2]]);
			MessageBox(m_hWnd, TEXT("You need to restart the player to apply the new settings."), TEXT("DecapPlayer"), MB_ICONEXCLAMATION | MB_OK);
		}
		SaveConfig();
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgComPort::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	for(int i = 1; i < 10; i++)
	{
		TCHAR desc[100];
		wsprintf(desc, TEXT("COM%d"), i);
		SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)desc);
		SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)desc);
	}
	for(i = 0; i < (sizeof(gs_availableBaudRates) / sizeof(gs_availableBaudRates[0])); ++i) {
		TCHAR desc[100];
		wsprintf(desc, TEXT("%d"), gs_availableBaudRates[i]);
		SendDlgItemMessage(m_hWnd, IDC_COMBO3, CB_ADDSTRING, 0, (LPARAM)desc);
		if (g_player.GetRS232()->GetBaudRate() == gs_availableBaudRates[i])
		{
			SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_SETCURSEL, i, 0);
		}
	}
	Update();
}

void CDlgComPort::Update()
{
	SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_SETCURSEL, g_player.GetRS232()->GetInPortId() - 1, 0);
	SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_SETCURSEL, g_player.GetRS232()->GetOutPortId() - 1, 0);
	for(int i = 0; i < (sizeof(gs_availableBaudRates) / sizeof(gs_availableBaudRates[0])); ++i) {
		if (g_player.GetRS232()->GetBaudRate() == gs_availableBaudRates[i])
		{
			SendDlgItemMessage(m_hWnd, IDC_COMBO3, CB_SETCURSEL, i, 0);
			break;
		}
	}
}

CDlgVolumes::CDlgVolumes():CDlgBase((LPCTSTR)IDD_VOLUMES, (LPCTSTR)IDD_VOLUMES_LS)
{
}

CDlgVolumes::~CDlgVolumes()
{

}

LRESULT CDlgVolumes::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		SaveConfig();
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgVolumes::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	Update();
}

void CDlgVolumes::Update()
{
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	info.nMax = F_ONE - F_ONE / 8 - 1;
	info.nMin = 0;
	info.nPage = 16;
	info.nPos = g_player.GetMixVolume() - F_ONE / 8;
	SetScrollInfo(GetDlgItem(m_hWnd, IDC_SCROLLBAR1), SB_CTL, &info, TRUE);
	
	info.nMax = F_ONE - 1;
	info.nMin = 0;
	info.nPage = 16;
	info.nPos = g_player.GetBassVolume();
	SetScrollInfo(GetDlgItem(m_hWnd, IDC_SCROLLBAR2), SB_CTL, &info, TRUE);
	
	info.nMax = 160;
	info.nMin = 0;
	info.nPage = 16;
	info.nPos = g_player.GetBassCutoff() - 40;
	SetScrollInfo(GetDlgItem(m_hWnd, IDC_SCROLLBAR3), SB_CTL, &info, TRUE);
	
	info.nMax = F_ONE - 1;
	info.nMin = 0;
	info.nPage = 16;
	info.nPos = g_player.GetMp3Volume();
	SetScrollInfo(GetDlgItem(m_hWnd, IDC_SCROLLBAR4), SB_CTL, &info, TRUE);
}

LRESULT CDlgVolumes::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_HSCROLL:
		if((HWND)lParam == GetDlgItem(m_hWnd, IDC_SCROLLBAR1))
		{
			F16 vol = (F16)(g_player.GetMixVolume() - F_ONE / 8);
			switch(LOWORD(wParam))
			{
			case SB_PAGELEFT:
				vol -= F_ONE / 16;
				break;
			case SB_PAGERIGHT:
				vol += F_ONE / 16;
				break;
			case SB_LINELEFT:
				vol -= F_ONE / 64;
				break;
			case SB_LINERIGHT:
				vol += F_ONE / 64;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				vol = HIWORD(wParam);
				break;
			default:
				return TRUE;
			}
			if(vol < 0)
				vol = 0;
			if(vol > F_ONE - F_ONE / 8)
				vol = F_ONE  - F_ONE / 8;
			g_player.SetMixVolume((F16)(vol  + F_ONE / 8));
			SCROLLINFO info;
			info.cbSize = sizeof(info);
			info.fMask = SIF_POS;
			info.nPos = g_player.GetMixVolume() - F_ONE / 8;
			SetScrollInfo(GetDlgItem(m_hWnd, IDC_SCROLLBAR1), SB_CTL, &info, TRUE);
			return TRUE;
		}
		else if((HWND)lParam == GetDlgItem(m_hWnd, IDC_SCROLLBAR2))
		{
			F16 vol = (F16)(g_player.GetBassVolume());
			switch(LOWORD(wParam))
			{
			case SB_PAGELEFT:
				vol -= F_ONE / 16;
				break;
			case SB_PAGERIGHT:
				vol += F_ONE / 16;
				break;
			case SB_LINELEFT:
				vol -= F_ONE / 64;
				break;
			case SB_LINERIGHT:
				vol += F_ONE / 64;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				vol = HIWORD(wParam);
				break;
			default:
				return TRUE;
			}
			if(vol < 0)
				vol = 0;
			if(vol > F_ONE)
				vol = F_ONE;
			g_player.SetBassVolume((F16)(vol));
			SCROLLINFO info;
			info.cbSize = sizeof(info);
			info.fMask = SIF_POS;
			info.nPos = g_player.GetBassVolume();
			SetScrollInfo(GetDlgItem(m_hWnd, IDC_SCROLLBAR2), SB_CTL, &info, TRUE);
			return TRUE;
		}
		else if((HWND)lParam == GetDlgItem(m_hWnd, IDC_SCROLLBAR3))
		{
			int vol = (g_player.GetBassCutoff() - 40);
			switch(LOWORD(wParam))
			{
			case SB_PAGELEFT:
				vol -= 10;
				break;
			case SB_PAGERIGHT:
				vol += 10;
				break;
			case SB_LINELEFT:
				vol -= 2;
				break;
			case SB_LINERIGHT:
				vol += 2;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				vol = HIWORD(wParam);
				break;
			default:
				return TRUE;
			}
			if(vol < 0)
				vol = 0;
			if(vol > 160)
				vol = 160;
			g_player.SetBassCutoff(vol + 40);
			SCROLLINFO info;
			info.cbSize = sizeof(info);
			info.fMask = SIF_POS;
			info.nPos = g_player.GetBassCutoff() - 40;
			SetScrollInfo(GetDlgItem(m_hWnd, IDC_SCROLLBAR3), SB_CTL, &info, TRUE);
			return TRUE;
		}
		else if((HWND)lParam == GetDlgItem(m_hWnd, IDC_SCROLLBAR4))
		{
			F16 vol = (F16)(g_player.GetMp3Volume());
			switch(LOWORD(wParam))
			{
			case SB_PAGELEFT:
				vol -= F_ONE / 16;
				break;
			case SB_PAGERIGHT:
				vol += F_ONE / 16;
				break;
			case SB_LINELEFT:
				vol -= F_ONE / 64;
				break;
			case SB_LINERIGHT:
				vol += F_ONE / 64;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				vol = HIWORD(wParam);
				break;
			default:
				return TRUE;
			}
			if(vol < 0)
				vol = 0;
			if(vol > F_ONE)
				vol = F_ONE;
			g_player.SetMp3Volume((F16)(vol));
			SCROLLINFO info;
			info.cbSize = sizeof(info);
			info.fMask = SIF_POS;
			info.nPos = g_player.GetMp3Volume();
			SetScrollInfo(GetDlgItem(m_hWnd, IDC_SCROLLBAR4), SB_CTL, &info, TRUE);
			return TRUE;
		}
		break;
	}
	return CDlgBase::WndProc(message, wParam, lParam);
}

