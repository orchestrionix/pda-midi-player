#include "PsTrack.h"

CPsTrack::CPsTrack()
{
	m_trackState = STATUS_NO_EVENT;
}

int CPsTrack::Attach(const void *pTrack, int len, unsigned int chnmask)
{
	m_reader.Attach(pTrack, len);
	m_channelMask = chnmask;
	m_nextTick = 0;
	m_outputPort = 0;
	m_trackState = STATUS_MORE_EVENT;
	return PSERR_SUCCESS;
}

void CPsTrack::Detach()
{
	m_reader.Detach();
	m_trackState = STATUS_NO_EVENT;
}

//return true if read a event successfully, else false
bool CPsTrack::ReadEvent(CPsTrack::EVENT *pEvent)
{
	if(m_trackState == STATUS_LAST_EVENT)
		m_trackState = STATUS_NO_EVENT;

	if(m_trackState == STATUS_NO_EVENT)
		return false;

	unsigned char cmd;
	bool gotCmd;
	pEvent->deltaTime = 0;
	do{
		gotCmd = true;
		if(m_reader.GetRemain() < 1)
		{
			m_trackState = STATUS_NO_EVENT;
			return false;
		}
		int deltaTime = m_reader.ReadVarLen();
		pEvent->deltaTime += deltaTime;
		m_nextTick += deltaTime;
		pEvent->pRawData = m_reader.GetCurrent();
		if(m_reader.PeekInt8() < 0x80)
			cmd = m_lastEventCode;
		else
			cmd = m_reader.ReadInt8();
		pEvent->command = cmd;
		if(cmd < 0xf8)//exclude System Realtime message
			m_lastEventCode = cmd;
		switch(cmd & 0xf0)
		{
		case 0xf0:
			pEvent->eventCode = cmd;
			if(cmd < 0xf8)//System Common Category messages 
			{
				pEvent->nExtraData = m_reader.ReadVarLen();
				pEvent->pExtraData = m_reader.GetCurrent();
				if(m_reader.GetRemain() < pEvent->nExtraData || !m_reader.Skip(pEvent->nExtraData))
				{
					m_trackState = STATUS_NO_EVENT;
					return false;
				}
				if(!(m_channelMask & LEAD_TRACK))
					gotCmd = false;
			}
			else if(cmd == EVENT_META)
			{
				pEvent->par1 = m_reader.ReadInt8();
				pEvent->nExtraData = m_reader.ReadVarLen();
				pEvent->pExtraData = m_reader.GetCurrent();
				if(m_reader.GetRemain() < pEvent->nExtraData || !m_reader.Skip(pEvent->nExtraData))
				{
					m_trackState = STATUS_NO_EVENT;
					return false;
				}
				if(pEvent->par1 == 0x2f)//End of Track
				{
					m_trackState = STATUS_LAST_EVENT;
//					return false;
				}
				else if(pEvent->par1 == 0x21)
				{

				}
				else if(!(m_channelMask & LEAD_TRACK))
					gotCmd = false;
			}
			else
			{
				while(!m_reader.IsEof() && m_reader.PeekInt8() < 0x80)
					m_reader.Skip(1);
				gotCmd = false;
			}
			break;
			
		case 0x80://Note off
		case 0x90://Note on
		case 0xa0://Aftertouch
		case 0xb0://Controller
			pEvent->eventCode = cmd & 0xf0;
			pEvent->channel = cmd & 0xf;
			if(m_reader.GetRemain() >= 2)
			{
				pEvent->par1 = m_reader.ReadInt8();
				pEvent->par2 = m_reader.ReadInt8();
			}
			else
			{
				m_trackState = STATUS_NO_EVENT;
				return false;
			}
			if(!(m_channelMask & (1 << pEvent->channel)))
				gotCmd = false;
			break;
		case 0xc0://Program Change
		case 0xd0://Channel Pressure
			pEvent->eventCode = cmd & 0xf0;
			pEvent->channel = cmd & 0xf;
			if(m_reader.GetRemain() >= 1)
			{
				pEvent->par1 = m_reader.ReadInt8();
			}
			else
			{
				m_trackState = STATUS_NO_EVENT;
				return false;
			}
			if(!(m_channelMask & (1 << pEvent->channel)))
				gotCmd = false;
			break;
		case 0xe0://Pitch Wheel
			pEvent->eventCode = cmd & 0xf0;
			pEvent->channel = cmd & 0xf;
			if(m_reader.GetRemain() >= 2)
			{
				pEvent->par1 = m_reader.ReadInt14LE();
			}
			else
			{
				m_trackState = STATUS_NO_EVENT;
				return false;
			}
			if(!(m_channelMask & (1 << pEvent->channel)))
				gotCmd = false;
			break;
			
		default:
			PsAssert(0);
		}
		pEvent->nRawData = m_reader.GetCurrent() - pEvent->pRawData;
	}while(!gotCmd);
	return true;
}

void CPsTrack::Rewind()
{
	m_reader.Rewind();
	m_outputPort = 0;
	m_nextTick = 0;
	m_trackState = STATUS_MORE_EVENT;
}
