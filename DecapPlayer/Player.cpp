// Player.cpp: implementation of the CPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "win32/PsWinDriver.h"
#include "common/PsAudioSystem.h"
#include "common/PsSequencer.h"
#include "win32/PsFileReader.h"
#include "skin.h"
#include "PlayList.h"
#include "Player.h"
#include "register.h"
#include "registerCore.h"
#include "Utils.h"
#include "PsSys.h"
#include "common/PsPortSoftSynth.h"
#include "win32/PsPortRS232.h"
#include "resmgr/ucl/ucl_dcmp.h"
#include "mp3channel.h"
#include "skindata.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//GetDeviceUniqueID
extern HWND hwndMain;

static CPsAudioSystem system;
static CPsSequencer sequencer;
static CPsSequence sequence;
static CPsWinDriver driver;
CPlayer g_player;
static CPsFileReader sbfile;
static CPsSynthesizer softsynth;
static CPsPortSoftSynth softport;
CPsPortRS232 comport;
CMP3Channel mp3Channel;

/*
void tracelog(const char* s)
{
	return;
	HANDLE h = CreateFile(TEXT("\\decaplog.txt"), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return;
	DWORD nWrite;
	SetFilePointer(h, 0, NULL, FILE_END);
	WriteFile(h, s, strlen(s), &nWrite, NULL);
	CloseHandle(h);
}
*/

CPlayer::CPlayer()
{
	m_status = STATUS_UNINITED;
	m_idPlaying = 0;
	m_bLoop = false;
	m_isSingle = false;
	m_isAutoAdvance = false;
	m_enableDecapCommand = true;
	m_songLength = 0;
	m_setposbar = true;
	m_tick = 0;
	m_timeMode = 0;
	m_pMediaData = NULL;
	m_softSynthVolThreshold = 40;
	m_maxPoly = 40;
	m_isCloseOtherAppOnActive = true;
	CPsSys::memset(m_channelDelays, 0, sizeof(m_channelDelays));
}

CPlayer::~CPlayer()
{
	if(m_status != STATUS_UNINITED)
	{
		Destroy();
	}
}

bool CPlayer::Play()
{
	if(m_status == STATUS_UNINITED || m_status == STATUS_STOPPING)
		return false;

	if(m_status == STATUS_PAUSED)
	{
		system.GetMixer()->AddChannel(&mp3Channel);
		sequencer.Play();
	}
	else if(m_status == STATUS_READY)
	{
		m_cmdAfterStop = CMD_NONE;
		if(!m_pMediaData)
			Open(m_idPlaying);
		if(m_pMediaData)
		{
			if(m_pOutputPorts[0] == &comport || m_pOutputPorts[1] == &comport)
			{
				if(!comport.IsOpened())
				{
					if(!comport.Open(&system))
						MessageBox(hwndMain, TEXT("Open COM port failed."), TEXT("Warning"), MB_OK);
				}
			}
			softport.SetEnable(Regist_GetFeatureCode() & FEATURE_SOFTSYNTH ? true : false);
			if(system.LockWaveOutDevice())
			{
				if(mp3Channel.Open(&system, mp3fn))
					system.GetMixer()->AddChannel(&mp3Channel);
				system.GetMixer()->ResetAGC();
				sequencer.Play();
				SynchronizeCOMVolume();
				g_skin.DrawChannelNumbers();
			}
		}
	}
	m_status = sequencer.GetState() == CPsSequencer::STATE_PLAYING?STATUS_PLAYING:STATUS_READY;
	return true;
}

void CPlayer::FreeModule()
{
	ASSERT(m_status != STATUS_UNINITED);
	system.GetMixer()->RemoveChannel(&mp3Channel);
	mp3Channel.Close();

	sequencer.Stop();
	sequence.Detach();
	if(m_pMediaData)
	{
		delete m_pMediaData;
		m_pMediaData = NULL;
	}
	m_status = STATUS_READY;
}

