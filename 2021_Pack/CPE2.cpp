#include "CPE2.h"

CPE2::CPE2(LPCBYTE pFile)
{
	this->pFile = (DWORD)pFile;
	mpNT = NtHeader(pFile);
	if (mpNT->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBoxA(NULL, "这不是一个有效的PE文件", "错误", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}
	mpOH = &mpNT->OptionalHeader;
	pPack = (HMODULE)GetBase();
}

CPE2::~CPE2()
{
}

BOOL CPE2::Lz4Mem()
{
	
	return 0;
}

PIMAGE_NT_HEADERS CPE2::NtHeader(LPCBYTE pFile)
{
	PLONG pAdd = ((PLONG)pFile + 15);
	return (PIMAGE_NT_HEADERS)(pFile + *pAdd);
}