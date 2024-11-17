#include "PsSoundBank.h"
#include "PsSys.h"

CPsSoundBank::CPsSoundBank()
{
	m_pPatchMap = NULL;
	m_pSBReader = NULL;
	m_pOffsets = NULL;
	m_pPatchs = NULL;
	m_pWaves = NULL;
	m_pPatchRefs = NULL;
	m_pWaveRefs = NULL;
	m_nPatch = 0;
	m_nWave = 0;
}

CPsSoundBank::~CPsSoundBank()
{
	Free();
}

void CPsSoundBank::Free()
{
	FreeAllPatchs();
	m_pSBReader = NULL;
	delete m_pPatchMap;
	m_pPatchMap = NULL;
	delete m_pPatchs;
	m_pPatchs = NULL;
	delete[] m_pWaves;
	m_pWaves = NULL;
	delete m_pPatchRefs;
	m_pPatchRefs = NULL;
	delete m_pWaveRefs;
	m_pWaveRefs = NULL;
	delete m_pOffsets;
	m_pOffsets = NULL;
	m_nPatch = 0;
	m_nWave = 0;
}

bool CPsSoundBank::Load(CPsReader *pReader)
{
	unsigned int n;
	Free();
	m_pSBReader = pReader;
	if(m_pSBReader->GetLength() < 4 + 4 + 128 + 16)
		return false;
	if(m_pSBReader->SetPosition(0, CPsReader::REF_BEGIN) != 0)
		return false;
	if(m_pSBReader->ReadInt() != 0x42535350)
		return false;
	if(m_pSBReader->ReadInt() != 0x30000)
		return false;
	m_pSBReader->SetPosition(128, CPsReader::REF_CURRENT);
	if((m_nPatch = m_pSBReader->ReadInt()) <= 0)
		return false;
	if((m_nWave = m_pSBReader->ReadInt()) <= 0)
		return false;
	n = m_nPatch * 2;
	m_pPatchMap = new unsigned int[n];
	if(!m_pPatchMap)
		return false;
	n *= sizeof(unsigned int);
	if(m_pSBReader->ReadBlock(m_pPatchMap, n) != n)
	{
		delete m_pPatchMap;
		m_pPatchMap = NULL;
		return false;
	}
	m_pPatchs = new CPsPatch*[m_nPatch];
	if(!m_pPatchs)
	{
		Free();
		return false;
	}
	m_pPatchRefs = new unsigned short[m_nPatch];
	if(!m_pPatchRefs)
	{
		Free();
		return false;
	}
	CPsSys::memset(m_pPatchRefs, 0, sizeof(unsigned short) * m_nPatch);
	m_pWaves = new CPsPatchWave*[m_nWave];
	if(!m_pWaves)
	{
		Free();
		return false;
	}
	m_pWaveRefs = new unsigned short[m_nWave];
	if(!m_pWaveRefs)
	{
		Free();
		return false;
	}
	CPsSys::memset(m_pWaveRefs, 0, sizeof(unsigned short) * m_nWave);
	n = m_nPatch + m_nWave + 1;
	m_pOffsets = new unsigned int[n];
	if(!m_pOffsets)
	{
		Free();
		return false;
	}
	n *= sizeof(unsigned int);
	if(!m_pSBReader->ReadBlock(m_pOffsets, n))
	{
		Free();
		return false;
	}
	return true;
}

CPsPatch* CPsSoundBank::LoadPatch(CPsMath *pMath, unsigned int bank, unsigned int prog, unsigned char *pRgnMap)
{
	if(!m_pPatchMap)
		return NULL;
	int id = FindPatch(bank, prog);
	if(id < 0)
		return NULL;
	if(m_pPatchRefs[id])
	{
		m_pPatchRefs[id]++;
		return m_pPatchs[id];
	}
	if(m_pSBReader->SetPosition(m_pOffsets[id], CPsReader::REF_BEGIN) == m_pOffsets[id])
	{
		m_pPatchs[id] = new CPsPatch();
		if(m_pPatchs[id])
		{
			if(m_pPatchs[id]->Load(m_pSBReader, bank, prog))
			{
				if(UpdateWaveLink(pMath, id, pRgnMap))
				{
					m_pPatchRefs[id] = 1;
					return m_pPatchs[id];
				}
				m_pPatchs[id]->Free();
			}
			delete m_pPatchs[id];
		}
	}
	return NULL;
}

