#ifndef __PSMIDIIO_H__
#define __PSMIDIIO_H__

#include "PsObject.h"

class CPsMidiWriter : public CPsObject
{
public:
	CPsMidiWriter(void* pBuf, int len);
	bool WriteInt32(int i);
	bool WriteInt16(int i);
	bool WriteInt14LE(int i);
	bool WriteVarLen(unsigned int value);
	bool WriteInt8(int i);
	bool WriteBlock(const void* pBuf, int len);

	unsigned char* GetCurrent(){
		return m_pCur;
	}

	void SetCurrent(void* p){
		m_pCur = (unsigned char*)p;
	}

	bool IsBufferFull(){
		return m_isFull;
	}

protected:
	unsigned char *m_pBuf, *m_pCur;
	int m_nSize;
	bool m_isFull;
};

class CPsMidiReader : public CPsObject
{
public:
	int GetRemain();
	int ReadInt14LE();
	bool Skip(int n);
	int PeekInt8();
	void Detach();
	void Attach(const void* pBuf, int len);
	void Rewind();
	bool IsEof();
	int ReadBlock(void *pBuf, int len);
	int ReadVarLen();
	int ReadInt8();
	int ReadInt16();
	int ReadInt32();
	CPsMidiReader();

	const unsigned char* GetCurrent(){
		return m_pCur;
	}

	int GetOffset(){
		return m_pCur - m_pBuf;
	}

protected:
	const unsigned char *m_pBuf, *m_pCur;
	int m_nSize;
};

#endif
