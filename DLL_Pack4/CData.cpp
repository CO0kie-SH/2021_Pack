#include "pch.h"
#include "CData.h"


BOOL Lz4Compress(LPMyLz4 pLZ4)
{
	int oldSize, newSize, isCompress = pLZ4->Compress;
	char* pBuff, * oldAddr, * newAddr = 0;
	auto hHeap = gAPI->gHeap;

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
	pBuff = (PCH)gAPI->pHeapAlloc(hHeap, 0, newSize);
	if (!pBuff)	return 0;

	if (isCompress)
	{
		newSize = LZ4_compress(
			oldAddr,/*ѹ��ǰ������*/
			pBuff,	/*ѹ���������*/
			oldSize	/*�ļ�ԭʼ��С*/);
		newAddr = (PCH)gAPI->pHeapAlloc(hHeap, 0, newSize + 8);
		if (newAddr)	//�����ַ�ɹ�
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

// ��̬�Ļ�ȡ�Լ���Ҫ�õ��ĺ�����ע���������ֻ��Ҫ���������ʱ���ȡ�Լ���API
	//	ʱʹ�ã������ȡ LoadLibrary GetProcAddress
DWORD MyGetProcAddress(DWORD Module, LPCSTR FunName)
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
	//ҳ����
	API.pVirtualAlloc = (PVirtualAlloc)GetProc(ke32, gszVirtualAlloc);
	API.pVirtualProtect = (PVirtualProtect)GetProc(ke32, gszVirtualProtect);
	//�ѿ���
	API.pGetProcessHeap = (PGetProcessHeap)GetProc(ke32, gszGetProcessHeap);
	API.gHeap = API.pGetProcessHeap();
	API.pHeapAlloc = (PHeapAlloc)GetProc(ke32, gszHeapAlloc);
	API.pHeapFree = (PHeapFree)GetProc(ke32, gszHeapFree);


	gAPI=(PMyWAPI)API.pHeapAlloc(API.gHeap, 0, sizeof(MyWAPI));
	if (!gAPI) API.pExitProcess(0);
	API.pmemcpy(gAPI, &API, sizeof(MyWAPI));
}