// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#pragma comment(linker, "/merge:.data=.text") 
#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/merge:.idata=.text")
//#pragma comment(linker, "/merge:.reloc=.text")
#pragma comment(linker, "/section:.text,RWE")
#pragma comment(linker, "/section:.idata,RW")

#include "CPack5.h"

HMODULE ghModule = 0;

// 字符串
constexpr const char* gszGetProcAddress = "GetProcAddress";

PIMAGE_NT_HEADERS NtHeader(PIMAGE_DOS_HEADER Dos)
{
	return (PIMAGE_NT_HEADERS)(Dos->e_lfanew + (DWORD)Dos);
}

NOINLine BOOL /*__stdcall*/ FixLOC(DWORD OldImageBase, DWORD NewImageBase,
	DWORD RVA, DWORD newTEXT, DWORD MaxSize)
{
	DWORD oldTEXT = 0x1000, /*newTEXT = 0x1000,*/
		OldSectionBase = OldImageBase + oldTEXT,
		NewSectionBase = NewImageBase + newTEXT;
	//DWORD dwNum = 0, dwOld = 0;

	// 1. 通过 DLL 的数据目录表 RVA 计算到重定位表的地址
	auto RelocTable = (PIMAGE_BASE_RELOCATION)
		(RVA + NewImageBase);

	// 2. 遍历重定位表(每一个重定位块)，结束条件是全0的结构
	while (RelocTable->SizeOfBlock)
	{
		if (MaxSize && RelocTable->VirtualAddress > MaxSize)
			break;

		typedef struct _TYPEOFFSET
		{
			USHORT Offset : 12;
			USHORT Type : 4;
		}TYPEOFFSET, * PTYPEOFFSET;

		// 3. 计算出重定位项数组的起始位置以及数组的元素个数
		PTYPEOFFSET TypeOffset = (PTYPEOFFSET)(RelocTable + 1);
		DWORD Count = (RelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
		//printf("%lu\t%lX\t%lu\n", ++dwNum,
		//	RelocTable->VirtualAddress, Count);
		//VirtualProtect((LPVOID)(NewImageBase + RelocTable->VirtualAddress), 0x1000, PAGE_EXECUTE_READWRITE, &dwOld);

		// 4. 遍历每一个重定位项，找到其中类型为 3 的项
		for (DWORD i = 0; i < Count; ++i)
		{
			if (TypeOffset[i].Type == 3)
			{
				DWORD* addr = (DWORD*)(TypeOffset[i].Offset + RelocTable->VirtualAddress + NewImageBase);
				DWORD newAddr = *addr - OldSectionBase + NewSectionBase;
				*addr = newAddr;
				_asm nop;
			}
		}
		//VirtualProtect((LPVOID)(NewImageBase + RelocTable->VirtualAddress), 0x1000, dwOld, &dwOld);
		RelocTable = (PIMAGE_BASE_RELOCATION)((DWORD)RelocTable + RelocTable->SizeOfBlock);
	}
	return TRUE;
}

// 去除括号内所有函数的名称粉碎机制，方便后续的调用和修改
extern "C"
{
	__declspec(dllexport) void* GetBase()
	{
		return ghModule;
	}

	NOINLine __declspec(dllexport) PIMAGE_DOS_HEADER GetBase2(DWORD Base)
	{
		auto lBase = (PIMAGE_DOS_HEADER)
			(Base & 0xFFFFF000);
		while (true)
		{
			if (lBase->e_magic == IMAGE_DOS_SIGNATURE &&
				NtHeader(lBase)->Signature == IMAGE_NT_SIGNATURE)
				return lBase;
			lBase = (PIMAGE_DOS_HEADER)(((LPCH)lBase) - 0x1000);
		}
		return 0;
	}

	// 创建一个裸函数作为新的 OEP，不会生成任何的其他代码
	__declspec(dllexport) void start()
	{
		PIMAGE_DOS_HEADER Base = 0;
		__asm
		{
			//_asm nop _asm nop _asm nop _asm nop _asm nop;
			mov Base, edx;
		}

		if (Base)	//有入口，则开始
		{
			if (Base->e_magic != IMAGE_DOS_SIGNATURE)	//手动修复
			{
				Base = GetBase2((DWORD)Base);
			}
			FixLOC(0x10000000, (DWORD)Base, 0x4000, 0x1000, 0x2000);
		}
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
		new CPack5();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
