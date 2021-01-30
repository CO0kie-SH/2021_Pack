#pragma once
#include <iostream>
#include "CData.h"


class CPack4
{
public:
	NOINLine CPack4();
	NOINLine ~CPack4();

	NOINLine BOOL UnOldBase();
	NOINLine BOOL FixIAT(DWORD RVA);
protected:

	NOINLine PIMAGE_NT_HEADERS NtHeader(DWORD pMem);
private:
	LPMyWAPI	API;
	DWORD	oldBase;
	DWORD	newBase;
	LPCH	oldAddr;
	HANDLE	mHeap;
	PIMAGE_OPTIONAL_HEADER32 mOH;
};

