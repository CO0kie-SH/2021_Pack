// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#define NOINLine __declspec(noinline) 
#define DLLEXport __declspec(dllexport) 

#pragma comment(linker, "/merge:.data=.text") 
//#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/section:.rdata,RW")
#pragma comment(linker, "/section:.text,RWE")


extern "C"
{
	NOINLine BOOL /*__stdcall*/ FixLOC(DWORD OldImageBase, DWORD NewImageBase,
		DWORD RVA, DWORD newTEXT, DWORD MaxSize)
	{
		DWORD oldTEXT = 0x1000, /*newTEXT = 0x1000,*/
			OldSectionBase = OldImageBase + oldTEXT,
			NewSectionBase = NewImageBase + newTEXT;
		auto RelocTable = (PIMAGE_BASE_RELOCATION)
			(RVA + NewImageBase);

		if (RelocTable->VirtualAddress == 0)	return 0;
		while (RelocTable->SizeOfBlock)
		{
			if (MaxSize && RelocTable->VirtualAddress >= MaxSize)
				break;

			typedef struct _TYPEOFFSET
			{
				USHORT Offset : 12;
				USHORT Type : 4;
			}TYPEOFFSET, * PTYPEOFFSET;

			PTYPEOFFSET TypeOffset = (PTYPEOFFSET)(RelocTable + 1);
			DWORD Count = (RelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;

			for (DWORD i = 0; i < Count; ++i)
			{
				if (TypeOffset[i].Type == 3)
				{
					DWORD* addr = (DWORD*)(TypeOffset[i].Offset + RelocTable->VirtualAddress + NewImageBase);
					DWORD newAddr = *addr - OldSectionBase + NewSectionBase;
					*addr = newAddr;
				}
			}
			RelocTable = (PIMAGE_BASE_RELOCATION)((DWORD)RelocTable + RelocTable->SizeOfBlock);
		}
		return TRUE;
	}

	// 导出入口
	NOINLine __declspec(dllexport) void start()
	{
		MessageBoxA(0, "CO0kie", 0, 0);
	}
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		start();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