void CPlayer::DrawTime()
{
	int pos;
	if(m_isStandaloneMp3)
		pos = (mp3Channel.m_totalDecoded * 1000 / system.GetConfig()->GetMixFrequency());
	else
		pos = sequencer.GetTimePosition();
	if(m_timeMode == 0)
	{
		g_skin.DrawTimer(pos / 1000, true);
	}
	else if(m_timeMode == 1)
	{
		g_skin.DrawTimer((pos - m_songLength) / 1000, true);
	}
	else
	{
		g_skin.DrawTimer(system.GetMixer()->GetMixChannels(), false);
	}
}

extern int totalPlayerTime;

void FixWM2005NoiseBug()
{
	system.LockWaveOutDevice();
	system.Update();
	Sleep(100);
	system.UnlockWaveOutDevice();
}

bool CPlayer::Update()
{
	if(m_status != STATUS_UNINITED)
	{
#ifdef ENABLE_MONITOR
		{
			TCHAR s[100];
			HDC dc = GetDC(hwndMain);
			RECT rc = {
				0, 0, 100, 30
			};
			wsprintf(s, TEXT("%d   "), comport.m_monitor.GetActiveNotes());
			DrawText(dc, s, -1, &rc, DT_LEFT | DT_TOP);
			ReleaseDC(hwndMain, dc);
		}
#endif
		if(comport.IsPowerOffOccured())
		{
			if(m_status == STATUS_READY)
			{
				//remove noise in WM2005
				FixWM2005NoiseBug();
				FixWM2005NoiseBug();
				comport.ClearPowerOffOccured();
			}
			else
				Stop();
		}
		system.Update();
		totalPlayerTime = GetTickCount();

#ifdef DEBUG
		softsynth.GetOscPool()->CheckConsistancy();
#endif
		if(m_pMediaData)
		{
			if(mp3Channel.GetState() == CMP3Channel::STATE_STOPPED && sequencer.GetState() == CPsSequencer::STATE_STOPPED)
			{
				if(m_status == STATUS_PLAYING)
				{
					if(m_isSingle)
					{
						m_status = STATUS_READY;
						if(m_isAutoAdvance)
							Next(false);
					}
					else
						Next(false);
				}
				else if(m_status == STATUS_STOPPING)
				{
					m_status = STATUS_READY;
					if(m_cmdAfterStop == CMD_OPEN)
					{
						m_cmdAfterStop = CMD_NONE;
						system.UnlockWaveOutDevice();
						Open(m_idPlaying);
					}
					else if(m_cmdAfterStop == CMD_OPEN_AND_PLAY)
					{
						m_cmdAfterStop = CMD_NONE;
						system.UnlockWaveOutDevice();
						if(Open(m_idPlaying))
							Play();
					}
					else if(m_isAutoAdvance || g_skin.PlaylistGetNext() != -1)
						Next(false);
				}
			}
			if(m_setposbar)
			{
				g_skin.SetBar(CSkin::CTL_POSITION, sequencer.GetTickPosition(), sequence.GetInfo()->nTick);
			}
			DrawGraph();
			if(m_status == STATUS_PLAYING || m_status == STATUS_STOPPING)
			{
				DrawTime();
				SystemIdleTimerReset();
				if((sequencer.GetState() == CPsSequencer::STATE_STOPPING1 || sequencer.GetState() == CPsSequencer::STATE_STOPPING2) && GetSongDelay() > 0)
				{
					int stopDelay = GetSongDelay();
					int stopCounter = sequencer.GetStopCount() / 1000;
					if(stopCounter < stopDelay)
					{
						wsprintf(m_title, TEXT("Pause: %dms (%d)"), stopDelay, stopDelay - stopCounter);
						ShowTitle();
					}
				}
			}
			else if(m_status == STATUS_PAUSED)
			{
				if((m_tick & 7) > 3)
					DrawTime();
				else
					g_skin.DrawTimer(0x10000001, true);
			}
			g_skin.DrawSwitch(BG_XML_mp3, CTLS_XML_mp3, mp3Channel.GetState() == CMP3Channel::STATE_PLAYING);
		}
#ifdef DECAP_PLAYER
		unsigned int decapCmd = comport.GetDecapCommand();
		if(m_enableDecapCommand)
		{
			switch(decapCmd & 0xffff)
			{
			case CPsPortRS232::DECAP_CMD_PAUSE:
				Pause();
				break;
			case CPsPortRS232::DECAP_CMD_PREV:
				Prev();
				break;
			case CPsPortRS232::DECAP_CMD_NEXT:
				Next(true);
				break;
			case CPsPortRS232::DECAP_CMD_PLAY:
				Play();
				break;
			case CPsPortRS232::DECAP_CMD_STOP:
				Stop();
				break;
			case CPsPortRS232::DECAP_CMD_SET_VOLUME:
				{
					int vol = (decapCmd >> 16) * 0xffff / 127;
					g_skin.SetBar(CSkin::CTL_VOLUME, vol, 0xffff);
					SetVolume(vol);
					SynchronizeCOMVolume();
				}
				break;
			case CPsPortRS232::DECAP_CMD_SELECT_SONG:
				SelectPlaylistItem(decapCmd >> 16);
				break;
			case CPsPortRS232::DECAP_CMD_SELECT_LIST:
				g_playlist.Open(hwndMain, decapCmd >> 16);
				break;
			}
		}
#endif //DECAP_PLAYER
		if(m_status == STATUS_READY)
			m_stoptick++;
		else
			m_stoptick = 0;
		if(m_stoptick == 1)
		{
			DrawGraph();
			g_skin.DrawTimer(0x10000000, true);
			system.UnlockWaveOutDevice();
		}
		if(m_prevStatus != m_status)
		{
			if(m_status == STATUS_READY)
				comport.SendStatus(0xFC);
			if(m_prevStatus == STATUS_READY)
				comport.SendStatus(0xFA);
			m_prevStatus = m_status;
		}
		totalPlayerTime = GetTickCount() - totalPlayerTime;
	}
	g_skin.DrawSwitch(BG_XML_rs232, CTLS_XML_rs232, m_lastRs232Received < comport.GetTotalReceived());
	m_lastRs232Received = comport.GetTotalReceived();
	m_tick++;
	return false;
}

