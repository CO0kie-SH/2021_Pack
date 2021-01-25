#include "CPE.h"

BOOL PrintStr(LPCSTR Str, HANDLE FileHandle = 0)
{
	puts(Str);
	if (FileHandle)
		CloseHandle(FileHandle);
	return 0;
}

#pragma region ���ں���

CPE::CPE() :mpFile(0)
{
	mHeap = HeapCreate(0, 0, 0);
	ZeroMemory(&mPE, sizeof(MyPE));
}

CPE::~CPE()
{
	HeapDestroy(mHeap);
}


BOOL CPE::ReadFile(LPCSTR Path)
{
	// ��ֻ���ķ�ʽ��ָ�����ļ���Ҫ���ļ�����
	HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("�ļ��򿪴���");


	// ��ȡ�ļ��Ĵ�С
	DWORD dwRead, FileSize = GetFileSize(FileHandle, &dwRead);
	if (dwRead || FileSize < 0x40)	//DOSͷ
		return PrintStr("�ļ���С����", FileHandle);

	// ��ȡ�ļ�
	BYTE buff[0x40];
	if (::ReadFile(FileHandle, (LPVOID)buff, 0x40, &dwRead, NULL)
		== FALSE || dwRead != 0x40 ||
		buff[0] != 0x4D ||
		buff[1] != 0x5A) {
		return PrintStr("DOSͷ����", FileHandle);
	}

	// ����ѿռ�
	auto mpFile = (LPBYTE)HeapAlloc(mHeap, 0, FileSize * sizeof(BYTE));
	if(!mpFile)
		return PrintStr("�����ڴ����", FileHandle);

	// ��ȡ�ļ�
	memcpy(mpFile, buff, 0x40);
	if (::ReadFile(FileHandle, (LPVOID)(mpFile + 0x40), FileSize, &dwRead, NULL)
		== FALSE || dwRead != FileSize - 0x40)
		return PrintStr("�ļ���ȡ����", FileHandle);

	// �رվ������ֹй¶
	CloseHandle(FileHandle);
	printf("���ļ��ɹ����ļ���С��%lu��0x%p\n", FileSize, mpFile);

	//��ʼ��PE�ṹ��
	mPE = { FileSize,(DWORD)mpFile };
	return CheckPE(mpFile, &mPE);			//��ʼ��PE
}

BOOL CPE::CheckPE(LPCBYTE pMem, LPMyPE pPE)
{
	auto pNT = NtHeader(pMem);				//PEͷ
	if (pNT->Signature != IMAGE_NT_SIGNATURE)
		return PrintStr("PEͷ����");

	auto& pFH = pNT->FileHeader;			//�ļ�ͷ
	printf("  �ļ�ͷ��\n���εĸ�����%u\n��չͷ��С��0x%X\n�ļ��ı�־��0x%X\n",
		pFH.NumberOfSections, pFH.SizeOfOptionalHeader, pFH.Characteristics);

	auto& pOH = pNT->OptionalHeader;		//��չͷ
	printf("  ��չͷ��\nħ���֣�0x%X\n�����С��0x%X\n��ڵ㴦����0x%X��\n\n" \
		"�������ʼ��0x%X\n���ݶ���ʼ��0x%X\n�����С����0x%lX��\n" \
		"Ĭ�ϼ��ػ�ַ��0x%lX\n�ڴ���룺0x%lX\n�ļ����룺0x%lX\n" \
		"����ͷ����С��0x%lX\n����ƫ�ƣ�->0x%p\nDLL��־��0x%X\n",
		pOH.Magic, pOH.SizeOfCode, pOH.AddressOfEntryPoint,
		pOH.BaseOfCode, pOH.BaseOfData, pOH.SizeOfImage,
		pOH.ImageBase, pOH.SectionAlignment, pOH.FileAlignment,
		pOH.SizeOfHeaders, pMem + pOH.SizeOfHeaders, pOH.DllCharacteristics);

	//����PE�ṹ���ڴ��ַ
	pPE->NumberOfSections = &pFH.NumberOfSections;
	pPE->SizeOfHeaders = &pOH.SizeOfHeaders;
	pPE->SizeOfImage = &pOH.SizeOfImage;
	pPE->SectionAlignment = &pOH.SectionAlignment;
	pPE->FileAlignment = &pOH.FileAlignment;
	pPE->AddressOfEntryPoint = &pOH.AddressOfEntryPoint;
	return CheckSection(pPE);
}

BOOL CPE::CheckSection(LPMyPE pPE, PIMAGE_SECTION_HEADER* pOut)
{
	printf("  �����б����£�\n");

	//��ȡ����ͷ��
	auto pSEC = IMAGE_FIRST_SECTION(NtHeader((LPCBYTE)pPE->FileMemAddr));
	BYTE name[9] = {};
	for (WORD i = 0, max = *pPE->NumberOfSections; i < max; i++, ++pSEC)
	{
		if (pOut) continue;
		memcpy(name, pSEC->Name, 8);
		printf("%u\t%s\tVA��%08lX\tVSize��%08lX\t" \
			"RVA��%08lX\tRSize��%08lX\t������%lX\n",
			i + 1, name, pSEC->VirtualAddress, pSEC->Misc.VirtualSize,
			pSEC->PointerToRawData, pSEC->SizeOfRawData, pSEC->Characteristics);
	}
	if (pOut) *pOut = pSEC;
	--pSEC;

	// �������һ���εĴ�С
	DWORD dwSize = pSEC->PointerToRawData + pSEC->SizeOfRawData;
	return dwSize == pPE->FileSize;
}

