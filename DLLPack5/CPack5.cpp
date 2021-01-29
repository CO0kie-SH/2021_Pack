#include "pch.h"
#include "CPack5.h"


CPack5::CPack5(CData5* pData5)
{
	CMyFile cFile;
	this->nowBase = (DWORD)pData5->exeBase;
	this->K32s = &pData5->K32s;
	this->NTDLLs = &pData5->NTs;

	fileBase = cFile.ReadOldEXE();
	if (fileBase == 0)	return;

	LoadEXE();
}

CPack5::~CPack5()
{
	__asm
	{
		xor eax, eax;
		push eax;
		mov eax, ExitProcess;
		call eax;
	}
}


BOOL CPack5::UnOldBase()
{
	
	return TRUE;
}


BOOL CPack5::LoadEXE()
{
	// 判断文件是否合法
	auto pBase = (PIMAGE_DOS_HEADER)this->fileBase;
	if (pBase == 0 ||
		pBase->e_magic != IMAGE_DOS_SIGNATURE ||
		NtHeader(pBase)->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	// 初始化原文件参数
	auto fNT = NtHeader(pBase);
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

NOINLine BOOL CPack5::FixIAT(DWORD RVA)
{
	DWORD ImageBase = nowBase;
	HANDLE hHeap = GetProcessHeap();
	// 修复IAT实际上就是，遍历导入表，并加载DLL，获取函数地址，填充IAT
	auto ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)(ImageBase + RVA);
	// 遍历导入表，导入表以一个全0 的结构结尾
	while (ImportTable->Name != 0)
	{
		// 计算出导入表对应DLL的名称，并通过LoadLibrary进行加载
		CHAR* DllName = (CHAR*)(ImportTable->Name + ImageBase);
		HMODULE hMdule = LoadLibraryA(DllName);

		// 加载失败则返回
		if (!hMdule)	return 0;

		// 获取到 IAT 的地址，计算出相应的序号或名称
		auto IAT = (PIMAGE_THUNK_DATA)(ImportTable->FirstThunk + ImageBase);

		// 遍历 IAT 表，判断其中保存的函数是序号的还是名称的
		while (IAT->u1.Function)
		{
			DWORD OldProtect = 0, FunctionAddr = 0;
			VirtualProtect(IAT, 4, PAGE_READWRITE, &OldProtect);

			// 如果最高位为0，那么保存的就是名称结构体的RVA
			if ((IAT->u1.Function & 0x80000000) == 0)
			{
				auto Name = (PIMAGE_IMPORT_BY_NAME)(IAT->u1.Function + ImageBase);
				FunctionAddr = (DWORD)GetProcAddress(hMdule, Name->Name);
			}
			else
			{
				// 如果没有名字，那么低16位保存的就是序号
				FunctionAddr = (DWORD)GetProcAddress(hMdule, (LPCSTR)(IAT->u1.Ordinal & 0xFFFF));
			}

			// 加密函数地址，在这里将函数地址 - 1
			FunctionAddr -= 1;

			// 申请一块空间，用于保存解密的代码
			LPCH Addr = (LPCH)HeapAlloc(hHeap, 0, 10);
			if (!Addr)	return 0;

			// 替换表格
			memcpy(Addr, OpCode, 10);
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
