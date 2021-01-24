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
	if (mpFile)
		free(mpFile);
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
	mpFile = (LPBYTE)malloc(FileSize * sizeof(BYTE));
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
	return CheckPE(mpFile);					//��ʼ��PE
}

BOOL CPE::CheckPE(LPCBYTE pMem)
{
	auto pNT = NtHeader(pMem);				//PEͷ
	if (NtHeader(mpFile)->Signature != IMAGE_NT_SIGNATURE)
		return PrintStr("PEͷ����");


	auto& pFH = pNT->FileHeader;			//�ļ�ͷ
	printf("  �ļ�ͷ��\n���εĸ�����%u\n��չͷ��С��0x%X\n�ļ��ı�־��0x%X\n",
		pFH.NumberOfSections, pFH.SizeOfOptionalHeader, pFH.Characteristics);

	auto& pOH = pNT->OptionalHeader;
	printf("  ��չͷ��\nħ���֣�0x%X\n�����С��0x%X\n��ڵ㴦����0x%X��\n\n" \
		"�������ʼ��0x%X\n���ݶ���ʼ��0x%X\n�����С����0x%lX��\n" \
		"Ĭ�ϼ��ػ�ַ��0x%lX\n�ڴ���룺0x%lX\n�ļ����룺0x%lX\n" \
		"����ͷ����С��0x%lX\n����ƫ�ƣ�->0x%p\nDLL��־��0x%X\n",
		pOH.Magic, pOH.SizeOfCode, pOH.AddressOfEntryPoint,
		pOH.BaseOfCode, pOH.BaseOfData, pOH.SizeOfImage,
		pOH.ImageBase, pOH.SectionAlignment, pOH.FileAlignment,
		pOH.SizeOfHeaders, pMem + pOH.SizeOfHeaders, pOH.DllCharacteristics);

	//����PE�ṹ���ڴ��ַ
	mPE.NumberOfSections = &pFH.NumberOfSections;
	mPE.SizeOfHeaders = &pOH.SizeOfHeaders;
	mPE.SizeOfImage = &pOH.SizeOfImage;
	mPE.SectionAlignment = &pOH.SectionAlignment;
	mPE.FileAlignment = &pOH.FileAlignment;
	mPE.AddressOfEntryPoint = &pOH.AddressOfEntryPoint;
	return CheckSection(&mPE);
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
	PBYTE buff = (PBYTE)HeapAlloc(mHeap, HEAP_ZERO_MEMORY, 200);
	if (!buff)
		return PrintStr("�޷������ѿ���");
	memcpy(buff, "\x90\x90\x90\x90", 4);

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
		return PrintStr("�ļ��򿪴���");

	//д���ļ�
	DWORD dwLen = 0, dwLen2 = 0, dwRet = 0;
	WriteFile(FileHandle, (LPVOID)pPE->FileMemAddr, pPE->FileSize, &dwLen, 0);
	WriteFile(FileHandle, (LPVOID)buff, 0x200, &dwLen2, 0);
	CloseHandle(FileHandle);

	//�жϳɹ�
	dwRet = dwLen + dwLen2 == pPE->FileSize + 0x200;
	printf("  д���ļ���%s\n", dwRet ? "�ɹ�" : "ʧ��");
	CheckSection(pPE);
	return dwRet;
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
