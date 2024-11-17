#include "common/PsAudioSystem.h"
#include "PsPortRS232.h"
#include "common/PsTrack.h"
#include "player.h"
#include "PsSys.h"
#include "resource.h"

//#define DEBUG_LOG
#define SKIP_SETTING 0
#ifdef DEBUG_LOG
FILE *flog;
#endif

void CMonitor::Process(unsigned char *p, int len)
{
	while(len > 0)
	{
		while(m_numFilled < m_numToFill && len > 0)
		{
			m_buffer[m_numFilled++] = *p++;
			len--;
		}
		if(m_numFilled == m_numToFill)
		{
			if(ProcessOneEvent())
			{
				m_numFilled = 0;
				m_numToFill = 1;
			}
		}
	}
}

bool CMonitor::ProcessOneEvent()
{
	switch(m_buffer[0] & 0xf0)
	{
	case 0x80:
		if(m_numFilled < 3)
		{
			m_numToFill = 3;
			return false;
		}
		if(m_noteState[m_buffer[0] & 0xf][m_buffer[1]] > 0)
			m_noteState[m_buffer[0] & 0xf][m_buffer[1]]--;
		return true;
	case 0x90:
		if(m_numFilled < 3)
		{
			m_numToFill = 3;
			return false;
		}
		if(m_buffer[2] == 0)
		{
			if(m_noteState[m_buffer[0] & 0xf][m_buffer[1]] > 0)
				m_noteState[m_buffer[0] & 0xf][m_buffer[1]]--;
		}
		else
			m_noteState[m_buffer[0] & 0xf][m_buffer[1]]++;
		return true;
	}
	return true;
}

int CMonitor::GetActiveNotes()
{
	int nActive = 0;
	for(int i = 0; i < 16; i++)
	{
		for(int j = 0; j < 127; j++)
		{
			if(m_noteState[i][j] > 1)
				nActive++;
		}
	}
	return nActive;
}

CPsPortRS232::CPsPortRS232():
	m_pSystem(NULL),
	m_hOutPort(INVALID_HANDLE_VALUE),
	m_hInPort(INVALID_HANDLE_VALUE),
	m_writeThread(NULL),
	m_readThread(NULL),
	m_state(STATE_CLOSED),
	m_delay(50)
#ifdef USE_HIGH_RES_TIMER
	,m_hTimer(NULL),
	m_hEvent(NULL)
#endif
{
	m_accordionSubMidChan = m_accordionMainMidChan = -1;
	m_outPortId = 1;
	m_inPortId = 1;
	m_baudRate = CBR_38400;
#ifdef DECAP_PLAYER
	m_ch1IsUseCompression = FALSE;
	m_ch1CompMinVelo = m_ch1MinThreshVol = 1;
	m_ch1Ctrl89 = 127;
#endif

	HKEY hkey;
	DWORD val, s;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, PLAYER_REG_KEY, 0, 0, &hkey) == ERROR_SUCCESS)
	{
		s = sizeof(val);
		if(RegQueryValueEx(hkey, TEXT("COMBaudRate"), NULL, NULL, (LPBYTE)&val, &s) == ERROR_SUCCESS)
		{
			m_baudRate = val;
		}
		RegCloseKey(hkey);
	}
}

