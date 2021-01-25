#include "CMyPack.h"
#define EMIT _asm _emit

__declspec(naked noinline) int Vjmp()
{
	__asm
	{
		push ebp;
		mov esi, [esp + 4];
		mov edi, [ebp + 4];
		mov ebx, esp;
		push edx;
		mov edx, edx;
		sub edx, 0x12345678;	//计算随机基址
		push edx;
		push esi;
		push edi;
	tag_Vjmp:
		EMIT(0xE9)EMIT(0x0C);	//跳至Start
	}
}

CMyPack::CMyPack()
{
	DWORD ker32, ntdll, Base, Pack, lEbp;
	__asm
	{
		mov lEbp, ebx;			//还原原栈底
		mov eax, [ebx - 4];		//还原壳区段
		mov Pack, eax;
	}
	this->mEbp = lEbp;
	Go();
}

CMyPack::~CMyPack()
{
	DWORD lEbp = this->mEbp;
	__asm
	{
		nop;
		nop;
		mov ebx, lEbp;
		mov esp, ebx;
		pop ebp;
		mov eax, [esp - 0xC];
		add eax, 0x12B2;
		jmp eax;
		nop;
	}
	Vjmp();
}

void CMyPack::Go()
{
	_asm nop;
	_asm nop;
}
