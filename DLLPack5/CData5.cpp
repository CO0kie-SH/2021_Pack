#include "pch.h"
#include "CData5.h"


#pragma region 类外函数
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
	// 以只读的方式打开指定的文件，要求文件存在
	HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("文件打开错误。");


	// 获取文件的大小
	DWORD dwRead, FileSize = GetFileSize(FileHandle, &dwRead);
	if (dwRead || FileSize < 0x40)	//DOS头
		return PrintStr("文件大小错误。", FileHandle);

	// 读取文件
	BYTE buff[0x40];
	if (::ReadFile(FileHandle, (LPVOID)buff, 0x40, &dwRead, NULL)
		== FALSE || dwRead != 0x40 ||
		buff[0] != 0x4D ||
		buff[1] != 0x5A) {
		return PrintStr("DOS头错误。", FileHandle);
	}

	// 申请堆空间
	auto mpFile = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize * sizeof(BYTE));
	if (!mpFile)
		return PrintStr("分配内存错误。", FileHandle);

	// 读取文件
	memcpy(mpFile, buff, 0x40);
	if (::ReadFile(FileHandle, (LPVOID)(mpFile + 0x40), FileSize, &dwRead, NULL)
		== FALSE || dwRead != FileSize - 0x40)
		return PrintStr("文件读取错误。", FileHandle);

	// 关闭句柄，防止泄露
	CloseHandle(FileHandle);
	//printf("打开文件成功，文件大小：%lu，0x%p\n", FileSize == dwRead, mpFile);
	*pFileSize = FileSize;
	return (int)mpFile;
}

BOOL SaveFile5(LPCSTR Path, LPCCH pMem, DWORD pFileSize)
{
	// 以只读的方式打开指定的文件，要求文件存在
	HANDLE FileHandle = CreateFileA(Path, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("新文件打开错误。");
	DWORD dwLen = 0;
	WriteFile(FileHandle, pMem, pFileSize, &dwLen, 0);
	// 关闭句柄，防止泄露
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

	if (isCompress)	//压缩
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
	//1. 获取预估的压缩后的字节数:
	pBuff = (PCH)HeapAlloc(hHeap, 0, newSize);
	if (!pBuff)	return 0;

	if (isCompress)
	{
		newSize = LZ4_compress(
			oldAddr,/*压缩前的数据*/
			pBuff,	/*压缩后的数据*/
			oldSize	/*文件原始大小*/);
		newAddr = (PCH)HeapAlloc(hHeap, 0, newSize + 8);
		if (newAddr)	//分配地址成功
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
			oldAddr,/*压缩后的数据*/
			newAddr,/*解压出来的数据*/
			oldSize,/*压缩后的大小*/
			newSize	/*压缩前的大小*/);
		pLZ4->newAddr = pBuff;
		pLZ4->newSize = newSize;
		pLZ4->oldSize = oldSize;
	}
	return newAddr > 0;
}
#pragma endregion


// 动态的获取自己需要用到的函数，注意这个函数只需要在最起初的时候获取自己的API
	//	时使用，例如获取 LoadLibrary GetProcAddress
NOINLine DWORD __stdcall MyGetProcAddress5(DWORD Module, LPCSTR FunName)
{
	// 获取 Dos 头和 Nt 头
	auto DosHeader = (PIMAGE_DOS_HEADER)Module;
	auto NtHeader = (PIMAGE_NT_HEADERS)(Module + DosHeader->e_lfanew);
	// 获取导出表结构
	DWORD ExportRva = NtHeader->OptionalHeader.DataDirectory[0].VirtualAddress;
	auto ExportTable = (PIMAGE_EXPORT_DIRECTORY)(Module + ExportRva);
	// 找到导出名称表、序号表、地址表
	auto NameTable = (DWORD*)(ExportTable->AddressOfNames + Module);
	auto FuncTable = (DWORD*)(ExportTable->AddressOfFunctions + Module);
	auto OrdinalTable = (WORD*)(ExportTable->AddressOfNameOrdinals + Module);
	// 遍历找名字
	for (DWORD i = 0; i < ExportTable->NumberOfNames; ++i)
	{
		// 获取名字
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
	// 加载K32
	myk32.pGetProcAddress = GetProc;
	myk32.pLoadLibraryA = (PLoadLibraryA)		GetProc(ke32, gszLoadLibraryA);
	myk32.pGetModuleHandleA = (PGetModuleHandleA)GetProc(ke32, gszGetModuleHandleA);
	myk32.pExitProcess = (PExitProcess)			GetProc(ke32, gszExitProcess);
	this->exeBase = myk32.pGetModuleHandleA(0);
	// 加载NTDLL
	this->ntdll = myk32.pLoadLibraryA(gszNTDLL);
	myntdll.pmemset = (Pmemset)GetProc(this->ntdll, gszmemset);
	myntdll.pmemcpy = (Pmemcpy)GetProc(this->ntdll, gszmemcpy);
	myntdll.pstrcmp = (Pstrcmp)GetProc(this->ntdll, gszstrcmp);
	// 加载USER32
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
