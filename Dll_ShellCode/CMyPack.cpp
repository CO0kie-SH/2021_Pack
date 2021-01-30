#include "CMyPack.h"
#define EMIT _asm _emit

DWORD dwPack = 0, dwOEP = 0;
EXTERN_C __declspec(dllexport) void setgdw(DWORD pack, DWORD oep)
{
	dwPack = pack;
	dwOEP = oep;
}

//__declspec(naked noinline) int Vjmp()
//{
//	__asm
//	{
//		push ebp;
//		mov esi, [esp + 4];
//		mov edi, [ebp + 4];
//		mov ebx, esp;
//		push edx;
//		mov edx, edx;
//		sub edx, 0x12345678;	//计算随机基址
//		push edx;
//		push esi;
//		push edi;
//	tag_Vjmp:
//		EMIT(0xE9)EMIT(0x0C);	//跳至Start
//	}
//}

CMyPack::CMyPack()
{
	DWORD ker32, Pack;
	__asm
	{
		mov [ecx], ebx;			//还原原栈顶
		mov eax, [ebx];
		mov ker32, eax;
		mov Pack, edx;
		nop;
	}
	mKer32 = GetBase(ker32);
	mPack = Pack;
	Go();
}

CMyPack::~CMyPack()
{
	__asm
	{
		mov esp, [ecx];
		mov eax, [ecx + 8];
		sub eax, dwPack;
		add eax, dwOEP;
		jmp eax;
	}
}

void CMyPack::Go()
{
	_asm nop;
	_asm nop;
}

DWORD CMyPack::GetBase(DWORD k32)
{
	__asm
	{
		mov eax, dword ptr fs : [0x30]
		mov eax, dword ptr[eax + 0x0C]
		mov eax, dword ptr[eax + 0x0C]
		mov eax, dword ptr[eax]
		mov eax, dword ptr[eax]
		mov eax, dword ptr[eax + 0x18]
		ret;
	}
	return 0;
}
