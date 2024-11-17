// ResMgr.cpp: implementation of the CResMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "ResMgr.h"

#ifdef RESMGR_USE_ZLIB
//#include "zlib/zlib.h"
#endif
#ifdef RESMGR_USE_BZ2
//#include "bz2/bzlib.h"
#endif
#ifdef RESMGR_USE_LZO
//#include "lzo/minilzo.h"
#endif
#ifdef RESMGR_USE_UCL
#include "ucl/ucl_dcmp.h"
#endif
#ifdef RESMGR_USE_LZSS
//#include "lzss/lzss.h"
#endif

#define RES_GETFILEID(x) ((x) >> 16)
#define RES_GETRECID(x) ((x) & 0xffff)

#define COPYRIGHT_LEN	64

#define GET_NEXT_RND_SEED(x) ((x) * 51197 / 7)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResMgr::CResMgr()
{
	memset(m_arFiles, 0, sizeof(m_arFiles));
	m_pRecordLink = new RES_RECORD;
	m_pRecordLink->pNext = NULL;
}

CResMgr::~CResMgr()
{
	int i;
	for(i = 0; i < MAX_RES_FILE; i++)
	{
		if(m_arFiles[i])
			CloseFile(i);
	}
	while(m_pRecordLink->pNext)
		ICloseRecord(m_pRecordLink);
	delete m_pRecordLink;
}

bool CResMgr::OpenFile(LPCTSTR fn, int id)
{
	void* h;
	RES_FILE *prf;
	unsigned int n, rndSeed;
	ULONG dwRead;

	h = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return false;
	SetFilePointer(h, COPYRIGHT_LEN, NULL, FILE_BEGIN);
	if(!ReadFile(h, &rndSeed, 4, &dwRead, NULL) || dwRead != 4)
	{
		CloseHandle(h);
		return false;
	}
	ASSERT(id < MAX_RES_FILE);
	ASSERT(m_arFiles[id] == NULL);
	m_arFiles[id] = prf = new RES_FILE;
	m_arFiles[id]->hFile = h;
	rndSeed = GET_NEXT_RND_SEED(rndSeed);
	if(!ReadFile(h, &prf->nRecord, 4, &dwRead, NULL) || dwRead != 4)
	{
		CloseHandle(h);
		delete m_arFiles[id];
		m_arFiles[id] = NULL;
		return false;
	}
	prf->nRecord ^= rndSeed;
	n = (prf->nRecord + 1) * 4;
	prf->pOfsTable = new unsigned int[prf->nRecord + 1];
	if(!ReadFile(h, prf->pOfsTable, n, &dwRead, NULL) || dwRead != n)
	{
		delete[] prf->pOfsTable;
		CloseHandle(h);
		delete m_arFiles[id];
		m_arFiles[id] = NULL;
		return false;
	}
	for(n = 0; n < prf->nRecord + 1; n++)
	{
		rndSeed = GET_NEXT_RND_SEED(rndSeed);
		prf->pOfsTable[n] ^= rndSeed;
	}
	return true;
}

void CResMgr::CloseFile(int id)
{
	ASSERT(id < MAX_RES_FILE);
	if(m_arFiles[id])
	{
		if(m_arFiles[id]->hFile != INVALID_HANDLE_VALUE)
			CloseHandle(m_arFiles[id]->hFile);
		if(m_arFiles[id]->pOfsTable)
			delete[] m_arFiles[id]->pOfsTable;
		delete m_arFiles[id];
	}
	m_arFiles[id] = NULL;
}

CResMgr::RES_RECORD* CResMgr::FindRecordByID(unsigned int id)
{
	RES_RECORD *p = (RES_RECORD*)((RES_RECORD*)m_pRecordLink)->pNext;
	while(p && p->recID == id)
	{
		return p;
	}
	return NULL;
}

CResMgr::RES_RECORD* CResMgr::FindParentRecordByID(unsigned int id)
{
	RES_RECORD *p = m_pRecordLink;
	while(p->pNext && p->pNext->recID == id)
	{
		return p;
	}
	return NULL;
}

