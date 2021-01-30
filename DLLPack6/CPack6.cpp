#include "pch.h"
#include "CPack6.h"
#include <iostream>
#include "CPe.h"

CPack6::CPack6()
	:nowBase(0), pFile(0)
{
	CDbg6 dbg6;
	CWind6 wind6;
	LPCH base = (LPCH)GetModuleHandleA(0);
	printf("当前随机基址：0x%p\n", base);

	MyLz4 lz4 = { FALSE,base + 0x7000 };

	DWORD key1 = 1, key2 = 2,
		bRet = Lz4Compress6(&lz4);
	if (bRet == 0)	return;

	key2 = wind6.Create();
	XorMem6((PBYTE)lz4.newAddr, (PBYTE)lz4.newAddr, lz4.newSize, key2);
	//TextAES(FALSE, (PBYTE)lz4.newAddr, lz4.newSize, 0x34333231);

	this->pFile = lz4.newAddr;
	this->Size = lz4.newSize;

	if (CreateCMD())
		return;
	LoadEXE6();
}

CPack6::~CPack6()
{
	__asm
	{
		xor eax, eax;
		push eax;
		mov edx, ExitProcess;
		call edx;
	}
}

BOOL CPack6::LoadEXE6()
{
	CDbg6 dbg6;
	// 判断文件是否合法
	auto pBase = (PIMAGE_DOS_HEADER)this->pFile;
	if (pBase == 0 ||
		pBase->e_magic != IMAGE_DOS_SIGNATURE ||
		NtHeader((DWORD)pBase)->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	// 初始化原文件参数
	auto fNT = NtHeader((DWORD)pBase);
	auto fOH = &fNT->OptionalHeader;
	auto fSECnew = IMAGE_FIRST_SECTION(fNT);
	nowBase = (DWORD)VirtualAlloc(NULL, fOH->SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (nowBase == 0)	return 0;

	// 计算大小
	LPCH nBase = (LPCH)this->nowBase, fBase = (LPCH)pBase;
	DWORD dwTEXT = fOH->BaseOfCode;

	// 清空镜像
	//memset(nBase + dwTEXT, 0x0, fOH->SizeOfImage - dwTEXT);
	for (DWORD i = fNT->FileHeader.NumberOfSections; i > 0; )
	{
		auto& SEC = fSECnew[--i];
		LPCH VA = nBase + SEC.VirtualAddress,
			FOA = fBase + SEC.PointerToRawData;
		memcpy(nBase + SEC.VirtualAddress,	//VA
			fBase + SEC.PointerToRawData,	//FOA
			SEC.SizeOfRawData);				//RSize
	}

	// 先修复重定位
	auto& RELOC = fOH->DataDirectory[5];
	if (!FixLOC(fOH->ImageBase, (DWORD)nBase,		//旧、新基址
		RELOC.VirtualAddress, fOH->BaseOfCode, 0))	//RVA、段偏移
		return 0;
	memset(nBase + RELOC.VirtualAddress, 0, RELOC.Size);

	// 再修复IAT表
	if (!FixIAT(fOH->DataDirectory[1].VirtualAddress))	return 0;

	// 返回新的OEP
	nBase += fOH->AddressOfEntryPoint;
	//printf("新基址：%08lX\n新入口：%p\n", nowBase, nBase);
	_asm call nBase;
	return TRUE;
}

BOOL CPack6::FixIAT(DWORD RVA)
{
	CDbg6 dbg6;
	DWORD ImageBase = nowBase;
	HANDLE hHeap = GetProcessHeap();	//获取默认堆
	auto ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)(ImageBase + RVA);
	while (ImportTable->Name != 0)
	{
		CHAR* DllName = (CHAR*)(ImportTable->Name + ImageBase);
		HMODULE hMdule = LoadLibraryA(DllName);

		if (!hMdule)	return 0;

		auto IAT = (PIMAGE_THUNK_DATA)(ImportTable->FirstThunk + ImageBase);
		while (IAT->u1.Function)
		{
			DWORD OldProtect = 0, FunctionAddr = 0;
			VirtualProtect(IAT, 4, PAGE_READWRITE, &OldProtect);

			if ((IAT->u1.Function & 0x80000000) == 0)
			{
				auto Name = (PIMAGE_IMPORT_BY_NAME)(IAT->u1.Function + ImageBase);
				FunctionAddr = (DWORD)GetProcAddress(hMdule, Name->Name);
			}
			else
			{
				FunctionAddr = (DWORD)GetProcAddress(hMdule, (LPCSTR)(IAT->u1.Ordinal & 0xFFFF));
			}
			FunctionAddr -= 0x1234;

			LPCH Addr = (LPCH)HeapAlloc(hHeap, 0, 13);
			if (!Addr)	return 0;

			memcpy(Addr, gIAT, 13);
			*(LPDWORD)&Addr[1] = FunctionAddr;
			IAT->u1.Function = (DWORD)Addr;

			VirtualProtect(IAT, 4, OldProtect, &OldProtect);
			VirtualProtect(Addr, 10, PAGE_EXECUTE_READWRITE, &OldProtect);
			IAT++;
		}

		ImportTable++;
	}
	return TRUE;
}

BOOL CPack6::CreateCMD()
{
	// 1.启动进程 -挂起
	STARTUPINFO stSi = { sizeof(STARTUPINFOA) };
	PROCESS_INFORMATION stPi = { 0 };
	BOOL bRet = CreateProcess(
		L"C:\\Windows\\SysWOW64\\cmd.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED| CREATE_NEW_CONSOLE,//挂起进程
		NULL,
		NULL,
		&stSi,
		&stPi);
	if (bRet == FALSE)
	{
		MessageBox(NULL, L"create notepad error", NULL, MB_OK);
		return 0;
	}

	// 2.获取进程寄存器上下文
	CONTEXT threadContext = { CONTEXT_ALL };
	bRet = GetThreadContext(stPi.hThread, &threadContext);
	if (bRet == FALSE)
	{
		MessageBox(NULL, L"get context error", NULL, MB_OK);
		return 0;
	}

	// 3.读取主程序加载基址 : 启动进程时【ebx + 8】存放 加载基址
	DWORD ImageBase;
	DWORD dwReadSize;
	ReadProcessMemory(
		stPi.hProcess,
		(PVOID)(threadContext.Ebx + 8),
		&ImageBase,
		4,
		&dwReadSize);
	// 4. 释放内存空间 
	typedef ULONG(WINAPI* MyNtUnmapViewOfSection)(HANDLE ProcessHandle, PVOID BaseAddress);
	// 4.1 使用NtUnmapViewOfSection未文档函数,先或函数
	MyNtUnmapViewOfSection UnMapviewOfSection =
		(MyNtUnmapViewOfSection)GetProcAddress(
			LoadLibrary(L"ntdll.dll"),
			"NtUnmapViewOfSection");
	// 4.2 释放目标进程内存空间
	UnMapviewOfSection(stPi.hProcess, (PVOID)ImageBase);

	// 5.1 读取新PE文件，
	CPe pe;
	//pe.Analysis_Pe((char*)"PE查看器.exe");
	if (pe.Analysis_Pe(this->pFile, this->Size) == false)
		return 0;

	// 5.2 申请新空间
	DWORD NewImageBase = pe.GetNtHeader()->OptionalHeader.ImageBase;
	DWORD SizeImage = pe.GetNtHeader()->OptionalHeader.SizeOfImage;

	LPVOID NewProcessBaseAddr = VirtualAllocEx(
		stPi.hProcess,
		(LPVOID)ImageBase,
		SizeImage,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);

	if (NewProcessBaseAddr == 0)	return 0;
	// 5.3 写入数据到目标进程中
	// 5.3.1 替换PE头
	WriteProcessMemory(
		stPi.hProcess,
		NewProcessBaseAddr,
		pe.GetPeBuff(),
		pe.GetNtHeader()->OptionalHeader.SizeOfHeaders,	// 头部大小
		&dwReadSize
	);


	// 5.3.3 替换节
	// 源exe
	PIMAGE_SECTION_HEADER SectionnAddress = pe.GetSectionHeader();
	// 获取区段个数
	DWORD count = pe.GetNtHeader()->FileHeader.NumberOfSections;
	for (int i = 0; i < count; i++)
	{
		// 将每个区段写入到目标文件中
		WriteProcessMemory(
			stPi.hProcess,
			(LPVOID)((DWORD)NewProcessBaseAddr + SectionnAddress[i].VirtualAddress),
			(LPVOID)((DWORD)pe.GetPeBuff() + SectionnAddress[i].PointerToRawData),
			SectionnAddress[i].SizeOfRawData,
			&dwReadSize
		);

	}
	threadContext.Eax = pe.GetOptionalHeander()->AddressOfEntryPoint + (DWORD)NewProcessBaseAddr;
	SetThreadContext(stPi.hThread, &threadContext);

	ResumeThread(stPi.hThread);
	return TRUE;
}
