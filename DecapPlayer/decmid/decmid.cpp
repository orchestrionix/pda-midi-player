// decmid.cpp : Defines the entry point for the DLL application.
//
#include <windows.h>
#include "decmid.h"
#include "../resmgr/ucl/ucl_dcmp.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


int calcKey(int userId, int fileLen)
{
	int ret;
	ret = (fileLen ^ 0x304d5751) * 211 ^ userId * 11;
	
	return ret % 19973;
}

void decrypt(unsigned char *pBuf, int len, int key)
{
	int i;
	unsigned char magic[256];
	
	int tmp = key * 13;
	for(i = 0; i < 256; i++)
	{
		tmp = (tmp ^ 0x9a87df9f) * 13 % 0x54781 >> 2;
		magic[i] = (unsigned char)tmp;
	}
	
	for(i = 0; i < len; i++)
	{
		pBuf[i] ^= magic[i & 255];
	}
}

DECMID_API int _stdcall DecapMidi_Decrypt(const void* pIn, unsigned int inLenght, int userId, void* pOut, unsigned int* pOutLength)
{
	BYTE *pSrc = (BYTE*)pIn;
	if(inLenght < 16 || *(int*)pSrc != 0x44494d44 || *(int*)(pSrc + 4) != 0x10000)
	{
		return DECMID_ERR_UNSUPPORTED_FORMAT;
	}
	
	unsigned int midiSize = *(int*)(pSrc + 12);
	if(*(int*)(pSrc + 8) != calcKey(userId, midiSize))
	{
		return DECMID_INVALID_USER_ID;
	}
	
	if(pOut)
	{
		decrypt(pSrc + 16, inLenght - 16, userId);
		
		unsigned int ds;
		ucl_nrv2e_decompress_le32(pSrc + 16, inLenght - 16, (unsigned char*)pOut, &ds);
	}
	
	if(pOutLength)
		*pOutLength = midiSize;

	return DECMID_ERR_NONE;
}
