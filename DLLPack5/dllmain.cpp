// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#pragma comment(linker, "/merge:.data=.text") 
#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/section:.rdata,RW")
#pragma comment(linker, "/section:.text,RWE")

#include "CPack5.h"

HMODULE ghModule = 0;

EXTERN_C DLLEXport NOINLine void go()
{
	CData5 data5;
	CPack5 pack5(&data5);
}



// 获取当前ESP
__declspec(naked) DWORD GetESP()
{
	__asm
	{
		mov eax, [esp];
		ret
	}
}


// 去除括号内所有函数的名称粉碎机制，方便后续的调用和修改
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

	__declspec(dllexport) void* GetBase()
	{
		return ghModule;
	}

	NOINLine __declspec(dllexport) PIMAGE_DOS_HEADER GetBase2(DWORD Base)
	{
		auto lBase = (PIMAGE_DOS_HEADER)
			(Base & 0xFFFFF000);
		while ((DWORD)lBase > 0x1000)
		{
			if (lBase->e_magic == IMAGE_DOS_SIGNATURE &&
				NtHeader(lBase)->Signature == IMAGE_NT_SIGNATURE)
				return lBase;
			lBase = (PIMAGE_DOS_HEADER)(((DWORD)lBase) - 0x1000);
		}
		return 0;
	}

	// 导出入口
	NOINLine __declspec(dllexport) void start()
	{
		PIMAGE_DOS_HEADER Base;
		DWORD dwEAX, dwEDI;
		__asm
		{
			mov Base, edx;
			mov dwEAX, eax;
			mov dwEDI, edi;
		}
		if (Base == 0 && dwEAX == 0x10000000 && dwEDI == 0x1)
			return;
		//if (dwType == 0x10000000 && dwEDI == 0x1)	//这是加壳exe调用的DLL
		//	TextDLL();
		//if (dwType == 0x1 ||
		//	(DWORD)Base == 0x1)
		//{
		//	
		//	/*Base = (PIMAGE_DOS_HEADER)GetBase2(GetESP());
		//	FixLOC(0x10000000, (DWORD)Base, 0x5000, 0x1000, 0x5000);*/
		//}
		auto nowBase = (DWORD)GetBase2(GetESP()),
			newBase = (DWORD)VirtualAlloc(NULL, 0x6000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (newBase == 0)	return;
		memcpy((LPCH)newBase, (LPCH)nowBase, 0x6000);
		FixLOC(0x10000000, newBase, 0x5000, 0x1000, 0x5000);
		newBase = (DWORD)go - nowBase + newBase;
		_asm call newBase;
		ExitProcess(0);
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
		ghModule = hModule;
		start();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