int CPlayer::calcKey(int userId, int fileLen)
{
	int ret;
	ret = (fileLen ^ 0x304d5751) * 211 ^ userId * 11;

	return ret % 19973;
}

void CPlayer::decrypt(unsigned char *pBuf, int len, int key)
{
	int i;
	unsigned char magic[256];

	int tmp = key * 13;
	for(i = 0; i < 256; i++)
	{
		tmp = (tmp ^ 0x9a87df9f) * 13 % 0x54781 >> 2;
		magic[i] = (unsigned char)tmp;
	}

	for(i = 0; i < len; i++)
	{
		pBuf[i] ^= magic[i & 255];
	}
}

bool CPlayer::DecryptMIDI(DWORD &len)
{
	BYTE *pSrc = (BYTE*)m_pMediaData;
	if(*(int*)(pSrc + 4) != 0x10000)
	{
		MessageBox(hwndMain, TEXT("Unsupported file format."), TEXT("Error"), MB_OK);
		return false;
	}
	
	unsigned int midiSize = *(int*)(pSrc + 12);
	if(!Regist_IsRegistered() || *(int*)(pSrc + 8) != calcKey(Regist_GetUserId(), midiSize))
	{
		MessageBox(hwndMain, TEXT("Sorry, you are not authorized to play this file."), TEXT("Error"), MB_OK);
		return false;
	}

	decrypt(pSrc + 16, len - 16, Regist_GetUserId());

	BYTE *pD = new BYTE[midiSize];

	unsigned int ds;
	ucl_nrv2e_decompress_le32(pSrc + 16, len - 16, pD, &ds);

	delete m_pMediaData;
	m_pMediaData = pD;

	len = ds;
	return true;
}

