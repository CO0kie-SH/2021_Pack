#include "pch.h"
#include "CPack4.h"


BOOL FixREL(DWORD Base, DWORD ModBase, DWORD RVA, DWORD dwTEXT)
{
	// 由于DLL本身是已经加载到内存中的，其中所有的全局变量都被重定位到
	//	了DLL目前的加载基址，通常是(0x7XXXXXXX)，在我们将DLL拷贝到PE
	//	文件中后，由于加载基址和区段基址被该变了，所以需要进行修复

	// 0. 获取到要使用的字段
	DWORD OldImageBase = Base;		//(0x400000)/(0x1000000)
	DWORD NewImageBase = ModBase;	//新的随机基址
	DWORD OldSectionBase = OldImageBase + dwTEXT;	//区段偏移
	DWORD NewSectionBase = NewImageBase + dwTEXT;	//区段偏移
	//DWORD dwNum = 0, dwOld = 0;

	// 1. 通过 DLL 的数据目录表 RVA 计算到重定位表的地址
	auto RelocTable = (PIMAGE_BASE_RELOCATION)
		(RVA + NewImageBase);
	// 2. 遍历重定位表(每一个重定位块)，结束条件是全0的结构
	while (RelocTable->SizeOfBlock)
	{

		typedef struct _TYPEOFFSET
		{
			USHORT Offset : 12;
			USHORT Type : 4;
		}TYPEOFFSET, * PTYPEOFFSET;

		// 3. 计算出重定位项数组的起始位置以及数组的元素个数
		PTYPEOFFSET TypeOffset = (PTYPEOFFSET)(RelocTable + 1);
		DWORD Count = (RelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
		//printf("%lu\t%lX\t%lu\n", ++dwNum,
		//	RelocTable->VirtualAddress, Count);
		//VirtualProtect((LPVOID)(NewImageBase + RelocTable->VirtualAddress), 0x1000, PAGE_EXECUTE_READWRITE, &dwOld);

		// 4. 遍历每一个重定位项，找到其中类型为 3 的项
		for (DWORD i = 0; i < Count; ++i)
		{
			if (TypeOffset[i].Type == 3)
			{

				DWORD* addr = (DWORD*)(TypeOffset[i].Offset + RelocTable->VirtualAddress + NewImageBase);
				DWORD newAddr = *addr - OldSectionBase + NewSectionBase;
				*addr = newAddr;
				_asm nop;
			}
		}
		//VirtualProtect((LPVOID)(NewImageBase + RelocTable->VirtualAddress), 0x1000, dwOld, &dwOld);
		RelocTable = (PIMAGE_BASE_RELOCATION)((DWORD)RelocTable + RelocTable->SizeOfBlock);
	}
	return TRUE;
}

CPack4::CPack4()
{
	API = gAPI;
	mHeap = API->gHeap;
	oldBase = (DWORD)API->pGetModuleHandleA(0);
	auto pNT = NtHeader(oldBase);
	mOH = &pNT->OptionalHeader;

	//解压旧数据
	oldAddr = (LPCH)oldBase + mOH->BaseOfCode;
	MyLz4 lz4 = { FALSE,oldAddr };
	if (Lz4Compress(&lz4))
	{
		newBase = (DWORD)lz4.newAddr;
		UnOldBase();
	}
}

CPack4::~CPack4()
{
	API->pExitProcess(0);
}

BOOL CPack4::UnOldBase()
{
	auto pNTnew = NtHeader(newBase);
	auto pOHnew = &pNTnew->OptionalHeader;
	auto pSECnew = IMAGE_FIRST_SECTION(pNTnew);
	auto pSECold = IMAGE_FIRST_SECTION(NtHeader(oldBase));

	DWORD RVA = 0;
	LPCH oldBase = (LPCH)this->oldBase, newBase = (LPCH)this->newBase;

	//还原镜像
	API->pmemset(oldBase + pSECold->VirtualAddress, 0, pSECold->Misc.VirtualSize);
	for (DWORD i = pNTnew->FileHeader.NumberOfSections; i >0; )
	{
		auto& SEC = pSECnew[--i];
		LPCH VA = oldBase + SEC.VirtualAddress,
			FOA = newBase + SEC.PointerToRawData;
		API->pmemcpy(VA, FOA, SEC.SizeOfRawData);
	}
	// 重定位表
	RVA = pOHnew->DataDirectory[5].VirtualAddress;
	if (!FixREL(pOHnew->ImageBase, this->oldBase, RVA, pSECold->VirtualAddress))	return 0;

	// IAT表
	RVA = pOHnew->DataDirectory[1].VirtualAddress;
	if (!FixIAT(RVA))	return 0;

	// 返回新的OEP
	oldBase += pOHnew->AddressOfEntryPoint;
	_asm call oldBase;
	return (int)oldBase;
}

BOOL CPack4::FixIAT(DWORD RVA)
{
	DWORD ImageBase = oldBase;
	HANDLE hHeap = mHeap;
	// 修复IAT实际上就是，遍历导入表，并加载DLL，获取函数地址，填充IAT
	auto ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)(ImageBase + RVA);
	// 遍历导入表，导入表以一个全0 的结构结尾
	while (ImportTable->Name != 0)
	{
		// 计算出导入表对应DLL的名称，并通过LoadLibrary进行加载
		CHAR* DllName = (CHAR*)(ImportTable->Name + ImageBase);
		HMODULE hMdule = API->pLoadLibraryA(DllName);
		
		// 加载失败则返回
		if (!hMdule)	return 0;

		// 获取到 IAT 的地址，计算出相应的序号或名称
		auto IAT = (PIMAGE_THUNK_DATA)(ImportTable->FirstThunk + ImageBase);

		// 遍历 IAT 表，判断其中保存的函数是序号的还是名称的
		while (IAT->u1.Function)
		{
			DWORD OldProtect = 0, FunctionAddr = 0;
			API->pVirtualProtect(IAT, 4, PAGE_READWRITE, &OldProtect);

			// 如果最高位为0，那么保存的就是名称结构体的RVA
			if ((IAT->u1.Function & 0x80000000) == 0)
			{
				auto Name = (PIMAGE_IMPORT_BY_NAME)(IAT->u1.Function + ImageBase);
				FunctionAddr = (DWORD)API->pGetProcAddress(hMdule, Name->Name);
			}
			else
			{
				// 如果没有名字，那么低16位保存的就是序号
				FunctionAddr = (DWORD)API->pGetProcAddress(hMdule, (LPCSTR)(IAT->u1.Ordinal & 0xFFFF));
			}

			// 加密函数地址，在这里将函数地址 - 1
			FunctionAddr -= 1;

			// 申请一块空间，用于保存解密的代码
			LPCH Addr = (LPCH)API->pHeapAlloc(hHeap, 0, 10);
			if (!Addr)	return 0;

			// 替换表格
			API->pmemcpy(Addr, OpCode, 10);
			*(LPDWORD)&Addr[1] = FunctionAddr;
			IAT->u1.Function = (DWORD)Addr;

			API->pVirtualProtect(IAT, 4, OldProtect, &OldProtect);
			API->pVirtualProtect(Addr, 10, PAGE_EXECUTE_READWRITE, &OldProtect);
			IAT++;
		}

		ImportTable++;
	}
	return TRUE;
}

PIMAGE_NT_HEADERS CPack4::NtHeader(DWORD pMem)
{
	LPDWORD pAdd = ((LPDWORD)pMem + 15);
	return (PIMAGE_NT_HEADERS)(pMem + *pAdd);
}
