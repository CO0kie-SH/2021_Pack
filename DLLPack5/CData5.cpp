#include "pch.h"
#include "CData5.h"


#pragma region ���⺯��
NOINLine PIMAGE_NT_HEADERS NtHeader(PIMAGE_DOS_HEADER Dos)
{
	return (PIMAGE_NT_HEADERS)(Dos->e_lfanew + (DWORD)Dos);
}

BOOL PrintStr(LPCSTR Str, HANDLE FileHandle)
{
	//puts(Str);
	if (FileHandle)
		CloseHandle(FileHandle);
	return 0;
}

BOOL ReadFile5(LPCSTR Path, DWORD* pFileSize)
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
	auto mpFile = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize * sizeof(BYTE));
	if (!mpFile)
		return PrintStr("�����ڴ����", FileHandle);

	// ��ȡ�ļ�
	memcpy(mpFile, buff, 0x40);
	if (::ReadFile(FileHandle, (LPVOID)(mpFile + 0x40), FileSize, &dwRead, NULL)
		== FALSE || dwRead != FileSize - 0x40)
		return PrintStr("�ļ���ȡ����", FileHandle);

	// �رվ������ֹй¶
	CloseHandle(FileHandle);
	//printf("���ļ��ɹ����ļ���С��%lu��0x%p\n", FileSize == dwRead, mpFile);
	*pFileSize = FileSize;
	return (int)mpFile;
}

