#include "pch.h"
#include "CPack6.h"
#include <iostream>

CPack6::CPack6()
	:nowBase(0), pFile(0)
{
	CDbg6 dbg6;
	CWind6 wind6;
	LPCH base = (LPCH)GetModuleHandleA(0);
	printf("��ǰ�����ַ��0x%p\n", base);

	MyLz4 lz4 = { FALSE,base + 0x7000 };

	DWORD key1 = 1, key2 = 2,
		bRet = Lz4Compress6(&lz4);
	if (bRet == 0)	return;

	key2 = wind6.Create();
	XorMem6((PBYTE)lz4.newAddr, (PBYTE)lz4.newAddr, lz4.newSize, key2);
	//TextAES(FALSE, (PBYTE)lz4.newAddr, lz4.newSize, 0x34333231);

	this->pFile = lz4.newAddr;

	LoadEXE6();
}

CPack6::~CPack6()
{
	__asm
	{
		xor eax, eax;
		push eax;
		mov edx, ExitProcess;
		call edx;
	}
}

BOOL CPack6::LoadEXE6()
{
	CDbg6 dbg6;
	// �ж��ļ��Ƿ�Ϸ�
	auto pBase = (PIMAGE_DOS_HEADER)this->pFile;
	if (pBase == 0 ||
		pBase->e_magic != IMAGE_DOS_SIGNATURE ||
		NtHeader((DWORD)pBase)->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	// ��ʼ��ԭ�ļ�����
	auto fNT = NtHeader((DWORD)pBase);
	auto fOH = &fNT->OptionalHeader;
	auto fSECnew = IMAGE_FIRST_SECTION(fNT);
	nowBase = (DWORD)VirtualAlloc(NULL, fOH->SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (nowBase == 0)	return 0;

	// �����С
	LPCH nBase = (LPCH)this->nowBase, fBase = (LPCH)pBase;
	DWORD dwTEXT = fOH->BaseOfCode;

	// ��վ���
	//memset(nBase + dwTEXT, 0x0, fOH->SizeOfImage - dwTEXT);
	for (DWORD i = fNT->FileHeader.NumberOfSections; i > 0; )
	{
		auto& SEC = fSECnew[--i];
		LPCH VA = nBase + SEC.VirtualAddress,
			FOA = fBase + SEC.PointerToRawData;
		memcpy(nBase + SEC.VirtualAddress,	//VA
			fBase + SEC.PointerToRawData,	//FOA
			SEC.SizeOfRawData);				//RSize
	}

	// ���޸��ض�λ
	auto& RELOC = fOH->DataDirectory[5];
	if (!FixLOC(fOH->ImageBase, (DWORD)nBase,		//�ɡ��»�ַ
		RELOC.VirtualAddress, fOH->BaseOfCode, 0))	//RVA����ƫ��
		return 0;
	memset(nBase + RELOC.VirtualAddress, 0, RELOC.Size);

	// ���޸�IAT��
	if (!FixIAT(fOH->DataDirectory[1].VirtualAddress))	return 0;

	// �����µ�OEP
	nBase += fOH->AddressOfEntryPoint;
	//printf("�»�ַ��%08lX\n����ڣ�%p\n", nowBase, nBase);
	_asm call nBase;
	return TRUE;
}

BOOL CPack6::FixIAT(DWORD RVA)
{
	CDbg6 dbg6;
	DWORD ImageBase = nowBase;
	HANDLE hHeap = GetProcessHeap();	//��ȡĬ�϶�
	auto ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)(ImageBase + RVA);
	while (ImportTable->Name != 0)
	{
		CHAR* DllName = (CHAR*)(ImportTable->Name + ImageBase);
		HMODULE hMdule = LoadLibraryA(DllName);

		if (!hMdule)	return 0;

		auto IAT = (PIMAGE_THUNK_DATA)(ImportTable->FirstThunk + ImageBase);
		while (IAT->u1.Function)
		{
			DWORD OldProtect = 0, FunctionAddr = 0;
			VirtualProtect(IAT, 4, PAGE_READWRITE, &OldProtect);

			if ((IAT->u1.Function & 0x80000000) == 0)
			{
				auto Name = (PIMAGE_IMPORT_BY_NAME)(IAT->u1.Function + ImageBase);
				FunctionAddr = (DWORD)GetProcAddress(hMdule, Name->Name);
			}
			else
			{
				FunctionAddr = (DWORD)GetProcAddress(hMdule, (LPCSTR)(IAT->u1.Ordinal & 0xFFFF));
			}
			FunctionAddr -= 0x1234;

			LPCH Addr = (LPCH)HeapAlloc(hHeap, 0, 13);
			if (!Addr)	return 0;

			memcpy(Addr, gIAT, 13);
			*(LPDWORD)&Addr[1] = FunctionAddr;
			IAT->u1.Function = (DWORD)Addr;

			VirtualProtect(IAT, 4, OldProtect, &OldProtect);
			VirtualProtect(Addr, 10, PAGE_EXECUTE_READWRITE, &OldProtect);
			IAT++;
		}

		ImportTable++;
	}
	return TRUE;
}
