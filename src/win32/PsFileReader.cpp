#include "PSFileReader.h"

CPsFileReader::CPsFileReader()
{
	m_handle = INVALID_HANDLE_VALUE;
}

CPsFileReader::~CPsFileReader()
{
	if(m_handle != INVALID_HANDLE_VALUE)
		CloseHandle(m_handle);
}

bool CPsFileReader::Open(const char *pFilename, int flag)
{
#ifdef UNICODE
	TCHAR s[MAX_PATH];
	MultiByteToWideChar(CP_OEMCP, 0, pFilename, -1, s, MAX_PATH);
	return Open(s, flag);
#else
	DWORD acc = 0, d;

	if(flag & F_READ)
		acc |= GENERIC_READ;
	if(flag & (F_WRITE | F_CREATE))
		acc |= GENERIC_WRITE;

	if(flag & F_CREATE)
		d = CREATE_ALWAYS;
	else
		d = OPEN_EXISTING;

	if(m_handle != INVALID_HANDLE_VALUE)
		CloseHandle(m_handle);
	m_handle = CreateFile(pFilename, acc, flag & F_WRITE?0:FILE_SHARE_READ, NULL, d, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_handle != INVALID_HANDLE_VALUE)
	{
		m_flag = flag;
		return true;
	}
	else
		return false;
#endif
}

bool CPsFileReader::Open(const unsigned short *pFilename, int flag)
{
	DWORD acc = 0, d;

	if(flag & F_READ)
		acc |= GENERIC_READ;
	if(flag & (F_WRITE | F_CREATE))
		acc |= GENERIC_WRITE;

	if(flag & F_CREATE)
		d = CREATE_ALWAYS;
	else
		d = OPEN_EXISTING;

	if(m_handle != INVALID_HANDLE_VALUE)
		CloseHandle(m_handle);
	m_handle = CreateFileW(pFilename, acc, flag & F_WRITE?0:FILE_SHARE_READ, NULL, d, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_handle != INVALID_HANDLE_VALUE)
	{
		m_flag = flag;
		return true;
	}
	else
		return false;
}

unsigned int CPsFileReader::ReadBlock(void *pBuf, unsigned int len)
{
	DWORD nr;
	ReadFile(m_handle, pBuf, len, &nr, NULL);
	return nr;
}

int CPsFileReader::ReadInt()
{
	int i = 0;
	DWORD nr;
	ReadFile(m_handle, &i, 4, &nr, NULL);
	return i;
}

int CPsFileReader::ReadIntBE()
{
	unsigned char c[4];
	DWORD nr;
	ReadFile(m_handle, c, 4, &nr, NULL);
	if(nr == 4)
		return (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | (c[3]);
	else
		return 0;
}

short CPsFileReader::ReadShort()
{
	short i = 0;
	DWORD nr;
	ReadFile(m_handle, &i, 2, &nr, NULL);
	return i;
}

signed char CPsFileReader::ReadChar()
{
	signed char i = 0;
	DWORD nr;
	ReadFile(m_handle, &i, 1, &nr, NULL);
	return i;
}

unsigned int CPsFileReader::SetPosition(unsigned int pos, int ref)
{
	DWORD m;

	if(ref == REF_BEGIN)
		m = FILE_BEGIN;
	else if(ref == REF_CURRENT)
		m = FILE_CURRENT;
	else
		m = FILE_END;
	return SetFilePointer(m_handle, pos, NULL, m);
}

unsigned int CPsFileReader::GetLength()
{
	return GetFileSize(m_handle, NULL);
}

bool CPsFileReader::IsEof()
{
	return GetPosition() == GetLength();
}

unsigned int CPsFileReader::GetPosition()
{
	return SetFilePointer(m_handle, 0, NULL, FILE_CURRENT);
}

void CPsFileReader::Close()
{
	if(m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}