bool CPsPortRS232::Open(CPsAudioSystem *pSystem)
{
#ifdef DEBUG_LOG
	flog = fopen("\\mp.log", "a+");
	SYSTEMTIME time;
	GetLocalTime(&time);
	fprintf(flog, "start log: %d-%d-%d %02d:%02d:%02d\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	fprintf(flog, "open port\n");
#endif
	
	m_pSystem = pSystem;
	m_readIndex = m_writeIndex = 0;
	CPsSys::memset(m_noteState, 0, sizeof(m_noteState));
#ifdef DECAP_PLAYER
	m_totalReceived = 0;
	m_isPowerOffOccured = false;
	m_lastAppendIsInsert = false;
	m_lastDecapCmd = DECAP_CMD_NONE;
	memset(m_decapControlData, -1, sizeof(m_decapControlData));
#endif
	m_readBuffer.Reset();
	m_state = STATE_OPENED;
	
	if(ReOpen())
	{
		HANDLE hThread = CreateThread(0, 0, TestSuspendThread, this, 0, NULL);
		SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
		CloseHandle(hThread);
		return true;
	}
	SendStatus(0xFF);
#ifdef DEBUG_LOG
	fclose(flog);
#endif
	return false;
}

HANDLE CPsPortRS232::OpenComPort(int id, DWORD baudRate)
{
	HANDLE h;

	TCHAR s[64];
	wsprintf(s, TEXT("COM%d:"), id);
	h = CreateFile(s, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(h != INVALID_HANDLE_VALUE)
	{
		DCB c;
		COMMTIMEOUTS timeout;
		c.DCBlength = sizeof(c);
		if(SKIP_SETTING || (GetCommState(h, &c) && GetCommTimeouts(h, &timeout)))
		{
			m_baudRate = baudRate;
			c.BaudRate = m_baudRate;
			c.ByteSize = 8;
			c.fParity = FALSE;
			c.Parity = NOPARITY;
			c.StopBits = ONESTOPBIT;
			timeout.ReadIntervalTimeout = 1;
			timeout.ReadTotalTimeoutConstant = 1;
			timeout.ReadTotalTimeoutMultiplier = 1;
			timeout.WriteTotalTimeoutConstant = 50;
			timeout.WriteTotalTimeoutMultiplier = 1;
			if(SKIP_SETTING || SetCommState(h, &c) /*&& SetCommTimeouts(h, &timeout)*/)
			{
#ifdef DEBUG_LOG
				fprintf(flog, "OpenComPort success\n");
#endif
				return h;
			}
		}
		CloseHandle(h);
		h = INVALID_HANDLE_VALUE;
	}
	return h;
}

bool CPsPortRS232::ReOpen()
{
	if(m_writeThread)
	{
		TerminateThread(m_writeThread, 0);
		CloseHandle(m_writeThread);
		m_writeThread = NULL;
	}
	if(m_readThread)
	{
		TerminateThread(m_readThread, 0);
		CloseHandle(m_readThread);
		m_readThread = NULL;
	}
	if(m_hOutPort != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hOutPort);
		if(m_hInPort == m_hOutPort)
			m_hInPort = INVALID_HANDLE_VALUE;
		m_hOutPort = INVALID_HANDLE_VALUE;
	}
	if(m_hInPort != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hInPort);
		m_hInPort = INVALID_HANDLE_VALUE;
	}
	m_hOutPort = OpenComPort(m_outPortId, m_baudRate);
	if(m_hOutPort == INVALID_HANDLE_VALUE)
		return false;
	if(m_inPortId == m_outPortId)
	{
		m_hInPort = m_hOutPort;
	}
	else
		m_hInPort = OpenComPort(m_inPortId, m_baudRate);
	if(m_hInPort == INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hOutPort);
		m_hOutPort = INVALID_HANDLE_VALUE;
		return false;
	}

	m_writeThread = CreateThread(0, 0, WriteThread, this, 0, NULL);
	SetThreadPriority(m_writeThread, THREAD_PRIORITY_HIGHEST);
	m_readThread = CreateThread(0, 0, ReadThread, this, 0, NULL);
	return true;
}

void CPsPortRS232::Close()
{
	if(m_hOutPort != INVALID_HANDLE_VALUE)
	{
		Reset();
		m_state = STATE_CLOSING;
		int waitCount = 0;
		while(waitCount < 10 && m_state != STATE_CLOSED)
		{
			++waitCount;
			Sleep(50);
		}
		if(m_state != STATE_CLOSED)
		{
			TerminateThread(m_writeThread, 0);
			TerminateThread(m_readThread, 0);
			m_state = STATE_CLOSED;
		}
		CloseHandle(m_readThread);
		CloseHandle(m_writeThread);

		CloseHandle(m_hOutPort);
		if(m_hInPort == m_hOutPort)
			m_hInPort = INVALID_HANDLE_VALUE;
		m_hOutPort = INVALID_HANDLE_VALUE;
		m_pSystem = NULL;
	}
#ifdef DEBUG_LOG
	fclose(flog);
#endif
}

