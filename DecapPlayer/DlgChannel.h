#ifndef __DLGCHANNEL_H__
#define __DLGCHANNEL_H__

#include "DlgBase.h"

class CDlgProg : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgProg(int sel);
	virtual ~CDlgProg();
protected:
	int m_sel;
};

class CDlgChannel : public CDlgBase  
{
public:
	void Update();
	void OnInitDialog();
	LRESULT OnCommand(int idCtl, int idNotify);
	CDlgChannel();
	virtual ~CDlgChannel();
protected:
};

#endif
