#ifndef __PSPORTRS232_H__
#define __PSPORTRS232_H__

#include "common/PsObject.h"

//#define USE_HIGH_RES_TIMER
//#define ENABLE_MONITOR

class CMonitor {
public:
	CMonitor() {
		memset(m_noteState, 0, sizeof(m_noteState));
		m_numToFill = 1;
		m_numFilled = 0;
	}
	void Process(unsigned char *p, int len);
	bool ProcessOneEvent();
	int GetActiveNotes();

	unsigned char m_noteState[16][128];
	unsigned char m_buffer[20];
	int m_numToFill;
	int m_numFilled;
};

class CRingBuffer {
public:
	enum
	{
		BUFFER_SIZE = 0x2000,
		BUFFER_SIZE_MASK = BUFFER_SIZE - 1
	};
	CRingBuffer() {
		Reset();
	}

	void Reset() {
		m_idxWrite = m_idxRead = 0;
	}
	void Append(unsigned char c) {
		int nextWrite = (m_idxWrite + 1) & BUFFER_SIZE_MASK;
		//If buffer is full, overwirte oldest one and move m_idxRead to next one
		if(nextWrite == m_idxRead)
			++m_idxRead;
		m_buffer[m_idxWrite] = c;
		m_idxWrite = nextWrite;
	}

	int GetFirst() {
		int c = m_buffer[m_idxRead];
		m_idxRead = (m_idxRead + 1) & BUFFER_SIZE_MASK;
		return c;
	}

	int Peek(int id = 0) {
		ASSERT(GetLength() > id);
		return m_buffer[(m_idxRead + id) & BUFFER_SIZE_MASK];
	}

	void Skip(int nSkip) {
		ASSERT(GetLength() >= nSkip);
		m_idxRead = (m_idxRead + nSkip) & BUFFER_SIZE_MASK;
	}
	int GetLength() {
		int len = m_idxWrite - m_idxRead;
		if(len < 0)
			len += BUFFER_SIZE;
		return len;
	}
	int m_idxWrite, m_idxRead;
	unsigned char m_buffer[BUFFER_SIZE];
};

class CPsPortRS232 : public CPsOutputPort
{
public:
	enum
	{
		STATE_CLOSED,
		STATE_CLOSING,
		STATE_OPENED,
		
		BUFFER_SIZE = 0x4000,
		BUFFER_MASK = BUFFER_SIZE - 1,

		WRITE_CACHE_SIZE = 0x2000
	};
#ifdef DECAP_PLAYER
	enum
	{
		DECAP_CMD_NONE = 0,
		DECAP_CMD_PAUSE,
		DECAP_CMD_PREV,
		DECAP_CMD_NEXT,
		DECAP_CMD_PLAY,
		DECAP_CMD_STOP,
		DECAP_CMD_SELECT_SONG,
		DECAP_CMD_SELECT_LIST,
		DECAP_CMD_SET_VOLUME
	};
	unsigned int GetDecapCommand();
	int GetTotalReceived(){	return m_totalReceived;}

