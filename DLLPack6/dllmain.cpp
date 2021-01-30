// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "CPack6.h"

#pragma comment(linker, "/merge:.data=.text") 
#pragma comment(linker, "/merge:.rdata=.text")
#pragma comment(linker, "/section:.text,RWE")

void _stdcall FusedFunc(DWORD funcAddress)
{
	_asm
	{
		jmp label1
		label2 :
		_emit 0xeb; //跳到下面的call
		_emit 0x04;
		CALL DWORD PTR DS : [EAX + EBX * 2 + 0x123402EB] ; //执行EB 02  也就是跳到下一句

														  //	call Init;// 获取一些基本函数的地址

														  // call下一条,用于获得eip
		_emit 0xE8;
		_emit 0x00;
		_emit 0x00;
		_emit 0x00;
		_emit 0x00;
		//-------跳到下面的call
		_emit 0xEB;
		_emit 0x0E;

		//-------花
		PUSH 0x0;
		PUSH 0x0;
		MOV EAX, DWORD PTR FS : [0] ;
		PUSH EAX;
		//-------花


		// fused:
		//作用push下一条语句的地址
		//pop eax;
		//add eax, 0x1b;
		/*push eax;*/
		CALL DWORD PTR DS : [EAX + EBX * 2 + 0x5019C083] ;

		push funcAddress; //这里如果是参数传入的需要注意上面的add eax,??的??
		retn;

		jmp label3

			// 花
			_emit 0xE8;
		_emit 0x00;
		_emit 0x00;
		_emit 0x00;
		_emit 0x00;
		// 花


	label1:
		jmp label2
			label3 :
	}
}

inline __declspec(naked) void RbCodeMain()
{
	__asm
	{
		push eax;
		call label1 - 1;
	label1:
		ret;
		pop eax;
		pop eax;
		jmp tag_Next;
		ret;
	tag_Next:
		jmp label1;
		ret;
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

	NOINLine DWORD GetBase6(DWORD Base)
	{
		DWORD lBase;
		if (!Base)
		__asm
		{
			call tag_esp;
		tag_esp:
			pop eax;
			mov Base, eax;
		}
		lBase = Base & 0xFFFFF000;
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
		DWORD dwEAX, dwEDI, dwEDX;
		_asm mov dwEAX, eax;
		_asm mov dwEDI, edi;
		_asm mov dwEDX, edx;
		__asm
		{
			push eax;
			call label1 - 1;
		label1:
			ret;
			pop eax;
			pop eax;
			jmp tag_Next;
			ret;
		tag_Next:
		}
		RbCodeMain();
		if (dwEAX == 0x10000000 && dwEDI == 0x1)
			return;		//Release调试
		if (dwEAX && dwEDX == 0x1)
			return;		//Debug调试
		__asm
		{
			push eax;
			call label2 - 1;
		label2:
			ret;
			pop eax;
			pop eax;
			jmp tag_Next2;
			ret;
		tag_Next2:
		}
		auto nowBase = GetBase6(0);

		FixLOC(0x10000000, nowBase, 0x6000, 0x1000, 0x6000);
		CPack6 pack6;
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

