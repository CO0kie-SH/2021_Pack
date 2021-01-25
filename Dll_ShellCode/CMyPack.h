#pragma once
#include <windows.h>

class CMyPack
{
public:
	CMyPack();
	~CMyPack();

private:
	void Go();
	DWORD GetBase(DWORD k32);

private:
	DWORD mEbp;
	DWORD mKer32;
	DWORD mPack;
	
};