bool CPlayer::Open(LPCTSTR fn)
{
	FreeModule();

	int fnlen = lstrlen(fn);
	const TCHAR *p = fn + fnlen - 1;
	while(p >= fn && *p != TEXT('\\')) p--;
	p++;

	m_isStandaloneMp3 = false;
	if(fnlen > 4)
	{
		if(lstrcmpi(fn + fnlen - 4, TEXT(".mp3")) == 0)
			m_isStandaloneMp3 = true;
	}

	lstrcpy(mp3fn, fn);
	lstrcpy(mp3fn + lstrlen(mp3fn) - 3, TEXT("mp3"));

	DWORD len;
	if(m_isStandaloneMp3)
	{
		if(!mp3Channel.Open(&system, mp3fn))
			return false;
		int total_time = mp3Channel.GetTotalTime();
		mp3Channel.Close();
		m_pMediaData = new char[64];
		CPsMidiWriter writer(m_pMediaData, 64);
		writer.WriteInt32(0x4d546864);
		writer.WriteInt32(6);
		writer.WriteInt16(0);//format
		writer.WriteInt16(1);//nTrack
		writer.WriteInt16(1000);//nDivision
		writer.WriteInt32(0x4d54726b);
		unsigned char *pLen = writer.GetCurrent();
		writer.WriteInt32(10);
		writer.WriteInt8(0);
		writer.WriteInt8(0xff);
		writer.WriteInt8(0x51);
		writer.WriteInt8(0x03);
		writer.WriteInt8(0x0f);
		writer.WriteInt8(0x42);
		writer.WriteInt8(0x40);
		writer.WriteVarLen(total_time);
		writer.WriteInt8(0xff);
		writer.WriteInt16(0x2f00);
		len = writer.GetCurrent() - (unsigned char*)m_pMediaData;
		int trklen = writer.GetCurrent() - pLen - 4;
		writer.SetCurrent(pLen);
		writer.WriteInt32(trklen);
	}
	else
	{
		HANDLE h = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h == INVALID_HANDLE_VALUE)
			goto failed;
		len = GetFileSize(h, NULL);
		m_pMediaData = new char[len];
		if(!ReadFile(h, m_pMediaData, len, &len, NULL))
		{
			CloseHandle(h);
			delete[] m_pMediaData;
			m_pMediaData = NULL;
			goto failed;
		}
		CloseHandle(h);
	}

	if(*(int*)m_pMediaData == 0x44494d44)
	{
		//decompress midi file
		if(!DecryptMIDI(len))
		{
			delete[] m_pMediaData;
			m_pMediaData = NULL;
			goto failed;
		}
	}

	sequence.Attach(m_pMediaData, len);
	sequencer.SetSequence(&sequence);
	m_songLength = sequence.GetInfo()->length;
	wsprintf(m_title, TEXT("%d. %s (%02d:%02d)"), g_playlist.GetShuffledOrder(m_idPlaying) + 1, p, (m_songLength / 1000)/ 60, (m_songLength / 1000) % 60);
	ShowTitle();
	g_skin.SetBar(CSkin::CTL_POSITION, 0, sequence.GetInfo()->nTick);
	return true;
failed:
	wsprintf(m_title, TEXT("Can't play \"%s\""), p);
	ShowTitle();
	return false;
}

int CPlayer::GetStatus()
{
	return m_status;
}

bool CPlayer::Pause()
{
	if(GetStatus() == STATUS_PLAYING)
	{
		sequencer.Pause();
		if(sequencer.GetState() == CPsSequencer::STATE_PAUSED)
		{
			system.GetMixer()->RemoveChannel(&mp3Channel);
			m_status = STATUS_PAUSED;
			return true;
		}
	}
	return false;
}

