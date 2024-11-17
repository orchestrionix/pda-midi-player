#include "stdafx.h"
#include "resource.h"
#include "DlgChannel.h"
#include "Player.h"
#include "utils.h"

static const TCHAR* const progName[] = {
	TEXT("Acoustic Grand Piano"),
	TEXT("Bright Acoustic Piano"),
	TEXT("Electric Grand Piano"),
	TEXT("Honky-tonk Piano"),
	TEXT("Rhodes Piano"),
	TEXT("Chorused Piano"),
	TEXT("Harpsichord"),
	TEXT("Clavichord"),

	TEXT("Celesta"),
	TEXT("Glockenspiel"),
	TEXT("Music box"),
	TEXT("Vibraphone"),
	TEXT("Marimba"),
	TEXT("Xylophone"),
	TEXT("Tubular Bells"),
	TEXT("Dulcimer"),

	TEXT("Hammond Organ"),
	TEXT("Percussive Organ"),
	TEXT("Rock Organ"),
	TEXT("Church Organ"),
	TEXT("Reed Organ"),
	TEXT("Accordian"),
	TEXT("Harmonica"),
	TEXT("Tango Accordian"),

	TEXT("Acoustic Guitar (nylon)"),
	TEXT("Acoustic Guitar (steel)"),
	TEXT("Electric Guitar (jazz) "),
	TEXT("Electric Guitar (clean)"),
	TEXT("Electric Guitar (muted)"),
	TEXT("Overdriven Guitar"),
	TEXT("Distortion Guitar"),
	TEXT("Guitar Harmonics"),

	TEXT("Acoustic Bass"),
	TEXT("Electric Bass(finger)"),
	TEXT("Electric Bass (pick)"),
	TEXT("Fretless Bass"),
	TEXT("Slap Bass 1"),
	TEXT("Slap Bass 2"),
	TEXT("Synth Bass 1"),
	TEXT("Synth Bass 2"),
	
	TEXT("Violin"),
	TEXT("Viola"),
	TEXT("Cello"),
	TEXT("Contrabass"),
	TEXT("Tremolo Strings"),
	TEXT("Pizzicato Strings"),
	TEXT("Orchestral Harp"),
	TEXT("Timpani"),
	
	TEXT("String Ensemble 1"),
	TEXT("String Ensemble 2"),
	TEXT("Synth Strings 1"),
	TEXT("Synth Strings 2"),
	TEXT("Choir Aahs"),
	TEXT("Voice Oohs"),
	TEXT("Synth Voice"),
	TEXT("Orchestra Hit"),
	
	TEXT("Trumpet"),
	TEXT("Trombone"),
	TEXT("Tuba"),
	TEXT("Muted Trumpet"),
	TEXT("French Horn"),
	TEXT("Brass Section"),
	TEXT("Synth Brass 1"),
	TEXT("Synth Brass 2"),

	TEXT("Soprano Sax"),
	TEXT("Alto Sax"),
	TEXT("Tenor Sax"),
	TEXT("Baritone Sax"),
	TEXT("Oboe"),
	TEXT("English Horn"),
	TEXT("Bassoon"),
	TEXT("Clarinet"),

	TEXT("Piccolo"),
	TEXT("Flute"),
	TEXT("Recorder"),
	TEXT("Pan Flute"),
	TEXT("Bottle Blow"),
	TEXT("Shakuhachi"),
	TEXT("Whistle"),
	TEXT("Ocarina"),
	
	TEXT("Lead 1 (square)"),
	TEXT("Lead 2 (sawtooth)"),
	TEXT("Lead 3 (calliope)"),
	TEXT("Lead 4 (chiff)"),
	TEXT("Lead 5 (charang)"),
	TEXT("Lead 6 (voice)"),
	TEXT("Lead 7 (fifths)"),
	TEXT("Lead 8 (bass+lead)"),
	
	TEXT("Pad 1 (new age)"),
	TEXT("Pad 2 (warm)"),
	TEXT("Pad 3 (polysynth)"),
	TEXT("Pad 4 (choir)"),
	TEXT("Pad 5 (bowed)"),
	TEXT("Pad 6 (metallic)"),
	TEXT("Pad 7 (halo)"),
	TEXT("Pad 8 (sweep)"),
	
	TEXT("FX 1 (rain)"),
	TEXT("FX 2 (soundtrack)"),
	TEXT("FX 3 (crystal)"),
	TEXT("FX 4 (atmosphere)"),
	TEXT("FX 5 (brightness)"),
	TEXT("FX 6 (goblins)"),
	TEXT("FX 7 (echoes)"),
	TEXT("FX 8 (sci-fi)"),
	
	TEXT("Sitar"),
	TEXT("Banjo"),
	TEXT("Shamisen"),
	TEXT("Koto"),
	TEXT("Kalimba"),
	TEXT("Bagpipe"),
	TEXT("Fiddle"),
	TEXT("Shanai"),
	
	TEXT("Tinkle Bell"),
	TEXT("Agogo"),
	TEXT("Steel Drums"),
	TEXT("Woodblock"),
	TEXT("Taiko Drum"),
	TEXT("Melodic Tom"),
	TEXT("Synth Drum"),
	TEXT("Reverse Cymbal"),
	
	TEXT("Guitar Fret Noise"),
	TEXT("Breath Noise"),
	TEXT("Seashore"),
	TEXT("Bird Tweet"),
	TEXT("Telephone Ring"),
	TEXT("Helicopter"),
	TEXT("Applause"),
	TEXT("Gunshot"),
};

