#pragma once
#include <Windows.h>
#define NOINLine __declspec(noinline) 
#define DLLEXport __declspec(dllexport) 

//EXTERN_C DLLEXport void* GetBaseDLL();
extern DLLEXport NOINLine PIMAGE_NT_HEADERS NtHeader(DWORD Dos);
extern HMODULE ghModule;

class CData6
{
};