bool CPlayer::Init()
{
	if(m_status == STATUS_UNINITED)
	{
		if(system.Open(&driver, 22050, 700))
		{
			system.GetMixer()->SetAGC(true);
//			testReverb();
			m_totalMixTime = 0;
			m_totalMixChannel = 0;
			m_curChnVBuf = 0;
			CPsSys::memset(m_chnVBuf, 0, sizeof(m_chnVBuf));
			TCHAR fn[MAX_PATH];
			CUtils::MapToInstalledPath(TEXT("gm.psb"), fn);
#ifdef UNICODE
			char myfn[MAX_PATH];
			WideCharToMultiByte(CP_OEMCP, 0, fn, -1, myfn, MAX_PATH, NULL, NULL);
			if(sbfile.Open(myfn, CPsFileReader::F_READ))
#else
			if(sbfile.Open(fn, CPsFileReader::F_READ))
#endif
			{
				if(softsynth.Open(&system, &sbfile))
				{
					softsynth.GetOscPool()->SetPolyNumber(m_maxPoly);
					softport.AttachSynthesizer(&softsynth);
					system.AttachOutputPort(1, &softport);
					comport.Open(&system);
					m_lastRs232Received = 0;
					system.AttachOutputPort(0, &comport);
					m_pOutputPorts[0] = &comport;
					m_pOutputPorts[1] = &softport;
					softport.SetEnable(Regist_GetFeatureCode() & FEATURE_SOFTSYNTH ? true : false);
					sequencer.Open(&system);

					DWORD vol;
					if(waveOutGetVolume(0, &vol) == MMSYSERR_NOERROR)
					{
						SetVolume(vol & 0xffff);
					}
					FixWM2005NoiseBug();
					FixWM2005NoiseBug();
					m_prevStatus = m_status = STATUS_READY;
					return true;
				}
			}
			system.Close();
		}
		return false;
	}
	return true;
}

bool CPlayer::Open(int id)
{
	Stop();
	if(g_playlist.GetItemCount() == 0)
	{
		HandleEmptyList();
		return false;
	}
	if(id >= g_playlist.GetItemCount())
		id = 0;
	m_idPlaying = id;
	g_skin.PlaylistSelectItem(m_idPlaying);
	return Open(g_playlist.GetShuffledFileName(id));
}

void CPlayer::Stop()
{
	if(m_status == STATUS_PLAYING || m_status == STATUS_PAUSED)
	{
		system.GetMixer()->RemoveChannel(&mp3Channel);
		mp3Channel.Close();
//		g_skin.SetBar(CSkin::CTL_POSITION, 0, midiInfo.nTick);
		sequencer.EndPlay();
//		sequencer.Stop();
		m_status = STATUS_STOPPING;
//		g_skin.DrawTimer(0);
	}
//	FreeModule();
}

void CPlayer::Next(bool forcePlay)
{
	int oldstat = m_status;
	if(m_status == STATUS_UNINITED || m_status == STATUS_STOPPING) return;
	Stop();
	if(g_playlist.GetItemCount() > 0)
	{
		comport.SendStatus(0xfb);
		if(g_skin.PlaylistGetNext() != -1)
			m_idPlaying = g_skin.PlaylistGetNext();
		else
			m_idPlaying++;
		if(m_idPlaying >= g_playlist.GetItemCount())
		{
			m_idPlaying = m_bLoop?0:g_playlist.GetItemCount() - 1;
			if(oldstat == STATUS_PLAYING)
				m_cmdAfterStop = (forcePlay || m_bLoop) ? CMD_OPEN_AND_PLAY : CMD_OPEN;
			else
				Open(m_idPlaying);
		}
		else
		{
			if(oldstat == STATUS_PLAYING)
				m_cmdAfterStop = CMD_OPEN_AND_PLAY;
			else
				Open(m_idPlaying);
		}
		g_skin.PlaylistSelectItem(m_idPlaying);
	}
	else
		HandleEmptyList();
}

void CPlayer::Prev()
{
	int oldstat = m_status;
	if(m_status == STATUS_UNINITED || m_status == STATUS_STOPPING) return;
	Stop();
	if(g_playlist.GetItemCount() > 0)
	{
		m_idPlaying--;
		if(m_idPlaying < 0)
		{
			m_idPlaying = m_bLoop?g_playlist.GetItemCount() - 1:0;
		}
		if(oldstat == STATUS_PLAYING)
			m_cmdAfterStop = CMD_OPEN_AND_PLAY;
		else
			Open(m_idPlaying);
		g_skin.PlaylistSelectItem(m_idPlaying);
	}
	else
		HandleEmptyList();
}

