#pragma once
#include <windows.h>

class CMyPack
{
public:
	CMyPack();
	~CMyPack();

private:
	void Go();
	DWORD mEbp;
	DWORD mBase;
};

