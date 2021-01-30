#include "CPE6.h"


CPE6::CPE6()
{
	auto Base = (HMODULE)GetBaseDLL();
	char PathDLL[MAX_PATH];
	GetModuleFileNameA(Base, PathDLL, MAX_PATH);

	// 读取文件
	DWORD SizeDll, SizeEXE;
	auto pMemDLL = (LPBYTE)ReadFile6(PathDLL, &SizeDll);
	auto pMemEXE = (LPBYTE)ReadFile6("..\\Debug\\demo.exe", &SizeEXE);
	if (!pMemDLL || !pMemEXE)	return;

	// 加密文件
	auto pXorMem = (PBYTE)HeapAlloc(GetProcessHeap(), 0, SizeEXE);
	if (pXorMem == 0)	return;


	XorMem6(pMemEXE, pXorMem, SizeEXE, 0x34333231);
	

	// 压缩文件
	MyLz4 lz4 = {
		TRUE,(LPCH)pXorMem,0,SizeEXE
	};
	BOOL bCom = Lz4Compress6(&lz4);
	if (!bCom)	return;

	// 保存加密文件备份
	SaveFile6("..\\Debug\\demo.xor.lz4",
		lz4.newAddr, lz4.newSize + 8);

	SetPE(pMemDLL, pMemEXE, &lz4);
}

CPE6::~CPE6()
{
}

BOOL CPE6::SetPE(LPBYTE pDLL, LPBYTE pEXE, LPMyLz4 lz4)
{
	// 壳信息
	auto dNT = NtHeader((DWORD)pDLL);
	auto dOH = &dNT->OptionalHeader;
	auto& dDir = dOH->DataDirectory;
	auto dSEC = IMAGE_FIRST_SECTION(dNT);
	// 原文件信息
	auto fNT = NtHeader((DWORD)pEXE);
	auto fOH = &fNT->OptionalHeader;

	DWORD dwfSEC = fNT->FileHeader.NumberOfSections,
		dllSize = 0x7000 - dOH->BaseOfCode,
		newHead = fOH->SizeOfHeaders,
		//新大小 = 头大小 + 壳镜像大小 + 8字 + 压缩包
		newSize = newHead + dllSize + 8 + lz4->newSize,
		//新区段 = 新大小 - 头大小
		newTEXT = newSize - newHead;
	

	// 申请空间
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

	// 清空信息表
	ZeroMemory(nDir, sizeof(IMAGE_DATA_DIRECTORY)
		* IMAGE_NUMBEROF_DIRECTORY_ENTRIES);
	ZeroMemory(nSEC + 1, sizeof(IMAGE_SECTION_HEADER) * dwfSEC);
	memcpy(nDir + 1, dDir + 1, 8);
	nSEC->SizeOfRawData = newTEXT;
	nSEC->Misc.VirtualSize = newTEXT;
	nSEC->Characteristics = 0xE00000E0;

	// 设置扩展大小
	nNT->FileHeader.NumberOfSections = 1;
	nOH->BaseOfData = 0;
	nOH->SizeOfCode = newTEXT;
	nOH->SizeOfImage = nSEC->VirtualAddress + nSEC->Misc.VirtualSize;

	// 设置OEP
	DWORD tmp = (DWORD)GetBaseDLL();
	tmp = (DWORD)GetProcAddress((HMODULE)tmp, "start") - tmp;
	nOH->AddressOfEntryPoint = tmp;
	//nOH->DllCharacteristics = 0x8100;

	PCCH savePath = "..\\Debug\\demo.6.exe";
	DeleteFileA(savePath);
	BOOL bSave = SaveFile6(savePath, pHeap, newSize);
	printf("保存：%s\n", bSave ? "成功" : "失败");
	return bSave;
}
