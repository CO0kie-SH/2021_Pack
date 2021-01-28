#include "pch.h"
#include "CData.h"


BOOL Lz4Compress(LPMyLz4 pLZ4)
{
	int oldSize, newSize, isCompress = pLZ4->Compress;
	char* pBuff, * oldAddr, * newAddr = 0;
	auto hHeap = gAPI->gHeap;

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
	pBuff = (PCH)gAPI->pHeapAlloc(hHeap, 0, newSize);
	if (!pBuff)	return 0;

	if (isCompress)
	{
		newSize = LZ4_compress(
			oldAddr,/*压缩前的数据*/
			pBuff,	/*压缩后的数据*/
			oldSize	/*文件原始大小*/);
		newAddr = (PCH)gAPI->pHeapAlloc(hHeap, 0, newSize + 8);
		if (newAddr)	//分配地址成功
		{
			LPDWORD size = (LPDWORD)newAddr;
			size[0] = oldSize;
			size[1] = newSize;
			gAPI->pmemcpy(newAddr + 8, pBuff, newSize);
			pLZ4->newAddr = newAddr;
			pLZ4->newSize = newSize;
		}
		gAPI->pHeapFree(hHeap, 0, pBuff);
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

PMyWAPI gAPI;

BOOL LoadAPI()
{
	HMODULE ke32 = 0;
	MyWAPI API;
	__asm
	{
		mov eax, dword ptr fs : [0x30]
		mov eax, dword ptr[eax + 0x0C]
		mov eax, dword ptr[eax + 0x0C]
		mov eax, dword ptr[eax]
		mov eax, dword ptr[eax]
		mov eax, dword ptr[eax + 0x18]
		mov ke32, eax;
	}
	auto GetProc = (PGetProcAddress)MyGetProcAddress((DWORD)ke32, gszGetProcAddress);
	API.ke32 = ke32;
	API.pGetProcAddress = GetProc;
	API.pLoadLibraryA = (PLoadLibraryA)GetProc(ke32, gszLoadLibraryA);
	API.pGetModuleHandleA = (PGetModuleHandleA)GetProc(ke32, gszGetModuleHandleA);
	API.pExitProcess = (PExitProcess)GetProc(ke32, gszExitProcess);
	API.ntdll = API.pLoadLibraryA(gszNTDLL);
	API.pmemcpy = (Pmemcpy)GetProc(API.ntdll, gszmemcpy);
	API.pmemset = (Pmemset)GetProc(API.ntdll, gszmemset);
	//页保护
	API.pVirtualAlloc = (PVirtualAlloc)GetProc(ke32, gszVirtualAlloc);
	API.pVirtualProtect = (PVirtualProtect)GetProc(ke32, gszVirtualProtect);
	//堆控制
	API.pGetProcessHeap = (PGetProcessHeap)GetProc(ke32, gszGetProcessHeap);
	API.gHeap = API.pGetProcessHeap();
	API.pHeapAlloc = (PHeapAlloc)GetProc(ke32, gszHeapAlloc);
	API.pHeapFree = (PHeapFree)GetProc(ke32, gszHeapFree);


	gAPI=(PMyWAPI)API.pHeapAlloc(API.gHeap, 0, sizeof(MyWAPI));
	if (!gAPI) API.pExitProcess(0);
	API.pmemcpy(gAPI, &API, sizeof(MyWAPI));
}