// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#define NOINLine __declspec(noinline) 
#define DLLEXport __declspec(dllexport) 

#pragma comment(linker, "/merge:.data=.text") 
//#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/section:.rdata,RW")
#pragma comment(linker, "/section:.text,RWE")

// 获取当前ESP
__declspec(naked) DWORD GetESP()
{
	__asm
	{
		mov eax, [esp];
		ret
	}
}

NOINLine PIMAGE_NT_HEADERS NtHeader(DWORD Dos)
{
	return (PIMAGE_NT_HEADERS)(Dos +
		((PIMAGE_DOS_HEADER)Dos)->e_lfanew);
}

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

	NOINLine DLLEXport DWORD GetBase6(DWORD Base)
	{
		DWORD lBase = 0;
		if (Base)
			lBase = Base & 0xFFFFF000;
		else
			__asm
		{
			call tag_esp;
		tag_esp:
			pop eax;
			mov lBase, lBase;
		}
		while (lBase > 0x1000)
		{
			if (
				((PIMAGE_DOS_HEADER)lBase)->e_magic == IMAGE_DOS_SIGNATURE &&
				NtHeader(lBase)->Signature == IMAGE_NT_SIGNATURE)
				return lBase;
			lBase -= 0x1000;
		}
		return 0;
	}

	// 导出入口
	NOINLine __declspec(dllexport) void start()
	{
		auto nowBase = GetBase6(GetESP()),
			newBase = (DWORD)VirtualAlloc(NULL, 0x6000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
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