BOOL CPE::AddSection(LPMyPE pPE, LPCSTR SavePath)
{
	if (pPE == 0)	pPE = &this->mPE;

	//��ʼ��������
	auto buff = (PBYTE)HeapAlloc(mHeap, HEAP_ZERO_MEMORY, 0x200);
	if (!buff)
		return PrintStr("�޷������ѿ���");
	memcpy(buff, "\xE9\x90\x90\x90\x90", 5);
	pPE->dwNewSECMemAddr = (DWORD)buff;
	pPE->dwNewSECSize = 0x200;

	//��ȡ����ͷ��
	auto pSEC = IMAGE_FIRST_SECTION(NtHeader((LPCBYTE)pPE->FileMemAddr));
	auto& pSEC_NEW = pSEC[*pPE->NumberOfSections];		//�õ��±�ͷ
	auto& pSEC_END = pSEC[*pPE->NumberOfSections - 1];	//�õ�ĩβ��ͷ

	//����ƫ��
	DWORD nSEC_RVA = pSEC_END.PointerToRawData + pSEC_END.SizeOfRawData;
	DWORD nSEC_VA = pSEC_END.VirtualAddress + pSEC_END.Misc.VirtualSize;
	nSEC_RVA = this->MathOffset(nSEC_RVA, *pPE->FileAlignment);
	nSEC_VA = this->MathOffset(nSEC_VA, *pPE->SectionAlignment);

	//����ƫ��
	pSEC_NEW.VirtualAddress = nSEC_VA;
	pSEC_NEW.PointerToRawData = nSEC_RVA;
	pSEC_NEW.Misc.VirtualSize = 0x200;
	pSEC_NEW.SizeOfRawData = 0x200;
	pSEC_NEW.Characteristics = 0xE00000E0;
	memcpy(pSEC_NEW.Name, ".CO0kie", 8);

	//�ӿǴ���
	if (!LoadDLL(pPE, &pSEC_NEW))
		return PrintStr("���ؿǲ��ʧ�ܡ�");

	//���þ�������
	*pPE->NumberOfSections = *pPE->NumberOfSections + 1;
	*pPE->SizeOfImage = pSEC_NEW.VirtualAddress + pSEC_NEW.Misc.VirtualSize;
	*pPE->AddressOfEntryPoint = pSEC_NEW.VirtualAddress;


	//�����ļ�
	if (SavePath)
		return SaveFile(SavePath, pPE, buff);
	return TRUE;
}

BOOL CPE::SaveFile(LPCSTR FilePath, LPMyPE pPE, PBYTE buff)
{
	// ��ֻ���ķ�ʽ��ָ�����ļ���Ҫ���ļ�����
	HANDLE FileHandle = CreateFileA(FilePath, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("���ļ��򿪴���");

	//д���ļ�
	DWORD dwLen = 0, dwLen2 = 0, dwRet = 0;
	WriteFile(FileHandle, (LPVOID)pPE->FileMemAddr, pPE->FileSize, &dwLen, 0);
	WriteFile(FileHandle, (LPVOID)pPE->dwNewSECMemAddr, pPE->dwNewSECSize, &dwLen2, 0);
	CloseHandle(FileHandle);

	//�жϳɹ�
	dwRet = dwLen + dwLen2 == pPE->FileSize + pPE->dwNewSECSize;
	printf("  д���ļ���%s\n", dwRet ? "�ɹ�" : "ʧ��");
	CheckSection(pPE);
	return dwRet;
}

BOOL CPE::LoadDLL(LPMyPE pPE, PIMAGE_SECTION_HEADER pNewSEC)
{
	// �Բ�ִ�� DllMain �ķ�ʽ�� Dll װ�ص��ڴ���
	auto Base = (LPCBYTE)LoadLibraryExA(DllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (!Base)
		return PrintStr("�������ʧ�ܡ�");

	//��ȡͷ����Ϣ
	auto pOH = NtHeader(Base)->OptionalHeader;
	auto pSEC = IMAGE_FIRST_SECTION(NtHeader(Base));
	
	//����������
	if (pPE->dwNewSECMemAddr)
		HeapFree(mHeap, 0, (LPVOID)pPE->dwNewSECMemAddr);

	DWORD dwSize = pSEC->Misc.VirtualSize;
	auto pMem = (LPBYTE)HeapAlloc(mHeap, 0, dwSize);
	auto pDLL = Base + pSEC->VirtualAddress;
	if (!pMem || dwSize < 0x200)
		return PrintStr("���������ζѿռ����");
	memcpy(pMem, pDLL, pSEC->Misc.VirtualSize);

	//��������ƫ��
	DWORD start = (DWORD)GetProcAddress((HMODULE)Base, "start") - (DWORD)Base;
	start -= pSEC->VirtualAddress + 27;
	memcpy(pMem + 15, &pNewSEC->VirtualAddress, 4);
	memcpy(pMem + 23, &start, 4);

	//����PE�ṹ
	pPE->dwNewSECMemAddr = (DWORD)pMem;
	pPE->dwNewSECSize = dwSize;
	pNewSEC->Misc.VirtualSize = dwSize;
	pNewSEC->SizeOfRawData = dwSize;
 	return TRUE;
}

DWORD CPE::MathOffset(DWORD Addr, DWORD Size)
{
	DWORD dw = Addr / Size;
	if (Addr % Size == 0)
		return dw*Size;
	return dw*Size + Size;
}

PIMAGE_NT_HEADERS CPE::NtHeader(LPCBYTE pFile)
{
	PLONG pAdd = ((PLONG)pFile + 15);
	return (PIMAGE_NT_HEADERS)(pFile + *pAdd);
}

#pragma endregion