void CPlayer::Backward()
{
	int i, m;
	if(m_pMediaData)
	{
		i = GetPosition();
		m = GetMaxPostion();
		i = i - m / 8;
		if(i < 0) i = 0;
		SetPosition(i);
//		if(m_status != STATUS_PLAYING)
//			Play();
	}
}

void CPlayer::Forward()
{
	int i, m;
	if(m_pMediaData)
	{
		m = GetMaxPostion();
		i = GetPosition() + m / 8;
		if(i > m - (2 << 10))
			i = m - (2 << 10);
		SetPosition(i);
//		if(m_status != STATUS_PLAYING)
//			Play();
	}
}

void CPlayer::HandleEmptyList()
{
	m_idPlaying = 0;
	g_skin.SetTitle(TEXT(""));
}

bool CPlayer::SetSurround(bool enabled)
{
	if(m_status == STATUS_UNINITED) return false;
	system.GetMixer()->SetSurround(enabled);
	return enabled;
}

bool CPlayer::GetSurround()
{
	if(m_status == STATUS_UNINITED) return false;
	return system.GetMixer()->IsSurround();
}

bool CPlayer::SetReverse(bool enabled)
{
	if(m_status == STATUS_UNINITED) return false;
	system.GetMixer()->SetSwapLR(enabled);
	return enabled;
}

bool CPlayer::GetReverse()
{
	return system.GetMixer()->IsSwapLR();
}

int CPlayer::GetVolume()
{
	DWORD v;
	if(waveOutGetVolume(0, &v) == MMSYSERR_NOERROR)
	{
		int vol = v & 0xffff;
		if(m_softSynthVolThreshold == 100)
			return 0xffff;
		vol = (vol * 100 - m_softSynthVolThreshold * 0xffff) / (100 - m_softSynthVolThreshold);
		if(vol < 0)
			vol = 0;
		if(vol > 0xffff)
			vol = 0xffff;
		return vol;
	}
	return 0;
}

int CPlayer::SetVolume(int vol)
{
	vol = (vol * (100 - m_softSynthVolThreshold) + m_softSynthVolThreshold * 0xffff) / 100;

	if(waveOutSetVolume(0, vol | (vol << 16)) == MMSYSERR_NOERROR)
		return vol;
	return 0;
}

int CPlayer::GetPosition()
{
	if(m_pMediaData == NULL) return 0;
	return sequencer.GetTickPosition();
}

int CPlayer::SetPosition(int pos)
{
	if(m_status != STATUS_PLAYING || m_status == STATUS_PAUSED)
		return 0;
	if(m_pMediaData == NULL)
		return 0;
//	if(m_status != STATUS_PLAYING && m_status != STATUS_PAUSED)
	sequencer.SetTickPosition(pos);
	return pos;
}

int CPlayer::GetMaxPostion()
{
	if(m_pMediaData == NULL) return 0;
	return sequence.GetInfo()->nTick;
}

void CPlayer::ShowTitle()
{
	g_skin.SetTitle(m_title);
}

int CPlayer::GetReverb()
{
	if(m_status == STATUS_UNINITED) return 0;
	return F2I(system.GetMixer()->GetReverbVolume() * 32);
}

int CPlayer::SetReverb(int revb)
{
	if(m_status == STATUS_UNINITED) return 0;
	if(revb > 32)revb = 32;
	system.GetMixer()->SetReverbVolume(I2F(revb) / 32);
	return revb;
}

int CPlayer::GetPanSep()
{
	if(m_status == STATUS_UNINITED) return 100;
	return F2I(system.GetMixer()->GetPanSeperation() * 100);
}

