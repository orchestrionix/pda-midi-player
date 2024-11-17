#ifndef __DLGOPTION_H__
#define __DLGOPTION_H__

#include "DlgBase.h"

class CDlgOption : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgOption();
	virtual ~CDlgOption();
protected:
};

class CDlgComPort : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgComPort();
	virtual ~CDlgComPort();
protected:
};

class CDlgVolumes : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgVolumes();
	virtual ~CDlgVolumes();
protected:
};

#endif
