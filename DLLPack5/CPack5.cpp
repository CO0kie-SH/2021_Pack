#include "pch.h"
#include "CPack5.h"


CPack5::CPack5(CData5* pData5)
{
	CMyFile cFile;
	this->nowBase = (DWORD)pData5->exeBase;
	this->K32s = &pData5->K32s;
	this->NTDLLs = &pData5->NTs;

	fileBase = cFile.ReadOldEXE();
	if (fileBase == 0)	return;

	LoadEXE();
}

CPack5::~CPack5()
{
	__asm
	{
		xor eax, eax;
		push eax;
		mov eax, ExitProcess;
		call eax;
	}
}


BOOL CPack5::UnOldBase()
{
	
	return TRUE;
}


BOOL CPack5::LoadEXE()
{
	// �ж��ļ��Ƿ�Ϸ�
	auto pBase = (PIMAGE_DOS_HEADER)this->fileBase;
	if (pBase == 0 ||
		pBase->e_magic != IMAGE_DOS_SIGNATURE ||
		NtHeader(pBase)->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	// ��ʼ��ԭ�ļ�����
	auto fNT = NtHeader(pBase);
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

NOINLine BOOL CPack5::FixIAT(DWORD RVA)
{
	DWORD ImageBase = nowBase;
	HANDLE hHeap = GetProcessHeap();
	// �޸�IATʵ���Ͼ��ǣ����������������DLL����ȡ������ַ�����IAT
	auto ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)(ImageBase + RVA);
	// ����������������һ��ȫ0 �Ľṹ��β
	while (ImportTable->Name != 0)
	{
		// �����������ӦDLL�����ƣ���ͨ��LoadLibrary���м���
		CHAR* DllName = (CHAR*)(ImportTable->Name + ImageBase);
		HMODULE hMdule = LoadLibraryA(DllName);

		// ����ʧ���򷵻�
		if (!hMdule)	return 0;

		// ��ȡ�� IAT �ĵ�ַ���������Ӧ����Ż�����
		auto IAT = (PIMAGE_THUNK_DATA)(ImportTable->FirstThunk + ImageBase);

		// ���� IAT ���ж����б���ĺ�������ŵĻ������Ƶ�
		while (IAT->u1.Function)
		{
			DWORD OldProtect = 0, FunctionAddr = 0;
			VirtualProtect(IAT, 4, PAGE_READWRITE, &OldProtect);

			// ������λΪ0����ô����ľ������ƽṹ���RVA
			if ((IAT->u1.Function & 0x80000000) == 0)
			{
				auto Name = (PIMAGE_IMPORT_BY_NAME)(IAT->u1.Function + ImageBase);
				FunctionAddr = (DWORD)GetProcAddress(hMdule, Name->Name);
			}
			else
			{
				// ���û�����֣���ô��16λ����ľ������
				FunctionAddr = (DWORD)GetProcAddress(hMdule, (LPCSTR)(IAT->u1.Ordinal & 0xFFFF));
			}

			// ���ܺ�����ַ�������ｫ������ַ - 1
			FunctionAddr -= 1;

			// ����һ��ռ䣬���ڱ�����ܵĴ���
			LPCH Addr = (LPCH)HeapAlloc(hHeap, 0, 10);
			if (!Addr)	return 0;

			// �滻���
			memcpy(Addr, OpCode, 10);
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