void CPsPortRS232::AppendData(int framePosition, const unsigned char* pRawData, int nRawData, bool isInsert)
{
	int nWrite = nRawData;
	if(!isInsert)
	{
		//in case a message is inserted to a running status, save the last status byte
		if(pRawData[0] > 0x80 && pRawData[0] < 0xF8)
			m_lastStatusByte = pRawData[0];
	}
	bool isLastMsgInsertedToRS = !isInsert && m_lastAppendIsInsert && (pRawData[0] < 0x80);
	if(isLastMsgInsertedToRS)
	{
		++nWrite;
	}
	unsigned int start = m_writeIndex & BUFFER_MASK;
	unsigned int end = m_writeIndex + 8 + nWrite;
	if(end - m_readIndex > BUFFER_SIZE)
		return;
	if((end & BUFFER_MASK) <= start)
	{
		for(int i = start; i < BUFFER_SIZE; i++)
			m_buffer[i] = 0;
		m_writeIndex += BUFFER_SIZE - start;
		start = m_writeIndex & BUFFER_MASK;
	}
	unsigned char *p = m_buffer + start;
	*p++ = framePosition;
	*p++ = framePosition >> 8;
	*p++ = framePosition >> 16;
	*p++ = framePosition >> 24;
	*p++ = nWrite;
	*p++ = nWrite >> 8;
	*p++ = nWrite >> 16;
	*p++ = nWrite >> 24;
	if(isLastMsgInsertedToRS)
		*p++ = m_lastStatusByte;
	for(int i = 0; i < nRawData; ++i)
	{
		p[i] = pRawData[i];
	}

	m_lastAppendIsInsert = isInsert;

/*
	if((p[0] & 0xF0) == 0xb0 && p[1] == 7) {
		int v = p[2] * m_channelCompression[p[0] & 0xF] / 127;
		if(v < 0)
			v = 0;
		else if(v > 127)
			v = 127;
		p[2] = v;
	}
*/
	m_writeIndex += 8 + nWrite;
	p = m_buffer + start + 8;
}

#ifdef DECAP_PLAYER

unsigned int CPsPortRS232::GetDecapCommand()
{
	if(m_hInPort == INVALID_HANDLE_VALUE)
		return DECAP_CMD_NONE;

	if(m_lastDecapCmd != DECAP_CMD_NONE)
	{
		unsigned int cmd = m_lastDecapCmd;
		m_lastDecapCmd = DECAP_CMD_NONE;
		return cmd;
	}
	return DECAP_CMD_NONE;
}

int CPsPortRS232::CalcPianoCH1Velocity(int v0)
{
	int CombinedVolume = m_ch1Ctrl07 * m_ch1Ctrl11 * m_ch1Ctrl89 / (127 * 127);
	if (CombinedVolume < m_ch1MinThreshVol)
		return 0;

	int FinalThreshVol = ((CombinedVolume - m_ch1MinThreshVol) * 127 / (127 - m_ch1MinThreshVol));

	int FinalVelocity = m_ch1CompMinVelo + (((127 - m_ch1CompMinVelo) * v0 * FinalThreshVol) / (127 * 127));

	//no need to check FinalVelocity bound here because it will be always in [1..127]
	return FinalVelocity;
}

#endif //DECAP_PLAYER

