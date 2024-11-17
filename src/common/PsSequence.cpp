#include "PsSequence.h"
#include "PsSys.h"

CPsSequence::CPsSequence()
{
	m_pMidi = NULL;
	m_info.nMidiTrack = 0;
	m_nTrack = 0;
}

int CPsSequence::Attach(const void *pMidi, int len)
{
	PsAssert(m_pMidi == NULL);//make sure detached
	int err = CollectMidiInfo(pMidi, len, &m_info);
	if(err != PSERR_SUCCESS)
		return err;
	m_pMidi = (const unsigned char*)pMidi;
	int trkid = 0;
	for(int i = 0; i < m_info.nMidiTrack; i++)
	{
		int nchn = 0;

		for(int j = 0; j < 16; ++j)
		{
			if(m_info.trackChannel[i] & (1 << j))
			{
				m_track[trkid].Attach(m_pMidi + m_info.trackStart[i], m_info.trackLength[i], (nchn == 0 ? CPsTrack::LEAD_TRACK : 0) | (1 << j));
				++nchn;
				if(++trkid >= MAX_TRACK)
					break;
			}
		}

		if(trkid >= MAX_TRACK)
			break;
		//we must attach to a track even the midi track has no channel event, because it
		//may contain system events
		if(nchn == 0)
		{
			m_track[trkid].Attach(m_pMidi + m_info.trackStart[i], m_info.trackLength[i], CPsTrack::LEAD_TRACK | 0xffff);
			if(++trkid >= MAX_TRACK)
				break;
		}
	}
	m_nTrack = trkid;
	return PSERR_SUCCESS;
}

void CPsSequence::Detach()
{
	for(int i = 0; i < MAX_TRACK; i++)
	{
		m_track[i].Detach();
	}
	m_pMidi = NULL;
}

void CPsSequence::Rewind()
{
	for(int i = 0; i < m_nTrack; i++)
	{
		m_track[i].Rewind();
	}
}

//#include <stdio.h>
static void AddProgramToMidiInfo(CPsSequence::MIDIINFO *pInfo, unsigned int prog)
{
	int i;
	if(pInfo->nProgram >= sizeof(pInfo->programs) / sizeof(pInfo->programs[0]))
		return;
	for(i = 0; i < pInfo->nProgram; i++)
	{
		if(pInfo->programs[i] == prog)
			return;
	}
	pInfo->programs[pInfo->nProgram++] = prog;
}