int CPlayer::SetPanSep(int ps)
{
	if(m_status == STATUS_UNINITED) return 100;
	system.GetMixer()->SetPanSeperation(I2F(ps) / 100);
	return ps;
}

void CPlayer::DrawGraph()
{
	softsynth.GetOscPool()->GetChannelVolumes(m_chnVBuf[m_curChnVBuf]);
	m_curChnVBuf = (m_curChnVBuf + 1) % NUM_CHNV_BUF;
	g_skin.DrawMeter(m_chnVBuf[m_curChnVBuf]);
}

void CPlayer::Destroy()
{
	FreeModule();
	system.DetachOutputPort(0);
	system.DetachOutputPort(1);
	comport.Close();
	softport.DetachSynthesizer();
	softsynth.Close();
	sequencer.Close();
	system.UnlockWaveOutDevice();
	system.Close();
	m_status = STATUS_UNINITED;
}

static const F32 s_tempo[9] =
{
	PSFLOAT2F(0.5), PSFLOAT2F(0.75), PSFLOAT2F(0.88), PSFLOAT2F(0.95), F_ONE, PSFLOAT2F(1.06), PSFLOAT2F(1.15), PSFLOAT2F(1.25), PSFLOAT2F(1.5)
};

void CPlayer::SetTempo(int i)
{
	CPsObject::PsAssert(i >= 0 && i < sizeof(s_tempo) / sizeof(s_tempo[0]));
	sequencer.SetTempoScale(s_tempo[i]);
}

void CPlayer::SetKeyTranspose(int i)
{
	softsynth.SetTranspose(i);
}

void CPlayer::SetMute(int channel, bool isMute)
{
	softsynth.SetMute(channel, isMute);
}

bool CPlayer::IsMute(int channel)
{
	return softsynth.IsMute(channel);
}

int CPlayer::GetProgram(int channel)
{
	return softsynth.GetProgram(channel);
}

void CPlayer::SetProgram(int channel, int prog)
{
	softsynth.SetProgram(channel, prog);
}

void CPlayer::SetSolo(int channel)
{
	int i;
	for(i = 0; i < 16; i++)
	{
		SetMute(i, i != channel);
	}
}

int CPlayer::GetOutputPortsScheme(int *pIndex)
{
	CPsOutputPort **p = system.GetOutputPorts();
	for(int i = 0; i < system.GetOutputPortNumber(); i ++)
	{
		pIndex[i] = -1;
		for(int j = 0; j < NUM_OUTPUT_PORT; j++)
		{
			if(p[i] == m_pOutputPorts[j])
			{
				pIndex[i] = j;
				break;
			}
		}
	}
	return system.GetOutputPortNumber();
}

int CPlayer::GetOutputPortNumber()
{
	return system.GetOutputPortNumber();
}

void CPlayer::SetOutputPortsScheme(int *pIndex, int num)
{
	for(int i = 0; i < num; i ++)
	{
		system.AttachOutputPort(i, m_pOutputPorts[pIndex[i]]);
	}
}

F16 CPlayer::GetMixVolume()
{
	return system.GetMixer()->GetMixVolume();
}

void CPlayer::SetMixVolume(F16 vol)
{
	system.GetMixer()->SetMixVolume(vol);
}

void CPlayer::SetBassVolume(F16 vol)
{
	system.GetMixer()->SetBassVolume(vol);
}

F16 CPlayer::GetBassVolume()
{
	return system.GetMixer()->GetBassVolume();
}

void CPlayer::SetBassCutoff(int freq)
{
	system.GetMixer()->SetBassCutoff(freq);
}

int CPlayer::GetBassCutoff()
{
	return system.GetMixer()->GetBassCutoff();
}

void CPlayer::SetCOMDelay(int delay)
{
	comport.SetDelay(delay);
}

int CPlayer::GetCOMDelay()
{
	return comport.GetDelay();
}

int CPlayer::GetTrueFrequency()
{
	return driver.GetPlayingFramePosition();
}

void CPlayer::SynchronizeCOMVolume()
{
	comport.SetMasterVolume(GetVolume() >> 9);
}