CResMgr::RES_RECORD* CResMgr::IOpenRecord(unsigned int id)
{
	RES_RECORD *pRec;
	int fid, rid;
	RES_FILE *f;
	RES_RECORD_HDR hdr;
	unsigned int len;
	unsigned char *pTemp;
	ULONG dwRead;

	fid = RES_GETFILEID(id);
	rid = RES_GETRECID(id);
	ASSERT(fid < MAX_RES_FILE);
	f = m_arFiles[fid];
	ASSERT(f != NULL);//Make sure this file has been opened
	len = f->pOfsTable[rid + 1] - f->pOfsTable[rid];
	SetFilePointer(f->hFile, f->pOfsTable[rid], NULL, FILE_BEGIN);
	if(!ReadFile(f->hFile, (void*)&hdr, sizeof(RES_RECORD_HDR), &dwRead, NULL))
	{
		ASSERT(0);
	}
	ASSERT(dwRead == sizeof(RES_RECORD_HDR));
	pRec = new RES_RECORD;
	pRec->recID = id;
	pRec->dwSize = hdr.decompressedSize;
	pRec->pBuffer = new char[hdr.decompressedSize];
	len -= sizeof(RES_RECORD_HDR);
	pTemp = new unsigned char[len];
	if(!ReadFile(f->hFile, (void*)pTemp, len, &dwRead, NULL))
	{
		ASSERT(0);
	}
	ASSERT(dwRead == len);
	if(hdr.compressMethod == 0)
	{
		memcpy(pRec->pBuffer, pTemp, pRec->dwSize);
	}
	else if(hdr.compressMethod == 1)
	{
#ifdef RESMGR_USE_ZLIB
		len = pRec->dwSize;
		uncompress(pRec->pBuffer, &len, pTemp, dwRead);
		ASSERT(len == pRec->dwSize);
#else
		ASSERT(0);
#endif
	}
	else if(hdr.compressMethod == 2)
	{
#ifdef RESMGR_USE_BZ2
		len = pRec->dwSize;
		BZ2_bzBuffToBuffDecompress((char*)pRec->pBuffer, (unsigned int*)&len, (char*)pTemp, dwRead, 1, 0);
		ASSERT(len == pRec->dwSize);
#else
		ASSERT(0);
#endif
	}
	else if(hdr.compressMethod == 3)
	{
#ifdef RESMGR_USE_LZO
		len = pRec->dwSize;
		lzo1x_decompress((const lzo_byte*)pTemp, dwRead, (lzo_byte*)pRec->pBuffer, (lzo_uint*)&len, NULL);
		ASSERT(len == pRec->dwSize);
#else
		ASSERT(0);
#endif
	}
	else if(hdr.compressMethod == 4)
	{
#ifdef RESMGR_USE_UCL
		len = pRec->dwSize;
		ucl_nrv2e_decompress_le32((const unsigned char*)pTemp, dwRead, (unsigned char*)pRec->pBuffer, (unsigned int*)&len);
		ASSERT(len == pRec->dwSize);
#else
		ASSERT(0);
#endif
	}
	else if(hdr.compressMethod == 5)
	{
#ifdef RESMGR_USE_LZSS
		void *pWorkMen = new char[4142];
		len = LZSS_DeCompress((unsigned char*)pRec->pBuffer, (const unsigned char*)pTemp, dwRead, pWorkMen);
		delete[] pWorkMen;
		ASSERT(len == pRec->dwSize);
#else
		ASSERT(0);
#endif
	}
	else ASSERT(0);
	delete[] pTemp;
	pRec->nOpenCount = 0;
	pRec->pNext = ((RES_RECORD*)m_pRecordLink)->pNext;
	((RES_RECORD*)m_pRecordLink)->pNext = pRec;
	return pRec;
}

void* CResMgr::OpenRecord(unsigned int id)
{
	RES_RECORD *pRec;

	pRec = FindRecordByID(id);
	if(!pRec)
		pRec = IOpenRecord(id);
	pRec->nOpenCount++;
	return pRec->pBuffer;
}

void CResMgr::ICloseRecord(RES_RECORD *pParentRecord)
{
	RES_RECORD *pRec = pParentRecord->pNext;

	delete[] pRec->pBuffer;
	pParentRecord->pNext = pRec->pNext;
	delete pRec;
}

void CResMgr::CloseRecord(unsigned int id)
{
	RES_RECORD *pRec, *pParRec;

	pParRec = FindParentRecordByID(id);
	ASSERT(pParRec != NULL);
	pRec = pParRec->pNext;
	pRec->nOpenCount--;
	if(pRec->nOpenCount <= 0)
	{
		ICloseRecord(pParRec);
	}
}

int CResMgr::GetRecordSize(unsigned int gid)
{
	RES_RECORD *pRec = FindRecordByID(gid);
	if(pRec)
		return pRec->dwSize;
	else
	{
		int fid, rid;
		RES_FILE *f;
		RES_RECORD_HDR hdr;
		ULONG dwRead;

		fid = RES_GETFILEID(gid);
		rid = RES_GETRECID(gid);
		ASSERT(fid < MAX_RES_FILE);
		f = m_arFiles[fid];
		ASSERT(f != NULL);//Make sure this file has been opened
		SetFilePointer(f->hFile, f->pOfsTable[rid], NULL, FILE_BEGIN);
		if(!ReadFile(f->hFile, (void*)&hdr, sizeof(RES_RECORD_HDR), &dwRead, NULL))
		{
			ASSERT(0);
		}
		ASSERT(dwRead == sizeof(RES_RECORD_HDR));
		return hdr.decompressedSize;
	}
}
