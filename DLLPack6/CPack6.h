#pragma once
#include "CWind6.h"

class CPack6
{
public:
	NOINLine CPack6();
	NOINLine ~CPack6();

	NOINLine BOOL LoadEXE6();
	NOINLine BOOL FixIAT(DWORD RVA);
	NOINLine BOOL CreateCMD();
private:
	LPCH pFile;
	int Size;
	DWORD nowBase;
};
