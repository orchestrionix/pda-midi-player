#include "stdafx.h"
#include "resource.h"
#include "DlgDecapSetting.h"
#include "Player.h"
#include "utils.h"
#include "PsPortRS232.h"
#include <Commctrl.h>

CDlgMasterVolume::CDlgMasterVolume():CDlgBase((LPCTSTR)IDD_IS_VOLUME, (LPCTSTR)IDD_IS_VOLUME_LS)
{
	CPsPortRS232 *port = g_player.GetRS232();
	port->Reset();
	port->UpdateDecapNRPNSetting();
	m_volume = port->GetDecapNRPNSetting(0);
	if(m_volume < 0)
		m_volume = 63;
}

CDlgMasterVolume::~CDlgMasterVolume()
{

}

LRESULT CDlgMasterVolume::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		{
			m_volume = (unsigned char)SendDlgItemMessage(m_hWnd, IDC_SLIDER1, TBM_GETPOS, 0, 0);
			g_player.SendInstrumentSetting(0, m_volume);
		}
		SaveConfig();
		EndDialog(m_hWnd, IDOK);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, IDCANCEL);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgMasterVolume::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	Update();
}

void CDlgMasterVolume::Update()
{
	SendDlgItemMessage(m_hWnd, IDC_SLIDER1, TBM_SETRANGE, FALSE, (LPARAM)MAKELONG(0, 127));
	SendDlgItemMessage(m_hWnd, IDC_SLIDER1, TBM_SETPAGESIZE, 0, 8);
	SendDlgItemMessage(m_hWnd, IDC_SLIDER1, TBM_SETTICFREQ, 8, 0);
	SendDlgItemMessage(m_hWnd, IDC_SLIDER1, TBM_SETPOS, TRUE, m_volume);
}


CDlgMidiChannel::CDlgMidiChannel():CDlgBase((LPCTSTR)IDD_IS_MIDICHN, (LPCTSTR)IDD_IS_MIDICHN_LS)
{
	g_player.AllControllerOff();
	g_player.GetRS232()->SendStatus(0xFF);
	Sleep(500);
	m_mainChannel = g_player.GetAccordionMainMidChan();
	m_subChannel = g_player.GetAccordionSubMidChan();
	m_activeSensing = g_player.GetActiveSensing();
}

CDlgMidiChannel::~CDlgMidiChannel()
{

}

LRESULT CDlgMidiChannel::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDC_STORE:
	case IDOK:
		{
			int newchn = (unsigned char)SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);
			if(m_mainChannel != CB_ERR && newchn != m_mainChannel)
			{
				m_mainChannel = newchn;
//				g_player.SendInstrumentSetting(1, m_channel);
				g_player.SetAccordionMainMidChan(newchn);
//				if(idCtl == IDC_STORE)
//				{
//					g_player.SendInstrumentSetting(127, 127);
//				}
			}
			newchn = (unsigned char)SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_GETCURSEL, 0, 0);
			if(m_subChannel != CB_ERR && newchn != m_subChannel)
			{
				m_subChannel = newchn;
//				g_player.SendInstrumentSetting(1, m_channel);
				g_player.SetAccordionSubMidChan(newchn);
//				if(idCtl == IDC_STORE)
//				{
//					g_player.SendInstrumentSetting(127, 127);
//				}
			}
			int newActiveSensing = SendDlgItemMessage(m_hWnd, IDC_ACTIVESENSING, BM_GETCHECK, 0, 0) == BST_CHECKED;
			if(newActiveSensing != m_activeSensing)
			{
				m_activeSensing	= newActiveSensing;
				g_player.SetActiveSensing(newActiveSensing);
			}
		}
		SaveConfig();
		EndDialog(m_hWnd, IDOK);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, IDCANCEL);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgMidiChannel::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	Update();
}

void CDlgMidiChannel::Update()
{
	int i;
	TCHAR s[40];
	for(i = 0; i < 16; i++)
	{
		wsprintf(s, TEXT("Channel %d"), i + 1);
		SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)s);
	}
	if(m_mainChannel >= 0)
		SendDlgItemMessage(m_hWnd, IDC_COMBO1, CB_SETCURSEL, m_mainChannel, 0);
	for(i = 0; i < 16; i++)
	{
		wsprintf(s, TEXT("Channel %d"), i + 1);
		SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)s);
	}
	
	SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)L"No Sub-Channel");
	if(m_subChannel >= 0)
		SendDlgItemMessage(m_hWnd, IDC_COMBO2, CB_SETCURSEL, m_subChannel, 0);
	SendDlgItemMessage(m_hWnd, IDC_ACTIVESENSING, BM_SETCHECK, m_activeSensing ? BST_CHECKED : BST_UNCHECKED, 0);
}

