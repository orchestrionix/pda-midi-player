#ifndef __DLGDECAPSETTING_H__
#define __DLGDECAPSETTING_H__

#include "DlgBase.h"

class CDlgMasterVolume : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgMasterVolume();
	virtual ~CDlgMasterVolume();
protected:
	int m_volume;
};

class CDlgMidiChannel : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgMidiChannel();
	virtual ~CDlgMidiChannel();
protected:
	int m_mainChannel;
	int m_subChannel;
	int m_activeSensing;
};

class CDlgMidiThru : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgMidiThru();
	virtual ~CDlgMidiThru();
protected:
	int m_mode;
};

class CDlgPressure : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgPressure();
	virtual ~CDlgPressure();
protected:
	int m_minPressure, m_maxPressure;
};

class CDlgChnDelay : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgChnDelay();
	virtual ~CDlgChnDelay();
protected:
};

class CDlgChnCompression : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgChnCompression(int port);
	virtual ~CDlgChnCompression();
protected:
	int m_port;
};

class CDlgPianoCH1Settings : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgPianoCH1Settings();
	virtual ~CDlgPianoCH1Settings();
protected:
};

#endif
