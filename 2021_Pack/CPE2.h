#pragma once

#include <iostream>
#include <Windows.h>


#include "..\DLL_Pack4\CData.h"
//#pragma comment(lib,"..\\Debug\\DLL_Pack4.lib")
//#pragma comment(lib,"..\\Release\\Dll1.lib")

BOOL PrintStr(LPCSTR Str, HANDLE FileHandle = 0);

class CPE2
{
public:
	CPE2(LPCBYTE pFile, DWORD Size);
	~CPE2();

	BOOL Lz4Mem();
private:
	PIMAGE_NT_HEADERS NtHeader(LPCBYTE pFile);
private:
	DWORD				pFile;
	DWORD				mFileSize;
	HMODULE				mpPack;
	PIMAGE_NT_HEADERS	mpNT;
	PIMAGE_OPTIONAL_HEADER32 mpOH;
};

