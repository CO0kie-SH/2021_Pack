#pragma once
#include <iostream>
#include "CData.h"


class CPack4
{
public:
	CPack4();
	~CPack4();

	BOOL UnOldBase();
	BOOL FixIAT(DWORD RVA);
protected:

	PIMAGE_NT_HEADERS NtHeader(DWORD pMem);
private:
	LPMyWAPI	API;
	DWORD	oldBase;
	DWORD	newBase;
	LPCH	oldAddr;
	PIMAGE_OPTIONAL_HEADER32 mOH;
};

