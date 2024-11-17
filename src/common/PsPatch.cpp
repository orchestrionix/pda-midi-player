#include "PsPatch.h"

CPsPatch::CPsPatch()
{
	m_pData = NULL;
}

CPsPatch::~CPsPatch()
{
	Free();
}

bool CPsPatch::Load(CPsReader *pReader, unsigned int bank_id, unsigned int prog_id)
{
	PsAssert(prog_id < 256);

	m_bank = (unsigned char)((bank_id >> 8) & 0x7f);
	m_bank |= (bank_id >> 24) & 0x80;
	m_subBank = (unsigned char)(bank_id & 0x7f);

	m_prog = (unsigned char)prog_id;
	
	pReader->ReadBlock(m_szName, (unsigned char)pReader->ReadChar());
	m_nRegion = (unsigned char)pReader->ReadShort();
	int nArt = pReader->ReadShort();

	PsAssert(m_nRegion > 0);
	PsAssert(IsDrum() ? m_nRegion == nArt : nArt == 1);

	int i = m_nRegion*sizeof(PSREGION) + nArt*sizeof(PSARTICULATION);
	m_pData = new unsigned char[i];
	if(!m_pData)
		return false;
	if(!pReader->ReadBlock(m_pData, i))
	{
		Free();
		return false;
	}
	return true;
}

void CPsPatch::Free()
{
	delete m_pData;
	m_pData = NULL;
}

CPsPatch::PSREGION* CPsPatch::GetRegion(int id)
{
	PsAssert(id >= 0 && id < m_nRegion);
	return (PSREGION*)(m_pData + (IsDrum()?m_nRegion:1) * sizeof(PSARTICULATION) + id * sizeof(PSREGION));
}

CPsPatch::PSREGION* CPsPatch::FindRegion(int note, int &id)
{
	PSREGION *pRgn;

	int i;
	int n = GetRegionNumber();
	for(i = n - 1; i >= 0; i--)
	{
		pRgn = GetRegion(i);
		if(note >= pRgn->minKey && note <= pRgn->maxKey)
		{
			id = i;
			return pRgn;
		}
	}
	id = -1;
	return NULL;
}
