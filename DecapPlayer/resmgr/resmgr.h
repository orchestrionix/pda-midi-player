// ResMgr.h: interface for the CResMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESMGR_H__DA92C48C_81EB_42C6_96B0_EE0792A1BF11__INCLUDED_)
#define AFX_RESMGR_H__DA92C48C_81EB_42C6_96B0_EE0792A1BF11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define RESMGR_USE_ZLIB
//#define RESMGR_USE_BZ2
//#define RESMGR_USE_LZO
#define RESMGR_USE_UCL
//#define RESMGR_USE_LZSS

#define RESMGR_GET_STRING_COUNT(p) (((int*)(p))[0] / 4)
#define RESMGR_GET_STRING(p, id) ((const char *)(p) + ((int*)(p))[(id)])

class CResMgr  
{
public:
	int GetRecordSize(unsigned int gid);
	void CloseRecord(unsigned int id);
	void* OpenRecord(unsigned int id);
	typedef struct {
		HANDLE			hFile;
		unsigned int	nRecord;
		unsigned int	*pOfsTable;
	}RES_FILE;
	
	typedef struct {
		unsigned char compressMethod;
		char pad0, pad1, pad2;
		unsigned int decompressedSize;
	}RES_RECORD_HDR;
	
	typedef struct _RES_RECORD{
		unsigned int recID;
		unsigned int nOpenCount;
		unsigned int dwSize;
		void *pBuffer;
		struct _RES_RECORD *pNext;
	}RES_RECORD;

	void CloseFile(int id);
	bool OpenFile(LPCTSTR fn, int id);

	enum {
		MAX_RES_FILE = 2,
		INVALID_RECORD_ID = 0xffffffff
	};
	CResMgr();
	virtual ~CResMgr();

protected:
	void ICloseRecord(RES_RECORD *pParentRecord);
	RES_RECORD* IOpenRecord(unsigned int id);
	RES_RECORD* FindParentRecordByID(unsigned int id);
	RES_RECORD* FindRecordByID(unsigned int id);
	RES_FILE	*m_arFiles[MAX_RES_FILE];
	RES_RECORD	*m_pRecordLink;
};

#endif // !defined(AFX_RESMGR_H__DA92C48C_81EB_42C6_96B0_EE0792A1BF11__INCLUDED_)
