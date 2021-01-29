#include "pch.h"
#include "CData6.h"

HMODULE ghModule = 0;

DLLEXport NOINLine PIMAGE_NT_HEADERS NtHeader(DWORD Dos)
{
	return (PIMAGE_NT_HEADERS)(Dos +
		((PIMAGE_DOS_HEADER)Dos)->e_lfanew);
}