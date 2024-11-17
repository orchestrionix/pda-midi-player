#include "PsMidiIO.h"

CPsMidiWriter::CPsMidiWriter(void* pBuf, int len)
{
	m_pCur = m_pBuf = (unsigned char*)pBuf;
	m_nSize = len;
	m_isFull = false;
}

bool CPsMidiWriter::WriteInt32(int i)
{
	if(m_pCur - m_pBuf > m_nSize - 4)
	{
		m_isFull = true;
		return false;
	}
	*m_pCur++ = (unsigned char)(i >> 24);
	*m_pCur++ = (unsigned char)(i >> 16);
	*m_pCur++ = (unsigned char)(i >> 8);
	*m_pCur++ = (unsigned char)i;
	return true;
}

bool CPsMidiWriter::WriteInt16(int i)
{
	if(m_pCur - m_pBuf > m_nSize - 2)
	{
		m_isFull = true;
		return false;
	}
	*m_pCur++ = (unsigned char)(i >> 8);
	*m_pCur++ = (unsigned char)i;
	return true;
}

bool CPsMidiWriter::WriteInt14LE(int i)
{
	if(m_pCur - m_pBuf > m_nSize - 2)
	{
		m_isFull = true;
		return false;
	}
	*m_pCur++ = (unsigned char)(i & 0x7f);
	*m_pCur++ = (unsigned char)((i >> 7) & 0x7f);
	return true;
}

bool CPsMidiWriter::WriteVarLen(unsigned int value)
{
	unsigned long buffer;
	
	if(m_pCur - m_pBuf > m_nSize - 4)
	{
		m_isFull = true;
		return false;
	}
	buffer = value & 0x7F;
	
	while ( (value >>= 7) )
	{
		buffer <<= 8;
		buffer |= ((value & 0x7F) | 0x80);
	}
	
	while (true)
	{
		*m_pCur++ = (unsigned char)buffer;
		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
	}
	return true;
}

bool CPsMidiWriter::WriteInt8(int i)
{
	if(m_pCur - m_pBuf >= m_nSize)
	{
		m_isFull = true;
		return false;
	}
	*m_pCur++ = (unsigned char)i;
	return true;
}

bool CPsMidiWriter::WriteBlock(const void* pBuf, int len)
{
	const unsigned char* p = (unsigned char*)pBuf;

	if(m_pCur - m_pBuf > m_nSize - len)
	{
		m_isFull = true;
		return false;
	}
	while(len > 0)
	{
		*m_pCur++ = *p++;
		len--;
	}
	return true;
}

#define DEBUG_MIDIIO
#ifdef DEBUG_MIDIIO
#define TEST_OVERFLOW() 	{if(m_pCur - m_pBuf > m_nSize)\
								PsAssert(0);}
#else
#define TEST_OVERFLOW()
#endif

CPsMidiReader::CPsMidiReader()
{
	m_pCur = m_pBuf = NULL;
	m_nSize = 0;
}

int CPsMidiReader::ReadInt32()
{
	int r;

	if(m_pCur - m_pBuf <= m_nSize - 4)
		r = (m_pCur[0] << 24) | (m_pCur[1] << 16) | (m_pCur[2] << 8) | m_pCur[3];
	else
		r = 0;
	m_pCur += 4;
	TEST_OVERFLOW();
	return r;
}

int CPsMidiReader::ReadInt16()
{
	int r;

	if(m_pCur - m_pBuf <= m_nSize - 2)
		r = (m_pCur[0] << 8) | m_pCur[1];
	else
		r = 0;
	m_pCur += 2;
	TEST_OVERFLOW();
	return r;
}

int CPsMidiReader::ReadInt8()
{
	int r;

	if(m_pCur - m_pBuf < m_nSize)
		r = m_pCur[0];
	else
		r = 0;
	m_pCur++;
	TEST_OVERFLOW();
	return r;
}

int CPsMidiReader::ReadVarLen()
{
    int value = 0;
    unsigned char c;
	
	if(m_pCur - m_pBuf < m_nSize)
	{
		if((value = *m_pCur++) & 0x80)
		{
			value &= 0x7F;
			do
			{
				if(m_pCur - m_pBuf >= m_nSize)
					break;
				value = (value << 7) + ((c = *m_pCur++) & 0x7F);
			} while (c & 0x80);
		}
	}
	
	TEST_OVERFLOW();
    return value;
}

int CPsMidiReader::ReadBlock(void *pBuf, int len)
{
	unsigned char *p = (unsigned char*)pBuf;
	if(m_pCur - m_pBuf + len > m_nSize)
		len = m_nSize - (m_pCur - m_pBuf);
	while(len > 0)
	{
		*p++ = *m_pCur++;
		len--;
	}
	TEST_OVERFLOW();
	return p - (unsigned char*)pBuf;
}

bool CPsMidiReader::IsEof()
{
	return m_pCur - m_pBuf >= m_nSize;
}

void CPsMidiReader::Rewind()
{
	m_pCur = m_pBuf;
}

void CPsMidiReader::Attach(const void* pBuf, int len)
{
	m_pCur = m_pBuf = (unsigned char*)pBuf;
	m_nSize = len;
}

void CPsMidiReader::Detach()
{
	m_pCur = m_pBuf = NULL;
	m_nSize = 0;
}

int CPsMidiReader::PeekInt8()
{
	int r;

	if(m_pCur - m_pBuf < m_nSize)
		r = m_pCur[0];
	else
		r = 0;
	TEST_OVERFLOW();
	return r;
}

bool CPsMidiReader::Skip(int n)
{
	m_pCur += n;
	TEST_OVERFLOW();
	return m_pCur - m_pBuf <= m_nSize;
}

int CPsMidiReader::ReadInt14LE()
{
	int r;

	if(m_pCur - m_pBuf <= m_nSize - 2)
		r = (m_pCur[1] << 7) | m_pCur[0];
	else
		r = 0;
	m_pCur += 2;
	TEST_OVERFLOW();
	return r;
}

int CPsMidiReader::GetRemain()
{
	return m_nSize - (m_pCur - m_pBuf);
}
