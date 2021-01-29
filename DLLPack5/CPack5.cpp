#include "pch.h"
#include "CPack5.h"


CPack5::CPack5()
{
	CMyFile cFile;
	nowBase = (DWORD)GetModuleHandleA(0);
	fileBase = cFile.ReadOldEXE();
}