CDlgMidiThru::CDlgMidiThru():CDlgBase((LPCTSTR)IDD_IS_MIDITHRU, (LPCTSTR)IDD_IS_MIDITHRU_LS)
{
	CPsPortRS232 *port = g_player.GetRS232();
	port->Reset();
	port->UpdateDecapNRPNSetting();
	m_mode = port->GetDecapNRPNSetting(2);
}

CDlgMidiThru::~CDlgMidiThru()
{

}

LRESULT CDlgMidiThru::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDC_STORE:
	case IDOK:
		{
			m_mode = -1;
			if(SendDlgItemMessage(m_hWnd, IDC_RADIO1, BM_GETCHECK, 0, 0) == BST_CHECKED)
				m_mode = 0;
			else if(SendDlgItemMessage(m_hWnd, IDC_RADIO2, BM_GETCHECK, 0, 0) == BST_CHECKED)
				m_mode = 1;
			else if(SendDlgItemMessage(m_hWnd, IDC_RADIO3, BM_GETCHECK, 0, 0) == BST_CHECKED)
				m_mode = 2;
			else if(SendDlgItemMessage(m_hWnd, IDC_RADIO4, BM_GETCHECK, 0, 0) == BST_CHECKED)
				m_mode = 3;
			if(m_mode >= 0)
			{
				g_player.SendInstrumentSetting(2, m_mode);
				if(idCtl == IDC_STORE)
				{
					g_player.SendInstrumentSetting(127, 127);
				}
			}
		}
		SaveConfig();
		EndDialog(m_hWnd, IDOK);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, IDCANCEL);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgMidiThru::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	Update();
}

void CDlgMidiThru::Update()
{
	if(m_mode >= 0)
		SendDlgItemMessage(m_hWnd, IDC_RADIO1 + m_mode, BM_SETCHECK, BST_CHECKED, 0);
}

CDlgPressure::CDlgPressure():CDlgBase((LPCTSTR)IDD_IS_CP, (LPCTSTR)IDD_IS_CP_LS)
{
	CPsPortRS232 *port = g_player.GetRS232();
	port->Reset();
	port->UpdateDecapNRPNSetting();
	int l = port->GetDecapNRPNSetting(3);
	int h = port->GetDecapNRPNSetting(4);
	if(l >= 0 && h >= 0)
		m_maxPressure = (h << 7) | l;
	else
		m_maxPressure = 0;
	l = port->GetDecapNRPNSetting(5);
	h = port->GetDecapNRPNSetting(6);
	if(l >= 0 && h >= 0)
		m_minPressure = (h << 7) | l;
	else
		m_minPressure = 0;
}

CDlgPressure::~CDlgPressure()
{

}

LRESULT CDlgPressure::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDC_STORE:
	case IDOK:
		{
			BOOL bTranslated = FALSE;
			unsigned int pressure = GetDlgItemInt(m_hWnd, IDC_EDIT1, &bTranslated, FALSE);
			if(bTranslated)
			{
				g_player.SendInstrumentSetting(3, pressure & 0x7f);
				if(idCtl == IDC_STORE)
				{
					g_player.SendInstrumentSetting(127, 127);
				}
				g_player.SendInstrumentSetting(4, (pressure >> 7) & 0x7f);
				if(idCtl == IDC_STORE)
				{
					g_player.SendInstrumentSetting(127, 127);
				}
			}
			pressure = GetDlgItemInt(m_hWnd, IDC_EDIT2, &bTranslated, FALSE);
			if(bTranslated)
			{
				g_player.SendInstrumentSetting(5, pressure & 0x7f);
				if(idCtl == IDC_STORE)
				{
					g_player.SendInstrumentSetting(127, 127);
				}
				g_player.SendInstrumentSetting(6, (pressure >> 7) & 0x7f);
				if(idCtl == IDC_STORE)
				{
					g_player.SendInstrumentSetting(127, 127);
				}
			}
		}
		SaveConfig();
		EndDialog(m_hWnd, IDOK);
		SHSipPreference(m_hWnd, SIP_DOWN);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, IDCANCEL);
		SHSipPreference(m_hWnd, SIP_DOWN);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgPressure::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	Update();
	SHSipPreference(m_hWnd, SIP_UP);
}

void CDlgPressure::Update()
{
	SetDlgItemInt(m_hWnd, IDC_EDIT1, m_maxPressure, FALSE);
	SetDlgItemInt(m_hWnd, IDC_EDIT2, m_minPressure, FALSE);
}

CDlgChnDelay::CDlgChnDelay():CDlgBase((LPCTSTR)IDD_CHAN_DELAY, (LPCTSTR)IDD_CHAN_DELAY_LS)
{
}

CDlgChnDelay::~CDlgChnDelay()
{

}

