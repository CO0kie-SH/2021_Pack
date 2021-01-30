#include "CPE6.h"


CPE6::CPE6()
{
	auto Base = (HMODULE)GetBaseDLL();
	char PathDLL[MAX_PATH];
	GetModuleFileNameA(Base, PathDLL, MAX_PATH);

	// ��ȡ�ļ�
	DWORD SizeDll, SizeEXE;
	auto pMemDLL = (LPBYTE)ReadFile6(PathDLL, &SizeDll);
	auto pMemEXE = (LPBYTE)ReadFile6("..\\Debug\\demo.exe", &SizeEXE);
	if (!pMemDLL || !pMemEXE)	return;

	// �����ļ�
	auto pXorMem = (PBYTE)HeapAlloc(GetProcessHeap(), 0, SizeEXE);
	if (pXorMem == 0)	return;


	XorMem6(pMemEXE, pXorMem, SizeEXE, 0x34333231);
	

	// ѹ���ļ�
	MyLz4 lz4 = {
		TRUE,(LPCH)pXorMem,0,SizeEXE
	};
	BOOL bCom = Lz4Compress6(&lz4);
	if (!bCom)	return;

	// ��������ļ�����
	SaveFile6("..\\Debug\\demo.xor.lz4",
		lz4.newAddr, lz4.newSize + 8);

	SetPE(pMemDLL, pMemEXE, &lz4);
}

CPE6::~CPE6()
{
}

BOOL CPE6::SetPE(LPBYTE pDLL, LPBYTE pEXE, LPMyLz4 lz4)
{
	// ����Ϣ
	auto dNT = NtHeader((DWORD)pDLL);
	auto dOH = &dNT->OptionalHeader;
	auto& dDir = dOH->DataDirectory;
	auto dSEC = IMAGE_FIRST_SECTION(dNT);
	// ԭ�ļ���Ϣ
	auto fNT = NtHeader((DWORD)pEXE);
	auto fOH = &fNT->OptionalHeader;

	DWORD dwfSEC = fNT->FileHeader.NumberOfSections,
		dllSize = 0x7000 - dOH->BaseOfCode,
		newHead = fOH->SizeOfHeaders,
		//�´�С = ͷ��С + �Ǿ����С + 8�� + ѹ����
		newSize = newHead + dllSize + 8 + lz4->newSize,
		//������ = �´�С - ͷ��С
		newTEXT = newSize - newHead;
	

	// ����ռ�
	auto pNew = (LPCH)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, newSize),
		pHeap = pNew;
	if (pNew == 0)	return 0;
	
	memcpy(pNew, pEXE, fOH->SizeOfHeaders);
	pNew += fOH->SizeOfHeaders;
	for (DWORD i = dNT->FileHeader.NumberOfSections; i--;)
	{
		auto& SEC = dSEC[i];
		LPCH VA = pNew + SEC.VirtualAddress - dOH->BaseOfCode,
			RVA = (PCH)pDLL + SEC.PointerToRawData;
		memcpy(VA, RVA, SEC.SizeOfRawData);
	}
	pNew += dllSize;
	memcpy(pNew, lz4->newAddr, lz4->newSize + 8);

	auto nNT = NtHeader((DWORD)pHeap);
	auto nOH = &nNT->OptionalHeader;
	auto& nDir = nOH->DataDirectory;
	auto nSEC = IMAGE_FIRST_SECTION(nNT);

	// �����Ϣ��
	ZeroMemory(nDir, sizeof(IMAGE_DATA_DIRECTORY)
		* IMAGE_NUMBEROF_DIRECTORY_ENTRIES);
	ZeroMemory(nSEC + 1, sizeof(IMAGE_SECTION_HEADER) * dwfSEC);
	memcpy(nDir + 1, dDir + 1, 8);
	nSEC->SizeOfRawData = newTEXT;
	nSEC->Misc.VirtualSize = newTEXT;
	nSEC->Characteristics = 0xE00000E0;

	// ������չ��С
	nNT->FileHeader.NumberOfSections = 1;
	nOH->BaseOfData = 0;
	nOH->SizeOfCode = newTEXT;
	nOH->SizeOfImage = nSEC->VirtualAddress + nSEC->Misc.VirtualSize;

	// ����OEP
	DWORD tmp = (DWORD)GetBaseDLL();
	tmp = (DWORD)GetProcAddress((HMODULE)tmp, "start") - tmp;
	nOH->AddressOfEntryPoint = tmp;
	//nOH->DllCharacteristics = 0x8100;

	PCCH savePath = "..\\Debug\\demo.6.exe";
	DeleteFileA(savePath);
	BOOL bSave = SaveFile6(savePath, pHeap, newSize);
	printf("���棺%s\n", bSave ? "�ɹ�" : "ʧ��");
	return bSave;
}