void CPsPortRS232::Send(unsigned int framePosition, void *pEvent)
{
	if(m_hOutPort != INVALID_HANDLE_VALUE)
	{
		framePosition += m_delay * 1000;
		CPsTrack::EVENT *pEvt = (CPsTrack::EVENT*)pEvent;
//		WCHAR s[100];
//		wsprintf(s, TEXT("evt: %d, %d, %d\n"), framePosition, pEvt->channel, pEvt->command);
//		OutputDebugString(s);
		switch(pEvt->eventCode)
		{
		case CPsTrack::EVENT_NOTE_ON:
			if(pEvt->par2 > 0)
				m_noteState[pEvt->channel][pEvt->par1]++;
			else
			{
				if(m_noteState[pEvt->channel][pEvt->par1] > 0)
					m_noteState[pEvt->channel][pEvt->par1]--;
			}
			break;

		case CPsTrack::EVENT_NOTE_OFF:
			if(m_noteState[pEvt->channel][pEvt->par1] > 0)
				--m_noteState[pEvt->channel][pEvt->par1];
			break;

		case CPsTrack::EVENT_META:
			return;
		}
#ifdef ENABLE_MONITOR
		if(pEvt->eventCode != CPsTrack::EVENT_NOTE_ON && pEvt->eventCode != CPsTrack::EVENT_NOTE_OFF)
			return;
#endif
		if(pEvt->eventCode >= 0xf0)
		{
			//SysEx
			AppendData(framePosition, &pEvt->command, 1);
			AppendData(framePosition, pEvt->pExtraData, pEvt->nExtraData);
		}
		else
		{
			if (m_channelCompression[pEvt->channel] == 0)
				return;

			if (pEvt->eventCode == CPsTrack::EVENT_CONTROL_CHANGE && pEvt->par1 == 7)
			{
				unsigned char dat[3];
				dat[0] = pEvt->command;
				dat[1] = 7;
				int v = pEvt->par2 * m_channelCompression[pEvt->channel] / 127;
				if(v < 0)
					v = 0;
				else if(v > 127)
					v = 127;
				dat[2] = v;
				AppendData(framePosition, dat, 3);
#ifdef DECAP_PLAYER
				if (pEvt->channel == 0)
				{
					m_ch1Ctrl07 = v;
				}
#endif
			}
			else if (m_ch1IsUseCompression && pEvt->eventCode == CPsTrack::EVENT_NOTE_ON && pEvt->channel == 0 && pEvt->par2 > 0)
			{
				int v = CalcPianoCH1Velocity(pEvt->par2);
				if (v > 0)
				{
					unsigned char dat[3];
					dat[0] = pEvt->command;
					dat[1] = pEvt->par1;
					dat[2] = v;
					AppendData(framePosition, dat, 3);
				}
			}
			else {
				if (pEvt->eventCode == CPsTrack::EVENT_CONTROL_CHANGE && pEvt->par1 == 11)
				{
#ifdef DECAP_PLAYER
					if (pEvt->channel == 0)
					{
						m_ch1Ctrl11 = pEvt->par2;
					}
#endif
				}
				if(pEvt->pRawData[0] < 0x80)//running status?
					AppendData(framePosition, &pEvt->command, 1);
				AppendData(framePosition, pEvt->pRawData, pEvt->nRawData);
			}
		}
	}
}

const char* CPsPortRS232::GetName()
{
	return m_hOutPort != INVALID_HANDLE_VALUE ? "RS232" : "RS232(disconnected)";
}

void CPsPortRS232::Reset()
{
//	m_readIndex = m_writeIndex = 0;
	AllNoteOff();
	int ctick = 0;
	unsigned char dat[3];
	dat[1] = 121;
	dat[2] = 0;
	int i;
	for(i = 0; i < 16; i++)
	{
		dat[0] = 0xb0 | i;
		AppendData(0, dat, 3);//use -10000 to make event sent as soon as possible
	}
	for(i = 0; i < 16; i++)
	{
		dat[0] = 0xb0 | i;
		dat[1] = 7;
		dat[2] = 100 * m_channelCompression[i] / 127;
		AppendData(0, dat, 3);
	}
#ifdef DECAP_PLAYER
	m_ch1Ctrl07 = m_ch1Ctrl11 = 127;
#endif
}

void CPsPortRS232::AllNoteOff()
{
	int ctick = 0;
	for(int chn = 0; chn < 16; chn++)
	{
		for(int i = 0; i < 128; i++)
		{
			if(m_noteState[chn][i])
			{
				unsigned char a[3];
				a[0] = 0x90 | chn;
				a[1] = i;
				a[2] = 0;
				while(m_noteState[chn][i] > 0)
				{
					AppendData(ctick, a, 3);
					--m_noteState[chn][i];
				}
			}
		}
		{
			//turn off hold padel
			unsigned char a[3];
			a[0] = 0xB0 | chn;
			a[1] = 64;
			a[2] = 0;
			AppendData(ctick, a, 3);
		}
	}
}

void CPsPortRS232::ReduceVoice()
{
}

int CPsPortRS232::GetActiveCount()
{
	return m_readIndex != m_writeIndex;
}

void CPsPortRS232::GMSystemOn(unsigned int *pProgList, int nProg, unsigned char *pDrumList)
{
#ifdef DEBUG_LOG
	MEMORYSTATUS    memInfo;
	memInfo.dwLength = sizeof(memInfo);
	GlobalMemoryStatus(&memInfo);

	fprintf(flog, "GMSystemOn, total mem: %d free mem: %d\n", memInfo.dwTotalPhys, memInfo.dwAvailPhys);
#endif

	Reset();
}