void CPlayer::StopAndPlay(int id)
{
	m_idPlaying = id;
	if(m_status == STATUS_PLAYING)
	{
		Stop();
		m_cmdAfterStop = CMD_OPEN_AND_PLAY;
	}
	else if(m_status != STATUS_STOPPING)
	{
		if(Open(m_idPlaying))
			Play();
	}
	g_skin.PlaylistSelectItem(m_idPlaying);
}

void CPlayer::AllControllerOff()
{
	comport.Reset();
	softport.Reset();
}

void CPlayer::SendInstrumentSetting(int lsb, int msb)
{
	comport.SendInstrumentSetting(lsb, msb);
}

int CPlayer::GetAccordionMainMidChan()
{
	return comport.GetAccordionMainMidChan();
}

void CPlayer::SetAccordionMainMidChan(int chn)
{
	comport.SetAccordionMainMidChan(chn);
}

int CPlayer::GetAccordionSubMidChan()
{
	return comport.GetAccordionSubMidChan();
}

void CPlayer::SetAccordionSubMidChan(int chn)
{
	comport.SetAccordionSubMidChan(chn);
}

CPsPortRS232* CPlayer::GetRS232()
{
	return &comport;
}

void CPlayer::SetMaxPoly(int i)
{
	if(i > CPsOscPool::MAX_POLYPHONY)
		i = CPsOscPool::MAX_POLYPHONY;
	if(i < 30)
		i = 30;
	m_maxPoly = i;
	softsynth.GetOscPool()->SetPolyNumber(i);
}

int CPlayer::GetMaxPoly()
{
	return softsynth.GetOscPool()->GetPolyNumber();
}

void CPlayer::SetChannelDelayTable(short* delayTable)
{
	CPsSys::memcpy(m_channelDelays, delayTable, sizeof(delayTable[0]) * 16);
	int i;
	int minval = 0;
	for(i = 0; i < 16; i++)
	{
		if(delayTable[i] < minval)
			minval = delayTable[i];
	}
	if(minval < 0)
	{
		minval = -minval;
		short *pDelay;
		pDelay = softport.GetChannelDelays();
		for(i = 0; i < 16; i++)
		{
			pDelay[i] = minval;
		}
		pDelay = comport.GetChannelDelays();
		for(i = 0; i < 16; i++)
		{
			pDelay[i] = delayTable[i] + minval;
		}
	}
	else
	{
		CPsSys::memset(softport.GetChannelDelays(), 0, sizeof(delayTable[0]) * 16);
		CPsSys::memcpy(comport.GetChannelDelays(), delayTable, sizeof(delayTable[0]) * 16);
	}
}

void CPlayer::GetChannelDelayTable(short* delayTable)
{
	CPsSys::memcpy(delayTable, m_channelDelays, sizeof(delayTable[0]) * 16);
}

char* CPlayer::GetChannelCompressionTable(int port)
{
	if (port == 0)
		return softport.GetChannelCompression();
	else
		return comport.GetChannelCompression();
}

void CPlayer::SelectPlaylistItem(int id)
{
	if(GetStatus() == STATUS_READY || g_skin.PlaylistGetNext() == id)
		g_player.StopAndPlay(id);
	else
		g_skin.PlaylistSetNext(id);
}

void CPlayer::SetMp3Volume(F16 vol)
{
	mp3Channel.SetMixVolume(vol);
}

F16 CPlayer::GetMp3Volume()
{
	return (F16)mp3Channel.GetMixVolume();
}

int CPlayer::GetActiveSensing()
{
	return comport.GetActiveSensing();
}

void CPlayer::SetActiveSensing( int isEnabled )
{
	comport.SetActiveSensing(isEnabled);
}

int CPlayer::GetSongDelay() const
{
	return sequencer.GetStopDelayTime() / 1000;
}

void CPlayer::SetSongDelay( int nDelay )
{
	sequencer.SetStopDelayTime(nDelay * 1000);
}