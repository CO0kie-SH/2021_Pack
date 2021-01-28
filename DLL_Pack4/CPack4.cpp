#include "pch.h"
#include "CPack4.h"


BOOL FixREL(DWORD Base, DWORD ModBase, DWORD RVA, DWORD dwTEXT)
{
	// ����DLL�������Ѿ����ص��ڴ��еģ��������е�ȫ�ֱ��������ض�λ��
	//	��DLLĿǰ�ļ��ػ�ַ��ͨ����(0x7XXXXXXX)�������ǽ�DLL������PE
	//	�ļ��к����ڼ��ػ�ַ�����λ�ַ���ñ��ˣ�������Ҫ�����޸�

	// 0. ��ȡ��Ҫʹ�õ��ֶ�
	DWORD OldImageBase = Base;		//(0x400000)/(0x1000000)
	DWORD NewImageBase = ModBase;	//�µ������ַ
	DWORD OldSectionBase = OldImageBase + dwTEXT;	//����ƫ��
	DWORD NewSectionBase = NewImageBase + dwTEXT;	//����ƫ��
	//DWORD dwNum = 0, dwOld = 0;

	// 1. ͨ�� DLL ������Ŀ¼�� RVA ���㵽�ض�λ��ĵ�ַ
	auto RelocTable = (PIMAGE_BASE_RELOCATION)
		(RVA + NewImageBase);
	// 2. �����ض�λ��(ÿһ���ض�λ��)������������ȫ0�Ľṹ
	while (RelocTable->SizeOfBlock)
	{

		typedef struct _TYPEOFFSET
		{
			USHORT Offset : 12;
			USHORT Type : 4;
		}TYPEOFFSET, * PTYPEOFFSET;

		// 3. ������ض�λ���������ʼλ���Լ������Ԫ�ظ���
		PTYPEOFFSET TypeOffset = (PTYPEOFFSET)(RelocTable + 1);
		DWORD Count = (RelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
		//printf("%lu\t%lX\t%lu\n", ++dwNum,
		//	RelocTable->VirtualAddress, Count);
		//VirtualProtect((LPVOID)(NewImageBase + RelocTable->VirtualAddress), 0x1000, PAGE_EXECUTE_READWRITE, &dwOld);

		// 4. ����ÿһ���ض�λ��ҵ���������Ϊ 3 ����
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

CPack4::CPack4()
{
	API = gAPI;
	mHeap = API->gHeap;
	oldBase = (DWORD)API->pGetModuleHandleA(0);
	auto pNT = NtHeader(oldBase);
	mOH = &pNT->OptionalHeader;

	//��ѹ������
	oldAddr = (LPCH)oldBase + mOH->BaseOfCode;
	MyLz4 lz4 = { FALSE,oldAddr };
	if (Lz4Compress(&lz4))
	{
		newBase = (DWORD)lz4.newAddr;
		UnOldBase();
	}
}

CPack4::~CPack4()
{
	API->pExitProcess(0);
}

BOOL CPack4::UnOldBase()
{
	auto pNTnew = NtHeader(newBase);
	auto pOHnew = &pNTnew->OptionalHeader;
	auto pSECnew = IMAGE_FIRST_SECTION(pNTnew);
	auto pSECold = IMAGE_FIRST_SECTION(NtHeader(oldBase));

	DWORD RVA = 0;
	LPCH oldBase = (LPCH)this->oldBase, newBase = (LPCH)this->newBase;

	//��ԭ����
	API->pmemset(oldBase + pSECold->VirtualAddress, 0, pSECold->Misc.VirtualSize);
	for (DWORD i = pNTnew->FileHeader.NumberOfSections; i >0; )
	{
		auto& SEC = pSECnew[--i];
		LPCH VA = oldBase + SEC.VirtualAddress,
			FOA = newBase + SEC.PointerToRawData;
		API->pmemcpy(VA, FOA, SEC.SizeOfRawData);
	}
	// �ض�λ��
	RVA = pOHnew->DataDirectory[5].VirtualAddress;
	if (!FixREL(pOHnew->ImageBase, this->oldBase, RVA, pSECold->VirtualAddress))	return 0;

	// IAT��
	RVA = pOHnew->DataDirectory[1].VirtualAddress;
	if (!FixIAT(RVA))	return 0;

	// �����µ�OEP
	oldBase += pOHnew->AddressOfEntryPoint;
	_asm call oldBase;
	return (int)oldBase;
}

BOOL CPack4::FixIAT(DWORD RVA)
{
	DWORD ImageBase = oldBase;
	HANDLE hHeap = mHeap;
	// �޸�IATʵ���Ͼ��ǣ����������������DLL����ȡ������ַ�����IAT
	auto ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)(ImageBase + RVA);
	// ����������������һ��ȫ0 �Ľṹ��β
	while (ImportTable->Name != 0)
	{
		// �����������ӦDLL�����ƣ���ͨ��LoadLibrary���м���
		CHAR* DllName = (CHAR*)(ImportTable->Name + ImageBase);
		HMODULE hMdule = API->pLoadLibraryA(DllName);
		
		// ����ʧ���򷵻�
		if (!hMdule)	return 0;

		// ��ȡ�� IAT �ĵ�ַ���������Ӧ����Ż�����
		auto IAT = (PIMAGE_THUNK_DATA)(ImportTable->FirstThunk + ImageBase);

		// ���� IAT ���ж����б���ĺ�������ŵĻ������Ƶ�
		while (IAT->u1.Function)
		{
			DWORD OldProtect = 0, FunctionAddr = 0;
			API->pVirtualProtect(IAT, 4, PAGE_READWRITE, &OldProtect);

			// ������λΪ0����ô����ľ������ƽṹ���RVA
			if ((IAT->u1.Function & 0x80000000) == 0)
			{
				auto Name = (PIMAGE_IMPORT_BY_NAME)(IAT->u1.Function + ImageBase);
				FunctionAddr = (DWORD)API->pGetProcAddress(hMdule, Name->Name);
			}
			else
			{
				// ���û�����֣���ô��16λ����ľ������
				FunctionAddr = (DWORD)API->pGetProcAddress(hMdule, (LPCSTR)(IAT->u1.Ordinal & 0xFFFF));
			}

			// ���ܺ�����ַ�������ｫ������ַ - 1
			FunctionAddr -= 1;

			// ����һ��ռ䣬���ڱ�����ܵĴ���
			LPCH Addr = (LPCH)API->pHeapAlloc(hHeap, 0, 10);
			if (!Addr)	return 0;

			// �滻���
			API->pmemcpy(Addr, OpCode, 10);
			*(LPDWORD)&Addr[1] = FunctionAddr;
			IAT->u1.Function = (DWORD)Addr;

			API->pVirtualProtect(IAT, 4, OldProtect, &OldProtect);
			API->pVirtualProtect(Addr, 10, PAGE_EXECUTE_READWRITE, &OldProtect);
			IAT++;
		}

		ImportTable++;
	}
	return TRUE;
}

PIMAGE_NT_HEADERS CPack4::NtHeader(DWORD pMem)
{
	LPDWORD pAdd = ((LPDWORD)pMem + 15);
	return (PIMAGE_NT_HEADERS)(pMem + *pAdd);
}