static __int64 GetTime()
{
	SYSTEMTIME lastSysTime;
	GetSystemTime(&lastSysTime);
	__int64 lastTime = (((lastSysTime.wDay * 24) + lastSysTime.wHour) * 60 + lastSysTime.wMinute) * 60;
	lastTime = (lastTime + lastSysTime.wSecond) * 1000 + lastSysTime.wMilliseconds;
	return lastTime;
}

DWORD CPsPortRS232::TestSuspendThread(LPVOID lpParameter)
{
	__int64 lastTime = GetTime();

	while(((CPsPortRS232*)lpParameter)->m_state == STATE_OPENED)
	{
#ifndef _WIN32_WCE_EMULATION
		__int64 curTime = GetTime();
		__int64 diff = curTime - lastTime;
		lastTime = curTime;
		if(diff > 2000 || (GetTickCount() - ((CPsPortRS232*)lpParameter)->m_writeTick) > 2000)
		{
			((CPsPortRS232*)lpParameter)->m_isPowerOffOccured = true;
			((CPsPortRS232*)lpParameter)->ReOpen();
			Sleep(1000);
			lastTime = GetTime();
//			PlaySound((LPCTSTR)IDR_WAVE1, GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);
		}
#endif
		Sleep(100);
	}
	return 0;
}

DWORD CPsPortRS232::WriteThread(LPVOID lpParameter)
{
	return ((CPsPortRS232*)lpParameter)->RunWrite();
}

DWORD CPsPortRS232::RunWrite()
{
	DWORD lastTick = GetTickCount();
	int prevPos = 0;
	while(m_state == STATE_OPENED || GetActiveCount())
	{
		DWORD nwrite = 0;
		int lastPos = (int)((F64)m_pSystem->GetDriver()->GetPlayingFramePosition() * 1000 / m_pSystem->GetConfig()->GetMixFrequency());
		int curTick = GetTickCount();
		int curPos = curTick - lastTick;
		if(curPos > 25)
			curPos = 25;
		if(lastPos != prevPos)
		{
			curPos = lastPos;
			prevPos = lastPos;
			lastTick = GetTickCount();
		}
		else
			curPos = prevPos + curPos;

		m_writeTick = GetTickCount();

		while(m_hOutPort != INVALID_HANDLE_VALUE && m_readIndex < m_writeIndex)
		{
			unsigned int start = m_readIndex & BUFFER_MASK;
			if(start + 8 >= BUFFER_SIZE)
			{
				//buffer wrap
				m_readIndex += BUFFER_SIZE - start;
				continue;
			}
			unsigned char *p = (m_buffer + start);
			int nPos = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
			p += 4;
			unsigned int nSize = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
			p += 4;
			if(nSize == 0)
			{
				//buffer wrap
				m_readIndex += BUFFER_SIZE - start;
				continue;
			}
			nPos /= 1000;
			if(nPos > curPos)
				break;
//			int delta = curPos - nPos;
//			printf("%d\n", nPos - curPos);
			if(nwrite + nSize > WRITE_CACHE_SIZE)
				break;
			m_readIndex += 8 + nSize;
//		static int aaa = 0;
//			printf("%d: %d, %d, %d, ", ++aaa, nTime, diff, nSize);
			while(nSize > 0)
			{
//				printf("%x, ", *p);
				m_writeCache[nwrite++] = *p++;
				--nSize;
			}
//			printf("\n");
		}
		if(nwrite)
		{
			DWORD nWritten;
			WriteFile(m_hOutPort, m_writeCache, nwrite, &nWritten, NULL);
#ifdef ENABLE_MONITOR
			m_monitor.Process(m_writeCache, nwrite);
#endif
#ifdef DEBUG_LOG
			for(int i = 0; i < nwrite; i++)
				fprintf(flog, "%02X ", m_writeCache[i]);
#endif
		}
#ifdef USE_HIGH_RES_TIMER
		WaitForSingleObject(m_hEvent, INFINITE);
#else
		Sleep(10);
#endif
	}
	m_state = STATE_CLOSED;
	return 0;
}

//#define TEST_READ_CMD
#ifdef TEST_READ_CMD
int testcmd;
#endif

