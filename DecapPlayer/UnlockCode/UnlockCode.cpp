// UnlockCode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

DWORD ENCRYPT_MAGIC_NUMBER;

enum
{
	FEATURE_RS232 = 0x01,
	FEATURE_SOFTSYNTH = 0x02,
	FEATURE_MP3 = 0x04
};

int myxy(int a)
{
	return (a ^ 5878167 + (a >> 5)) / 5;
}

int Regist_GeneratePWD(int dwCode0, int dwSerial)
{
	int d;

	d=0x52415453 ^ dwCode0;
	d=(d*1139) ^ 0x4d5751;
	d=((d << (d & 0xf)) + 211) * 217 + 0x6d7771;

	for(int i = d & 7; i > 0; i--)
		d = (d ^ 3429) * 100 + dwSerial;
	return myxy(d)&0x4ffffff;
}

int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		printf("UnlockCode user_id serial [R|RS|RM|RSM]\n");
		return 1;
	}

	DWORD code = atoi(argv[1]) << 8;
	DWORD serial = atoi(argv[2]);
	
	char fs[64];
	lstrcpyn(fs, argv[3], 64);
	CharUpper(fs);
	if(strstr(fs, "R"))
		code |= FEATURE_RS232;
	if(strstr(fs, "S"))
		code |= FEATURE_SOFTSYNTH;
	if(strstr(fs, "M"))
		code |= FEATURE_MP3;

	ENCRYPT_MAGIC_NUMBER = 1139;
	printf("%08d-%08d\n", code, Regist_GeneratePWD(code, serial));
	return 0;
}
