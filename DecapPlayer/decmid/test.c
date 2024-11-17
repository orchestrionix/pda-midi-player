#include <windows.h>
#include <stdio.h>
#include "decmid.h"


int main(int argc, char* argv[])
{
	unsigned int srcLen;
	unsigned char *srcData;
	unsigned int dstLen;
	unsigned char *dstData;
	int ret;
	FILE *fout;
	FILE *fin;
	int user_id;

	user_id = atoi(argv[3]);

	fin = fopen(argv[1], "rb");
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
	
	ret = DecapMidi_Decrypt(srcData, srcLen, user_id, NULL, &dstLen);
	if(ret != DECMID_ERR_NONE)
	{
		free(srcData);
		printf("Error.\n");
		return 1;
	}
	
	dstData = (unsigned char*)malloc(dstLen);
	DecapMidi_Decrypt(srcData, srcLen, user_id, dstData, &dstLen);

	free(srcData);

	if(ret != DECMID_ERR_NONE)
	{
		free(dstData);
		printf("Error.\n");
		return 1;
	}

	fout = fopen(argv[2], "wb");
	if(!fout)
	{
		printf("Can't create %s.\n", argv[2]);
		return 1;
	}
	fwrite(dstData, dstLen, 1, fout);//decompressed size
	fclose(fout);
	free(dstData);
	printf("Done.\n");
	return 0;
}
