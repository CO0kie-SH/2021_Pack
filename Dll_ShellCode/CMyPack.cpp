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
		sub edx, 0x12345678;	//���������ַ
		push edx;
		push esi;
		push edi;
	tag_Vjmp:
		EMIT(0xE9)EMIT(0x0C);	//����Start
	}
}

CMyPack::CMyPack()
{
	DWORD ker32, ntdll, Base, Pack, lEbp;
	__asm
	{
		mov lEbp, ebx;			//��ԭԭջ��
		mov eax, [ebx - 4];		//��ԭ������
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