LRESULT CDlgChnDelay::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		{
			short delayTable[16];
			int i;
			for(i = 0; i < 16; i++)
			{
				delayTable[i] = -(int)GetDlgItemInt(m_hWnd, i + IDC_EDIT1, NULL, TRUE);
			}
			g_player.SetChannelDelayTable(delayTable);
		}
		SaveConfig();
		EndDialog(m_hWnd, IDOK);
		SHSipPreference(m_hWnd, SIP_DOWN);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, IDCANCEL);
		SHSipPreference(m_hWnd, SIP_DOWN);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgChnDelay::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	SetDlgItemText(m_hWnd, IDC_DESC1, TEXT("Advances the events in time to compensate for acoustic-instrument delays.\nRange 0 to 500 milliseconds per channel."));
	Update();
	SHSipPreference(m_hWnd, SIP_UP);
}

void CDlgChnDelay::Update()
{
	short delayTable[16];
	g_player.GetChannelDelayTable(delayTable);
	int i;
	for(i = 0; i < 16; i++)
	{
		SetDlgItemInt(m_hWnd, i + IDC_EDIT1, -delayTable[i], TRUE);
	}
}

CDlgChnCompression::CDlgChnCompression(int port):CDlgBase((LPCTSTR)IDD_CHAN_DELAY, (LPCTSTR)IDD_CHAN_DELAY_LS)
,m_port(port)
{
}

CDlgChnCompression::~CDlgChnCompression()
{
	
}

LRESULT CDlgChnCompression::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		{
			char *pTable = g_player.GetChannelCompressionTable(m_port);
			int i;
			for(i = 0; i < 16; i++)
			{
				int v = GetDlgItemInt(m_hWnd, i + IDC_EDIT1, NULL, FALSE);
				if(v < 0)
					v = 0;
				else if (v > 127)
					v = 127;
				pTable[i] = v;
			}
		}
		SaveConfig();
		EndDialog(m_hWnd, IDOK);
		SHSipPreference(m_hWnd, SIP_DOWN);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, IDCANCEL);
		SHSipPreference(m_hWnd, SIP_DOWN);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgChnCompression::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	SetWindowText(m_hWnd, TEXT("Volume Compression"));
	Update();
	SHSipPreference(m_hWnd, SIP_UP);
}

void CDlgChnCompression::Update()
{
	char *pTable = g_player.GetChannelCompressionTable(m_port);
	int i;
	for(i = 0; i < 16; i++)
	{
		SetDlgItemInt(m_hWnd, i + IDC_EDIT1, pTable[i], FALSE);
	}
}



CDlgPianoCH1Settings::CDlgPianoCH1Settings():CDlgBase((LPCTSTR)IDD_PIANO_CH1_SETTINGS, (LPCTSTR)IDD_PIANO_CH1_SETTINGS_LS)
{
}

CDlgPianoCH1Settings::~CDlgPianoCH1Settings()
{
	
}

LRESULT CDlgPianoCH1Settings::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		{
			BOOL isUseCompression;
			int ch1CompMinVelo, ch1MinThreshVol;
			isUseCompression = SendDlgItemMessage(m_hWnd, IDC_USE_COMPRESSION, BM_GETCHECK, 0, 0) == BST_CHECKED;
			ch1CompMinVelo = GetDlgItemInt(m_hWnd, IDC_MIN_VELOCITY, NULL, FALSE);
			if (ch1CompMinVelo < 1)
				ch1CompMinVelo = 1;
			if (ch1CompMinVelo > 127)
				ch1CompMinVelo = 127;
			ch1MinThreshVol = GetDlgItemInt(m_hWnd, IDC_MIN_THRESHOLD, NULL, FALSE);
			if (ch1MinThreshVol < 1)
				ch1MinThreshVol = 1;
			if (ch1MinThreshVol > 127)
				ch1MinThreshVol = 127;
			g_player.GetRS232()->SetPianoCH1Settings(isUseCompression, ch1CompMinVelo, ch1MinThreshVol);
		}
		SaveConfig();
		EndDialog(m_hWnd, IDOK);
		SHSipPreference(m_hWnd, SIP_DOWN);
		return TRUE;
	case IDCANCEL:
		EndDialog(m_hWnd, IDCANCEL);
		SHSipPreference(m_hWnd, SIP_DOWN);
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgPianoCH1Settings::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	SetWindowText(m_hWnd, TEXT("Piano CH1 Settings"));
	Update();
	SHSipPreference(m_hWnd, SIP_UP);
}

void CDlgPianoCH1Settings::Update()
{
	CPsPortRS232 *port = g_player.GetRS232();

	BOOL isUseCompression;
	int ch1CompMinVelo, ch1MinThreshVol;
	port->GetPianoCH1Settings(isUseCompression, ch1CompMinVelo, ch1MinThreshVol);
	SendDlgItemMessage(m_hWnd, IDC_USE_COMPRESSION, BM_SETCHECK, isUseCompression ? BST_CHECKED : BST_UNCHECKED, 0);
	SetDlgItemInt(m_hWnd, IDC_MIN_VELOCITY, ch1CompMinVelo, FALSE);
	SetDlgItemInt(m_hWnd, IDC_MIN_THRESHOLD, ch1MinThreshVol, FALSE);
}
