#include "pch.h"
#include "CData.h"


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

// 动态的获取自己需要用到的函数，注意这个函数只需要在最起初的时候获取自己的API
	//	时使用，例如获取 LoadLibrary GetProcAddress
DWORD MyGetProcAddress(DWORD Module, LPCSTR FunName)
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