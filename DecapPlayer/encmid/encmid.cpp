// encmid.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ucl\ucl.h"

#define DEST_EXT TEXT(".dmi")

int calcKey(int userId, int fileLen)
{
	int ret;
	ret = (fileLen ^ 0x304d5751) * 211 ^ userId * 11;

	return ret % 19973;
}

void encrypt(unsigned char *pBuf, int len, int key)
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

int main(int argc, char* argv[])
{
	TCHAR dstfn[MAX_PATH];
	
	if(argc == 3)
	{
		TCHAR fn[MAX_PATH];
		LPTSTR pf;
		if(GetFullPathName(argv[1], MAX_PATH, fn, &pf) == 0)
		{
			printf("Invalid source file.\n");
			return 1;
		}
		
		//remove extension name
		LPTSTR p = pf, pext = NULL;
		while(*p)
		{
			if(*p == TEXT('.'))
				pext = p;
			++p;
		}
		if(pext)
			*pext = 0;

		lstrcpy(dstfn, pf);
		lstrcat(dstfn, DEST_EXT);
	}
	else
	{
		printf("encmid source user_id\n");
		return 1;
	}

	unsigned int srcLen;
	unsigned char *srcData;
	FILE *fin = fopen(argv[1], "rb");
	if(!fin)
	{
		printf("Can't open source file.\n");
		return 1;
	}
	fseek(fin, 0, SEEK_END);
	srcLen = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	srcData = (unsigned char*)malloc(srcLen);
	if(fread(srcData, 1, srcLen, fin) != srcLen)
	{
		printf("Can't read source file.\n");
		fclose(fin);
		free(srcData);
		return 1;
	}
	fclose(fin);

	unsigned int compressDstLen = srcLen * 12 / 10;
	BYTE *compressDstData = (BYTE*)malloc(compressDstLen);

	ucl_nrv2e_99_compress(srcData, srcLen, compressDstData, &compressDstLen, NULL, 9, NULL, NULL);

	free(srcData);

	int uid = atoi(argv[2]);
	encrypt(compressDstData, compressDstLen, uid);
	int key = calcKey(uid, srcLen);

	FILE *fout = fopen(dstfn, "wb");
	if(!fout)
	{
		printf("Can't create %s.\n", dstfn);
		return 1;
	}
	fwrite("DMID", 4, 1, fout);//magic number
	int ver = 0x10000;
	fwrite(&ver, 4, 1, fout);//version
	fwrite(&key, 4, 1, fout);//key
	fwrite(&srcLen, 4, 1, fout);//decompressed size
	fwrite(compressDstData, compressDstLen, 1, fout);//compressed data
	fclose(fout);

	free(compressDstData);

	return 0;
}