	int GetAccordionMainMidChan() const {return m_accordionMainMidChan;}
	void SetAccordionMainMidChan(int chn) {m_accordionMainMidChan = chn;SendChannelSetting(chn, 1);}
	int GetAccordionSubMidChan() const {return m_accordionSubMidChan;}
	void SetAccordionSubMidChan(int chn) {m_accordionSubMidChan = chn;SendChannelSetting(chn, 2);}
	int GetActiveSensing() const {return m_activeSensing;}
	void SetActiveSensing(int isEnable) {m_activeSensing = isEnable;SendChannelSetting(isEnable, 3);}
	void UpdateDecapNRPNSetting();
	int GetDecapNRPNSetting(int id) {return m_decapControlData[id];}
	void SendStatus(unsigned char msg);
	void SendChannelSetting(unsigned char chn, unsigned char id);
	bool IsPowerOffOccured() const { return m_isPowerOffOccured; }
	void ClearPowerOffOccured() { m_isPowerOffOccured = false; }
	void SetPianoCH1Settings(BOOL isUseCompression, int ch1CompMinVelo, int ch1MinThreshVol)
	{
		m_ch1IsUseCompression = isUseCompression;
		m_ch1CompMinVelo = ch1CompMinVelo;
		m_ch1MinThreshVol = ch1MinThreshVol;
	}
	void GetPianoCH1Settings(BOOL &isUseCompression, int &ch1CompMinVelo, int &ch1MinThreshVol)
	{
		isUseCompression = m_ch1IsUseCompression;
		ch1CompMinVelo = m_ch1CompMinVelo;
		ch1MinThreshVol = m_ch1MinThreshVol;
	}
	int CalcPianoCH1Velocity(int v0);

#endif
	CPsPortRS232();
	bool Open(CPsAudioSystem *pSystem);
	bool ReOpen();
	void Close();
	virtual const char* GetName();
	virtual void Reset();
	virtual void AllNoteOff();
	virtual void ReduceVoice();
	virtual int GetActiveCount();
	virtual void GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList);
	virtual void Send(unsigned int framePosition, void *pEvent);
	void SetMasterVolume(int val);
	void SendInstrumentSetting(int msb, int lsb);

	void SetDelay(int delay){
		m_delay = delay;
	}

	int GetDelay(){
		return m_delay;
	}

	bool IsOpened(){
		return m_hOutPort != INVALID_HANDLE_VALUE;
	}

	int GetOutPortId() {
		return m_outPortId;
	};
	int GetInPortId() {
		return m_inPortId;
	}
	DWORD GetBaudRate() {
		return m_baudRate;
	}

	void SetOutPortId(int id) {
		m_outPortId = id;
	}
	void SetInPortId(int id) {
		m_inPortId = id;
	}

	void SetBaudRate(DWORD baudRate) {
		m_baudRate = baudRate;
	}

#ifdef ENABLE_MONITOR
	CMonitor m_monitor;
#endif

//protected:
	void AppendData(int framePosition, const unsigned char* pRawData, int nRawData, bool isInsert = false);
	static DWORD WINAPI WriteThread(LPVOID lpParameter);
	static DWORD WINAPI ReadThread(LPVOID lpParameter);
	static DWORD WINAPI TestSuspendThread(LPVOID lpParameter);
	DWORD RunWrite();
	DWORD RunRead();
	HANDLE OpenComPort(int id, DWORD baudRate);

	CPsAudioSystem *m_pSystem;
	unsigned char m_buffer[BUFFER_SIZE];
	unsigned int m_readIndex, m_writeIndex;
	unsigned char m_writeCache[WRITE_CACHE_SIZE];
	CRingBuffer m_readBuffer;

	unsigned char m_noteState[16][128];
	int m_state;
	int m_outPortId, m_inPortId;
	HANDLE m_hOutPort;
	HANDLE m_hInPort;
	DWORD m_baudRate;
	HANDLE m_writeThread;
	HANDLE m_readThread;
	unsigned int m_writeTick;
	int m_delay;
	unsigned char m_accordionMainMidChan, m_accordionSubMidChan;
	unsigned char m_activeSensing;
	unsigned char m_lastStatusByte;
	bool m_lastAppendIsInsert;

#ifdef USE_HIGH_RES_TIMER
	MMRESULT m_hTimer;
	HANDLE m_hEvent;
#endif

#ifdef DECAP_PLAYER
	bool m_isPowerOffOccured;
	unsigned int m_lastDecapCmd;
	int m_totalReceived;
	signed char m_decapControlData[128];

	//for Real Piano CH1 Settings->Use Compression
	int m_ch1Ctrl07, m_ch1Ctrl11, m_ch1Ctrl89;
	BOOL m_ch1IsUseCompression;
	int m_ch1CompMinVelo, m_ch1MinThreshVol;
#endif
};



#endif