BOOL SaveFile5(LPCSTR Path, LPCCH pMem, DWORD pFileSize)
{
	// ��ֻ���ķ�ʽ��ָ�����ļ���Ҫ���ļ�����
	HANDLE FileHandle = CreateFileA(Path, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("���ļ��򿪴���");
	DWORD dwLen = 0;
	WriteFile(FileHandle, pMem, pFileSize, &dwLen, 0);
	// �رվ������ֹй¶
	CloseHandle(FileHandle);
	return pFileSize == dwLen;
}

void XorMem5(PBYTE oldAddr, PBYTE newAddr, DWORD Size, DWORD Key)
{
	PBYTE bkey = (PBYTE)&Key;
	for (DWORD i = 0; i < Size; i++)
	{
		newAddr[i] = oldAddr[i] ^ bkey[0] ^ bkey[1] ^ bkey[2] ^ bkey[3];
	}
}

BOOL Lz4Compress(LPMyLz4 pLZ4)
{
	int oldSize, newSize, isCompress = pLZ4->Compress;
	char* pBuff, * oldAddr, * newAddr = 0;
	auto hHeap = GetProcessHeap();

	if (isCompress)	//ѹ��
	{
		oldSize = pLZ4->oldSize;
		newSize = LZ4_compressBound(pLZ4->oldSize);
		oldAddr = pLZ4->oldAddr;
	}
	else
	{
		oldAddr = pLZ4->oldAddr;
		int* tmp = (int*)oldAddr;
		oldAddr += 8;
		oldSize = tmp[1];
		newSize = tmp[0];
	}
	//1. ��ȡԤ����ѹ������ֽ���:
	pBuff = (PCH)HeapAlloc(hHeap, 0, newSize);
	if (!pBuff)	return 0;

	if (isCompress)
	{
		newSize = LZ4_compress(
			oldAddr,/*ѹ��ǰ������*/
			pBuff,	/*ѹ���������*/
			oldSize	/*�ļ�ԭʼ��С*/);
		newAddr = (PCH)HeapAlloc(hHeap, 0, newSize + 8);
		if (newAddr)	//�����ַ�ɹ�
		{
			LPDWORD size = (LPDWORD)newAddr;
			size[0] = oldSize;
			size[1] = newSize;
			memcpy(newAddr + 8, pBuff, newSize);
			pLZ4->newAddr = newAddr;
			pLZ4->newSize = newSize;
		}
		HeapFree(hHeap, 0, pBuff);
	}
	else
	{
		newAddr = pBuff;
		LZ4_uncompress_unknownOutputSize(
			oldAddr,/*ѹ���������*/
			newAddr,/*��ѹ����������*/
			oldSize,/*ѹ����Ĵ�С*/
			newSize	/*ѹ��ǰ�Ĵ�С*/);
		pLZ4->newAddr = pBuff;
		pLZ4->newSize = newSize;
		pLZ4->oldSize = oldSize;
	}
	return newAddr > 0;
}
#pragma endregion


// ��̬�Ļ�ȡ�Լ���Ҫ�õ��ĺ�����ע���������ֻ��Ҫ���������ʱ���ȡ�Լ���API
	//	ʱʹ�ã������ȡ LoadLibrary GetProcAddress
NOINLine DWORD __stdcall MyGetProcAddress5(DWORD Module, LPCSTR FunName)
{
	// ��ȡ Dos ͷ�� Nt ͷ
	auto DosHeader = (PIMAGE_DOS_HEADER)Module;
	auto NtHeader = (PIMAGE_NT_HEADERS)(Module + DosHeader->e_lfanew);
	// ��ȡ������ṹ
	DWORD ExportRva = NtHeader->OptionalHeader.DataDirectory[0].VirtualAddress;
	auto ExportTable = (PIMAGE_EXPORT_DIRECTORY)(Module + ExportRva);
	// �ҵ��������Ʊ���ű���ַ��
	auto NameTable = (DWORD*)(ExportTable->AddressOfNames + Module);
	auto FuncTable = (DWORD*)(ExportTable->AddressOfFunctions + Module);
	auto OrdinalTable = (WORD*)(ExportTable->AddressOfNameOrdinals + Module);
	// ����������
	for (DWORD i = 0; i < ExportTable->NumberOfNames; ++i)
	{
		// ��ȡ����
		char* Name = (char*)(NameTable[i] + Module);
		if (!strcmp(Name, FunName))
			return FuncTable[OrdinalTable[i]] + Module;
	}
	return -1;
}


CData5::CData5()
{
	HMODULE ke32 = 0;
	MyKe32 myk32; MyNTDLL myntdll;
	PGetProcAddress GetProc;
	__asm
	{
		mov eax, dword ptr fs : [0x30]
		mov eax, dword ptr[eax + 0x0C]
		mov eax, dword ptr[eax + 0x0C]
		mov eax, dword ptr[eax]
		mov eax, dword ptr[eax]
		mov eax, dword ptr[eax + 0x18]
		mov ke32, eax;
		push gszGetProcAddress;
		push eax;
		call MyGetProcAddress5;
		mov GetProc, eax;
	}
	this->k32 = ke32;
	// ����K32
	myk32.pGetProcAddress = GetProc;
	myk32.pLoadLibraryA = (PLoadLibraryA)		GetProc(ke32, gszLoadLibraryA);
	myk32.pGetModuleHandleA = (PGetModuleHandleA)GetProc(ke32, gszGetModuleHandleA);
	myk32.pExitProcess = (PExitProcess)			GetProc(ke32, gszExitProcess);
	this->exeBase = myk32.pGetModuleHandleA(0);
	// ����NTDLL
	this->ntdll = myk32.pLoadLibraryA(gszNTDLL);
	myntdll.pmemset = (Pmemset)GetProc(this->ntdll, gszmemset);
	myntdll.pmemcpy = (Pmemcpy)GetProc(this->ntdll, gszmemcpy);
	myntdll.pstrcmp = (Pstrcmp)GetProc(this->ntdll, gszstrcmp);
	// ����USER32
	this->u32 = myk32.pLoadLibraryA(gszUSER32);

	myntdll.pmemcpy(&this->K32s, &myk32, sizeof(MyKe32));
	myntdll.pmemcpy(&this->NTs, &myntdll, sizeof(MyNTDLL));
}

NOINLine int CData5::myStrcmp(LPSTR str1, LPSTR str2)
{
	Pstrcmp mystrcmp = this->NTs.pstrcmp;
	return mystrcmp(str1, str2);
}

Wind::Wind()
{
}

CMyFile::CMyFile()
{
}

DWORD CMyFile::ReadOldEXE()
{
	DWORD FileSize,
		pMem = ReadFile5("..\\Debug\\demo.exe", &FileSize);
	return pMem;
}