int CPsSequence::CollectMidiInfo(const void *pSrc, int nSrc, MIDIINFO *pInfo)
{
	pInfo->length = 0;
	pInfo->nMidiTrack = 0;
	pInfo->nTick = 0;
	pInfo->nProgram = 0;
	CPsSys::memset(pInfo->drums , 0, sizeof(pInfo->drums));
	CPsSys::memset(pInfo->trackChannel , 0, sizeof(pInfo->trackChannel));

	if(!pSrc || nSrc < 22)
		return PSERR_INVALID_PARAM;

	CPsMidiReader r;
	r.Attach(pSrc, nSrc);

	int magicnum = r.ReadInt32();
	if(magicnum == 0x52494646)
	{
		int len;
		len = r.ReadInt32();
		if(r.ReadInt32() != 0x524d4944)
			return PSERR_INVALID_FORMAT;
		while(r.ReadInt32() != 0x64617461)
		{
			len = r.ReadInt32();
			r.Skip(len);
			if(r.IsEof())
				return PSERR_INVALID_FORMAT;
		}
		r.ReadInt32();
		magicnum = r.ReadInt32();
	}

	if(magicnum != 0x4d546864)
		return PSERR_INVALID_FORMAT;
	if(r.ReadInt32() != 6)
		return PSERR_INVALID_FORMAT;
	pInfo->format = r.ReadInt16();

	pInfo->nMidiTrack = r.ReadInt16();
	if(pInfo->nMidiTrack < 1)
		return PSERR_INVALID_FORMAT;
	if(pInfo->nMidiTrack > MAX_TRACK)
		pInfo->nMidiTrack = MAX_TRACK;
	pInfo->nDivision = r.ReadInt16();

	CPsTrack tracks[MAX_TRACK];
	CPsTrack::EVENT evt[MAX_TRACK];
	CPsTrack::EVENT *pEvt;

	unsigned char lastPort[MAX_TRACK];
	char progDefined[MAX_OUTPUT_PORT][16];//set to 1 if Program_Change event fired
	unsigned short lastBank[MAX_OUTPUT_PORT][16];

	CPsSys::memset(progDefined, 0, sizeof(progDefined));
	CPsSys::memset(lastPort, 0, sizeof(lastPort));
	CPsSys::memset(lastBank, 0, sizeof(lastBank));

	int i;
	for(i = 0; i < MAX_OUTPUT_PORT; i++)
		lastBank[i][DRUM_CHANNEL] = 0x8000;

	CPsSys::memset(pInfo->hasNote, 0, sizeof(pInfo->hasNote));

	for(i = 0; i < pInfo->nMidiTrack; i++)
	{
		if(r.IsEof())
			return PSERR_INVALID_DATA;
		while(r.ReadInt32() != 0x4d54726b)
		{
			r.Skip(r.ReadInt32());
			if(r.IsEof())
				return PSERR_INVALID_DATA;
		}
		pInfo->trackLength[i] = r.ReadInt32();
		pInfo->trackStart[i] = r.GetOffset();
		tracks[i].Attach(r.GetCurrent(), pInfo->trackLength[i], CPsTrack::LEAD_TRACK | 0xffff);
		tracks[i].ReadEvent(evt + i);
		r.Skip(pInfo->trackLength[i]);
	}
	int ntick, mintrk;
	int tempo = DEFAULT_MIDI_TEMPO, tempotick = 0;
	while(true)
	{
		mintrk = -1;
		ntick = 0x7fffffff;
		for(i = 0; i < pInfo->nMidiTrack; i++)
		{
			if(!tracks[i].IsEndOfTrack() && tracks[i].GetNextTick() < ntick)
			{
				ntick = tracks[i].GetNextTick();
				mintrk = i;
			}
		}
		if(mintrk < 0)
			break;
		pInfo->nTick = ntick;
		int trkport = lastPort[mintrk];
		pEvt = evt + mintrk;
		if(pEvt->command < 0xf0)
			pInfo->trackChannel[mintrk] |= 1 << pEvt->channel;

		if(pEvt->eventCode == CPsTrack::EVENT_META)
		{
			if(pEvt->par1 == 0x51)
			{
				pInfo->length += (int)((F64)(ntick - tempotick) * tempo / (pInfo->nDivision * 1000));
				tempo = (pEvt->pExtraData[0] << 16) | (pEvt->pExtraData[1] << 8) | pEvt->pExtraData[2];
				tempotick = ntick;
			}
			else if(pEvt->par1 == 0x21)
			{
				lastPort[mintrk] = pEvt->pExtraData[0];//MIDI Port event
			}
		}
		else if(trkport < MAX_OUTPUT_PORT)
		{
			if(pEvt->eventCode == CPsTrack::EVENT_NOTE_ON)
			{
				if(!progDefined[trkport][pEvt->channel])
				{
					AddProgramToMidiInfo(pInfo, (pEvt->channel == DRUM_CHANNEL ? 0x8000 : 0) | (lastBank[trkport][pEvt->channel] << 16));
					progDefined[trkport][pEvt->channel] = 1;
				}
				if(pEvt->channel == DRUM_CHANNEL)
				{
					pInfo->drums[pEvt->par1] = 1;
				}
				pInfo->hasNote[mintrk] = 1;
			}
			else if(pEvt->eventCode ==  CPsTrack::EVENT_PROGRAM_CHANGE)
			{
				AddProgramToMidiInfo(pInfo, (pEvt->channel == DRUM_CHANNEL ? 0x8000 : 0) | (lastBank[trkport][pEvt->channel] << 16) | pEvt->par1);
				progDefined[trkport][pEvt->channel] = 1;
			}
			else if(pEvt->eventCode ==  CPsTrack::EVENT_CONTROL_CHANGE)
			{
				switch(pEvt->par1)
				{
				case 0://Coarse Bank Select
					lastBank[trkport][pEvt->channel] = (unsigned short)((pEvt->par2 << 8) | (lastBank[trkport][pEvt->channel] & 0xff));
					break;
				case 32://Fine Bank Select
					lastBank[trkport][pEvt->channel] = (unsigned short)((pEvt->par2 & 0xff) | (lastBank[trkport][pEvt->channel] & 0xff00));;
					break;
				}
			}
		}
		tracks[mintrk].ReadEvent(evt + mintrk);
	}

	pInfo->length += (int)((F64)(pInfo->nTick - tempotick) * tempo / (pInfo->nDivision * 1000));

	return PSERR_SUCCESS;
}

int CPsSequence::FindNote(const void *pSrc, int nSrc, FINDNOTEINFO *pInfo)
{
	CPsSys::memset(pInfo->notes, 0, sizeof(pInfo->notes));
	pInfo->totalNote = 0;

	if(!pSrc || nSrc < 22)
		return PSERR_INVALID_PARAM;

	CPsMidiReader r;
	r.Attach(pSrc, nSrc);

	if(r.ReadInt32() != 0x4d546864)
		return PSERR_INVALID_FORMAT;
	if(r.ReadInt32() != 6)
		return PSERR_INVALID_FORMAT;
	int fmt = r.ReadInt16();
	int nMidiTrack = r.ReadInt16();
	if(nMidiTrack > MAX_TRACK)
		return PSERR_INVALID_FORMAT;
	int nDivision = r.ReadInt16();

	CPsTrack track;
	CPsTrack::EVENT evt;

	int programs[16];
	CPsSys::memset(programs, 0, sizeof(programs));

	int i, len;
	for(i = 0; i < nMidiTrack; i++)
	{
		if(r.IsEof())
			return PSERR_INVALID_DATA;
		while(r.ReadInt32() != 0x4d54726b)
		{
			len = r.ReadInt32();
			if(r.GetRemain() >= len)
				r.Skip(len);
			else
				return PSERR_INVALID_DATA;
			if(r.IsEof())
				return PSERR_INVALID_DATA;
		}
		len = r.ReadInt32();
		track.Attach(r.GetCurrent(), len, 0xffff);
		while(track.ReadEvent(&evt))
		{
			if(evt.eventCode == CPsTrack::EVENT_PROGRAM_CHANGE)
				programs[evt.channel] = evt.par1;
			else if(evt.eventCode == CPsTrack::EVENT_NOTE_ON && evt.par2 > 0)
			{
				pInfo->totalNote++;
				if(evt.channel == 9)
				{
					if(pInfo->bank & 0x80000000)
					{
						pInfo->notes[evt.par1]++;
					}
				}
				else if(!(pInfo->bank & 0x80000000) && programs[evt.channel] == pInfo->program)
				{
					pInfo->notes[evt.par1]++;
				}
			}
		}
		if(r.GetRemain() >= len)
			r.Skip(len);
		else
			return PSERR_INVALID_DATA;
	}

	return PSERR_SUCCESS;
}