DWORD CPsPortRS232::RunRead()
{
	int lastCommand = 0;
	int stage = 0;
	int idNRPN;

	while(m_state == STATE_OPENED)
	{
		if(m_hInPort != INVALID_HANDLE_VALUE)
		{
			unsigned char c;
			DWORD readlen = 0;
#ifdef TEST_READ_CMD
			if(testcmd)
			{
				m_readBuffer.Append(0xf3);
				m_readBuffer.Append(1);
				m_readBuffer.Append(1);
				testcmd = 0;
#else
			while(true)
			{
				bool isRead = ReadFile(m_hInPort, &c, 1, &readlen, NULL) && readlen == 1;
				if(!isRead)
					break;
				m_readBuffer.Append(c);
#endif
				++m_totalReceived;
				int cmd = m_readBuffer.Peek();
				int runningStatus = 0;
				if(cmd < 0x80)
				{
					runningStatus = 1;
					cmd = lastCommand;
				}
				if(cmd < 0xf0)
					lastCommand = cmd;
				switch(cmd & 0xf0)
				{
				case 0x80:
				case 0x90:
				case 0xa0:
				case 0xe0:
					if(runningStatus + m_readBuffer.GetLength() >= 3)
					{
						unsigned char dat[3];
						dat[0] = runningStatus ? cmd : m_readBuffer.GetFirst();
						dat[1] = m_readBuffer.GetFirst();
						dat[2] = m_readBuffer.GetFirst();
						AppendData(0, dat, 3, true);
					}
					break;
				case 0xc0:
				case 0xd0:
					if(runningStatus + m_readBuffer.GetLength() >= 2)
					{
						unsigned char dat[2];
						dat[0] = runningStatus ? cmd : m_readBuffer.GetFirst();
						dat[1] = m_readBuffer.GetFirst();
						AppendData(0, dat, 2, true);
					}
					break;

				case 0xf0:
					switch(cmd)
					{
/*
hexF3 will always have 2 data-bytes following it:
Data1 (byte0-127) = selection: 0 = song, 1 = playlist , 2~127 not yet 
specified
Data2 (byte0-127) = value (which song or which playlist, range 1 to 128)

hexF4 will always have 2 data-bytes following it:
Data1 (byte0-127) = selection: 0 = volume, 1~127 not yet specified
Data2 (byte0-127) = value

This hexF4/Data1 = 0 (volume) is used to control the volume bij means of the 
remote-control. So if this data is received by the player, it must be as if 
the volume-slider is moved by hand:
--slider moves to the correct position
--Audio-volume is set (mp3/soundmodule)
--MasterVolume controller is send to the rs232

hexF9 = PrevSong
hexFA = StartSong
hexFB = NextSong
hexFC = StopSong
hexFD = PauseSong

*/
					case 0xf3:
						if(m_readBuffer.GetLength() >= 3)
						{
							if(m_readBuffer.Peek(1) == 0)
							{
								m_lastDecapCmd = DECAP_CMD_SELECT_SONG | (m_readBuffer.Peek(2) << 16);
							}
							else if(m_readBuffer.Peek(1) == 1)
							{
								m_lastDecapCmd = DECAP_CMD_SELECT_LIST | (m_readBuffer.Peek(2) << 16);
							}
							m_readBuffer.Skip(3);
						}
						break;
					case 0xf4:
						if(m_readBuffer.GetLength() >= 3)
						{
							if(m_readBuffer.Peek(1) == 0)
							{
								m_lastDecapCmd = DECAP_CMD_SET_VOLUME | (m_readBuffer.Peek(2) << 16);
							}
							else if(m_readBuffer.Peek(1) == 1)
							{
								m_accordionMainMidChan = m_readBuffer.Peek(2);
							}
							else if(m_readBuffer.Peek(1) == 2)
							{
								m_accordionSubMidChan = m_readBuffer.Peek(2);
							}
							else if(m_readBuffer.Peek(1) == 3)
							{
								m_activeSensing = m_readBuffer.Peek(2);
							}
							m_readBuffer.Skip(3);
						}
						break;

					case 0xf9:
						m_lastDecapCmd = DECAP_CMD_PREV;
						m_readBuffer.Skip(1);
						break;
					case 0xfa:
						m_lastDecapCmd = DECAP_CMD_PLAY;
						m_readBuffer.Skip(1);
						break;
					case 0xfb:
						m_lastDecapCmd = DECAP_CMD_NEXT;
						m_readBuffer.Skip(1);
						break;
					case 0xfc:
						m_lastDecapCmd = DECAP_CMD_STOP;
						m_readBuffer.Skip(1);
						break;
					case 0xfd:
						m_lastDecapCmd = DECAP_CMD_PAUSE;
						m_readBuffer.Skip(1);
						break;
					default:
						m_readBuffer.Skip(1);
						break;
					}
					break;

				case 0xb0:
					if((runningStatus + m_readBuffer.GetLength()) > 2)
					{
						int ctlId = m_readBuffer.Peek(1 - runningStatus);
						if(ctlId == 99)
						{
							if(m_readBuffer.Peek(2 - runningStatus) == 96)
								stage = 1;
						}
						else if(ctlId == 98)
						{
							if(stage == 1 && m_readBuffer.Peek(2 - runningStatus) == 101)
								stage = 2;
						}
						else if(ctlId == 6)
						{
							if(stage == 2)
							{
								stage = 3;
								idNRPN = m_readBuffer.Peek(2 - runningStatus);
							}
						}
						else if(ctlId == 38)
						{
							if(stage == 3)
							{
								m_decapControlData[idNRPN] = m_readBuffer.Peek(2 - runningStatus);
							}
						}
						unsigned char dat[3];
						dat[0] = runningStatus ? cmd : m_readBuffer.GetFirst();
						dat[1] = m_readBuffer.GetFirst();
						dat[2] = m_readBuffer.GetFirst();
						AppendData(0, dat, 3, true);
					}
					break;
				default:
					m_readBuffer.Skip(1);
				}
			}
		}
		Sleep(10);
	}
	return 0;
}

DWORD CPsPortRS232::ReadThread(LPVOID lpParameter)
{
	return ((CPsPortRS232*)lpParameter)->RunRead();
}

void CPsPortRS232::SetMasterVolume(int val)
{
#ifdef DECAP_PLAYER
	if(m_hOutPort != INVALID_HANDLE_VALUE)
	{
		unsigned char dat[3];
		dat[1] = 89;
		dat[2] = (unsigned char)val;
		for(int i = 0; i < 16; i++)
		{
			dat[0] = 0xb0 | i;
			AppendData(0, dat, 3);//use -10000 to make event sent as soon as possible
		}
	}
	m_ch1Ctrl89 = val;
#endif
}

void CPsPortRS232::SendInstrumentSetting(int msb, int lsb)
{
	if(m_decapControlData[1] == -1)
		return;
	if(m_hOutPort != INVALID_HANDLE_VALUE)
	{
#ifdef DECAP_PLAYER
		unsigned char dat[12];
		dat[0] = 0xb0 | m_decapControlData[1];
		dat[1] = 99;
		dat[2] = 97;
		dat[3] = 0xb0 | m_decapControlData[1];
		dat[4] = 98;
		dat[5] = 100;
		dat[6] = 0xb0 | m_decapControlData[1];
		dat[7] = 6;
		dat[8] = msb;
		dat[9] = 0xb0 | m_decapControlData[1];
		dat[10] = 38;
		dat[11] = lsb;
		AppendData(0, dat, 12);//use -10000 to make event sent as soon as possible
#endif
	}	
}

#ifdef DECAP_PLAYER
void CPsPortRS232::UpdateDecapNRPNSetting()
{
	Sleep(20);
	SendInstrumentSetting(126, 126);
	Sleep(200);
}

/*
--Any time the player starts playing (ether by pressing PLAY (>) on the 
player, or by receiving FA-Hex on the RS232-RX port, or any other possible 
way that starts the player): FA-Hex
--Any time the player goes to the next song (ether by pressing NEXT (>>) on 
the player, or by receiving FB-Hex on the RS232-RX port, or any other 
possible way that advances the player): FB-Hex
--Any time the player stops playing (ether by pressing STOP ([]) on the 
player, or by receiving FC-Hex on the RS232-RX port, or any other possible 
way that stops the player): FC-Hex
*/
void CPsPortRS232::SendStatus(unsigned char msg)
{
	if(m_hOutPort != INVALID_HANDLE_VALUE)
	{
		AppendData(0, &msg, 1);
	}
}

void CPsPortRS232::SendChannelSetting( unsigned char chn, unsigned char id )
{
	if(m_hOutPort != INVALID_HANDLE_VALUE)
	{
		unsigned char dat[3];
		dat[0] = 0xf4;
		dat[1] = id;
		dat[2] = chn;
		AppendData(0, dat, 3);
	}
}


#endif