CDlgProg::CDlgProg(int sel):m_sel(sel), CDlgBase((LPCTSTR)IDD_SEL_PROG, (LPCTSTR)IDD_SEL_PROG_LS)
{
}

CDlgProg::~CDlgProg()
{

}

LRESULT CDlgProg::OnCommand(int idCtl, int idNotify)
{
	switch(idCtl)
	{
	case IDOK:
		EndDialog(m_hWnd, SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_GETCURSEL, 0, 0));
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgProg::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	Update();
}

void CDlgProg::Update()
{
	int i;
	for(i = 0; i < sizeof(progName) / sizeof(progName[0]); i++)
	{
		SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_ADDSTRING, 0, (LPARAM)progName[i]);
	}
	SendDlgItemMessage(m_hWnd, IDC_PLAYLIST, LB_SETCURSEL, m_sel, 0);
}

CDlgChannel::CDlgChannel():CDlgBase((LPCTSTR)IDD_CHANNEL, (LPCTSTR)IDD_CHANNEL_LS)
{
}

CDlgChannel::~CDlgChannel()
{

}

LRESULT CDlgChannel::OnCommand(int idCtl, int idNotify)
{
	if(idCtl >= IDC_CHECK1 && idCtl < IDC_CHECK1 + 16)
	{
		if(idNotify == BN_CLICKED)
		{
			int id = idCtl - IDC_CHECK1;
			g_player.SetMute(id, !g_player.IsMute(id));
			SendDlgItemMessage(m_hWnd, idCtl, BM_SETCHECK, g_player.IsMute(id) ? BST_UNCHECKED : BST_CHECKED, 0);
		}
		return TRUE;
	}
	else if(idCtl >= IDC_BUTTON17 && idCtl < IDC_BUTTON17 + 16)
	{
		g_player.SetSolo(idCtl - IDC_BUTTON17);
		Update();
		return TRUE;
	}
	else if(idCtl >= IDC_BUTTON1 && idCtl < IDC_BUTTON1 + 16)
	{
		CDlgProg dlg(g_player.GetProgram(idCtl - IDC_BUTTON1));
		g_player.SetProgram(idCtl - IDC_BUTTON1, dlg.ShowModal(m_hWnd));
		Update();
		return TRUE;
	}
	switch(idCtl)
	{
	case IDOK:
		EndDialog(m_hWnd, idCtl);
		return TRUE;
	case IDC_BUTTON33:
		{
			int i;
			for(i = 0; i < 16; i++)
				g_player.SetMute(i, false);
			Update();
		}
		return TRUE;
	}
	return CDlgBase::OnCommand(idCtl, idNotify);
}

void CDlgChannel::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	Update();
}

void CDlgChannel::Update()
{
	int i;
	for(i = 0; i < 16; i++)
	{
		SetDlgItemInt(m_hWnd, IDC_BUTTON1 + i, g_player.GetProgram(i), FALSE);
		SendDlgItemMessage(m_hWnd, IDC_CHECK1 + i, BM_SETCHECK, g_player.IsMute(i) ? BST_UNCHECKED : BST_CHECKED, 0);
	}
}
