#ifndef __PSFILEREADER_H__
#define __PSFILEREADER_H__

#include "..\COMMON\PSObject.h"

class CPsFileReader : public CPsReader  
{
public:
	void Close();
	enum {
		F_READ = 1,
		F_WRITE = 2,
		F_CREATE = 4
	};

	unsigned int GetPosition();
	bool IsEof();
	unsigned int GetLength();
	unsigned int SetPosition(unsigned int pos, int ref);
	signed char ReadChar();
	short ReadShort();
	int ReadInt();
	int ReadIntBE();
	unsigned int ReadBlock(void* pBuf, unsigned int len);
	bool Open(const char *pFilename, int flag);
	bool Open(const unsigned short *pFilename, int flag);
	CPsFileReader();
	virtual ~CPsFileReader();

protected:
	HANDLE m_handle;
	int m_caps;
	int m_flag;
};

#endif