void CPsSoundBank::FreePatch(CPsPatch* p)
{
	unsigned int i;

	for(i = 0; i < m_nPatch; i++)
	{
		if(m_pPatchs[i] == p && m_pPatchRefs[i] > 0)
		{
			if(m_pPatchRefs[i] == 1)
			{
				int j;
				CPsPatch::PSREGION *prgn;
				for(j = 0; j < p->GetRegionNumber(); j++)
				{
					prgn = p->GetRegion(j);
					FreeWave((CPsPatchWave*)prgn->waveInfo.waveLink);
				}
				delete m_pPatchs[i];
			}
			m_pPatchRefs[i]--;
			return;
		}
	}
}

bool CPsSoundBank::UpdateWaveLink(CPsMath *pMath, int instId, unsigned char *pRgnMap)
{
	CPsPatch::PSREGION *prgn;
	int i;
	CPsPatch *p = m_pPatchs[instId];
	for(i = 0; i < p->GetRegionNumber(); i++)
	{
		prgn = p->GetRegion(i);
		if(pRgnMap)
		{
			int j;
			for(j = prgn->minKey; j <= prgn->maxKey; j++)
			{
				if(pRgnMap[j])
					break;
			}
			if(j > prgn->maxKey)
			{
				prgn->waveInfo.waveLink = 0;
				continue;
			}
		}
		CPsPatchWave *pWave = LoadWave(pMath, prgn->waveInfo.waveLink);
		prgn->waveInfo.waveLink = (unsigned int)pWave;
		if(prgn->waveInfo.waveLink == 0 || ! pWave->ExpandTo16Bit())
		{
			for(int j = 0; j < i; j++)
				FreeWave(pWave);
			return false;
		}
		if(prgn->waveInfo.loopType == CPsPatch::SAMPLE_LOOPFORWARD)
			pWave->UpdateWaveForInterpolation(prgn->waveInfo.loopStart, prgn->waveInfo.loopLength);
		else
			pWave->UpdateWaveForInterpolation(-1, -1);
	}
	return true;
}

CPsPatchWave* CPsSoundBank::LoadWave(CPsMath *pMath, int id)
{
	if(m_pWaveRefs[id] == 0)
	{
		if(m_pSBReader->SetPosition(m_pOffsets[m_nPatch + id], CPsReader::REF_BEGIN) != m_pOffsets[m_nPatch + id])
			return NULL;
		m_pWaves[id] = new CPsPatchWave();
		if(!m_pWaves[id])
			return NULL;
		if(!m_pWaves[id]->Load(pMath, m_pSBReader, id))
		{
			delete m_pWaves[id];
			m_pWaves[id] = NULL;
			return NULL;
		}
	}
	m_pWaveRefs[id]++;
	return m_pWaves[id];
}

void CPsSoundBank::FreeWave(CPsPatchWave* pSample)
{
	if(!pSample)
		return;
	unsigned int i;

	for(i = 0; i < m_nWave; i++)
	{
		if(m_pWaves[i] == pSample && m_pWaveRefs[i] > 0)
		{
			if(m_pWaveRefs[i] == 1)
			{
				delete m_pWaves[i];
				m_pWaves[i] = NULL;
			}
			m_pWaveRefs[i]--;
			return;
		}
	}
}

int CPsSoundBank::FindPatch(unsigned int bank, unsigned int prog)
{
	unsigned int i;
	for(i = 0; i < m_nPatch; i++)
	{
		if(m_pPatchMap[i*2] == bank && m_pPatchMap[i*2 + 1] == prog)
			return i;
	}
	bank &= 0x80007f00;
	for(i = 0; i < m_nPatch; i++)
	{
		if((m_pPatchMap[i*2] & 0x80007f00) == bank && m_pPatchMap[i*2 + 1] == prog)
			return i;
	}
	bank &= 0x80000000;
	for(i = 0; i < m_nPatch; i++)
	{
		if((m_pPatchMap[i*2] & 0x80000000) == bank && m_pPatchMap[i*2 + 1] == prog)
			return i;
	}
	if(bank & 0x80000000)
	{
		for(i = 0; i < m_nPatch; i++)
		{
			if(m_pPatchMap[i*2] & 0x80000000)
				return i;
		}
	}
	return -1;
}

void CPsSoundBank::FreeAllPatchs()
{
	unsigned int i;
	if(m_pPatchRefs)
	{
		for(i = 0; i < m_nPatch; i++)
		{
			if(m_pPatchRefs[i] > 0)
			{
				m_pPatchRefs[i] = 1;
				FreePatch(m_pPatchs[i]);
			}
		}
	}
	if(m_pWaveRefs)
	{
		for(i = 0; i < m_nWave; i++)
		{
			if(m_pWaveRefs[i] > 0)
			{
				m_pWaveRefs[i] = 1;
				FreeWave(m_pWaves[i]);
			}
		}
	}
}
