#pragma once
#include "CData5.h"

class CPack5
{
public:
	CPack5(CData5* pData5);
	~CPack5();

	NOINLine BOOL LoadEXE();
	NOINLine BOOL UnOldBase();
	NOINLine BOOL FixIAT(DWORD RVA);
private:
	DWORD nowBase;
	DWORD fileBase;
	LPMyKe32	K32s;
	LPMyNTDLL	NTDLLs;
};

