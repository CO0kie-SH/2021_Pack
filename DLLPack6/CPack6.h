#pragma once
#include "CData6.h"

class CPack6
{
public:
	NOINLine CPack6();
	NOINLine ~CPack6();

	NOINLine BOOL LoadEXE6();
	NOINLine BOOL FixIAT(DWORD RVA);
private:
	LPCH pFile;
	DWORD nowBase;
};
