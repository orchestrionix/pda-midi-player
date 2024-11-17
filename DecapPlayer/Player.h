// Player.h: interface for the CPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYER_H__B5169BFB_CC17_4144_AB1D_FA20046F9521__INCLUDED_)
#define AFX_PLAYER_H__B5169BFB_CC17_4144_AB1D_FA20046F9521__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PlayList.h"
#include "common/PsObject.h"

class CPsPortRS232;

#define PLAYER_REG_KEY TEXT("Software\\DECAP\\MidiPlayer")

bool SaveConfig();

class CPlayer  
{
public:
	enum {
		STATUS_UNINITED,
		STATUS_READY,
		STATUS_PLAYING,
		STATUS_STOPPING,
		STATUS_PAUSED
	};
	enum {
		CMD_NONE = 0,
		CMD_OPEN,
		CMD_OPEN_AND_PLAY,
	};
	enum {
		NUM_CHNV_BUF = 7,
		NUM_OUTPUT_PORT = 2
	};

	void SendInstrumentSetting(int lsb, int msb);
	void AllControllerOff();
	void StopAndPlay(int id);
	void SynchronizeCOMVolume();
	int GetTrueFrequency();
	int GetCOMDelay();
	void SetCOMDelay(int delay);
	int GetBassCutoff();
	void SetBassCutoff(int freq);
	F16 GetBassVolume();
	void SetBassVolume(F16 vol);
	void SetMixVolume(F16 vol);
	F16 GetMixVolume();
	int GetOutputPortNumber();
	int GetOutputPortsScheme(int *pIndex);
	void SetOutputPortsScheme(int *pIndex, int num);
	void SetSolo(int channel);
	void SetProgram(int channel, int prog);
	int GetProgram(int channel);
	void SetMute(int channel, bool isMute);
	bool IsMute(int channel);
	void SetTempo(int i);
	void SetKeyTranspose(int i);
	void Destroy();
	int SetPanSep(int ps);
	int GetPanSep();
	int SetReverb(int revb);
	int GetReverb();
	void ShowTitle();
	int GetMaxPostion();
	int SetPosition(int pos);
	int GetPosition();
	int SetVolume(int vol);
	int GetVolume();
	bool GetReverse();
	bool SetReverse(bool enabled);
	bool GetSurround();
	bool SetSurround(bool enabled);
	void HandleEmptyList();
	void Forward();
	void Backward();
	void Prev();
	void Next(bool forcePlay);
	void Stop();
	bool Open(int id);
	bool Init();
	bool Pause();
	int GetStatus();
	bool Update();
	bool Play();
	void SetMaxPoly(int i);
	int GetMaxPoly();
	void SetMp3Volume(F16 vol);
	F16 GetMp3Volume();

	CPlayer();
	virtual ~CPlayer();

	void SetShuffle(bool shuffle)
	{
		g_playlist.SetShuffle(shuffle);
	}

	bool GetShuffle()
	{
		return g_playlist.GetShuffle();
	}

	void SetRepeat(bool repeat)
	{
		m_bLoop = repeat;
	}

	void SetSingle(bool isSignle)
	{
		m_isSingle = isSignle;
	}

	bool GetSingle(){
		return m_isSingle;
	}


	void SetAutoAdvance(bool isAutoAdv)
	{
		m_isAutoAdvance = isAutoAdv;
	}

	bool GetAutoAdvance(){
		return m_isAutoAdvance;
	}

	bool GetRepeat(){
		return m_bLoop;
	}
	int GetTimeMode(){
		return m_timeMode;
	}
	void SetTimeMode(int mode){
		m_timeMode = mode;
	}

	CPsOutputPort** GetStockedOutputPorts(int *pNum){
		*pNum = NUM_OUTPUT_PORT;
		return m_pOutputPorts;
	}

	void SetSoftSynthVolThreshold(int nPercent) {
		int v = GetVolume();
		if(nPercent > 100)
			nPercent = 100;
		else if(nPercent < 0)
			nPercent = 0;
		m_softSynthVolThreshold = nPercent;
		SetVolume(v);
	}

	int GetSoftSynthVolThreshold() {
		return m_softSynthVolThreshold;
	}

	void SetCloseAppOnActive(bool isAutoClose) {
		m_isCloseOtherAppOnActive = isAutoClose;
	}

	bool GetCloseAppOnActive() const {
		return m_isCloseOtherAppOnActive;
	}

	bool GetEnableDecapCommand() const { return m_enableDecapCommand; }
	void SetEnableDecapCommand(bool isEnabled) { m_enableDecapCommand = isEnabled; }
	
	void SetChannelDelayTable(short* delayTable);
	void GetChannelDelayTable(short* delayTable);
	char* GetChannelCompressionTable(int port);

	void SelectPlaylistItem(int id);
	int GetAccordionMainMidChan();
	void SetAccordionMainMidChan(int chn);
	int GetAccordionSubMidChan();
	void SetAccordionSubMidChan(int chn);
	int GetActiveSensing();
	void SetActiveSensing(int isEnabled);
	void SetSongDelay(int nDelay);
	int GetSongDelay() const;

	CPsPortRS232* GetRS232();

	bool m_setposbar;
protected:
	void decrypt(unsigned char *pBuf, int len, int key);
	int calcKey(int userId, int fileLen);
	bool DecryptMIDI(DWORD &len);
	void DrawGraph();
	void DrawTime();
	bool Open(LPCTSTR fn);
	void FreeModule();
	bool m_initDone;
	bool m_bLoop;
	bool m_isSingle;
	bool m_isAutoAdvance;
	bool m_enableDecapCommand;
	int m_timeMode;
	int m_prevStatus;
	int m_status;
	int m_idPlaying;
	int m_songLength;
	TCHAR m_title[256];
	TCHAR mp3fn[MAX_PATH];
	int m_tick;
	int m_stoptick;
	void* m_pMediaData;
	int m_maxPoly;

	int m_totalMixTime;
	int m_totalMixChannel;

	short m_chnVBuf[NUM_CHNV_BUF][16];
	int m_curChnVBuf;
	int m_cmdAfterStop;
	int m_softSynthVolThreshold;
	DWORD m_hwVolume;
	int m_lastRs232Received;

	CPsOutputPort *m_pOutputPorts[NUM_OUTPUT_PORT];
	short m_channelDelays[16];

	bool m_isStandaloneMp3;

	bool m_isCloseOtherAppOnActive;
};

extern CPlayer g_player;

#endif // !defined(AFX_PLAYER_H__B5169BFB_CC17_4144_AB1D_FA20046F9521__INCLUDED_)
