#include "CPE.h"

BOOL PrintStr(LPCSTR Str, HANDLE FileHandle = 0)
{
	puts(Str);
	if (FileHandle)
		CloseHandle(FileHandle);
	return 0;
}

#pragma region 类内函数

CPE::CPE() :mpFile(0)
{
	mHeap = HeapCreate(0, 0, 0);
	ZeroMemory(&mPE, sizeof(MyPE));
}

CPE::~CPE()
{
	HeapDestroy(mHeap);
}


BOOL CPE::ReadFile(LPCSTR Path)
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
	auto mpFile = (LPBYTE)HeapAlloc(mHeap, 0, FileSize * sizeof(BYTE));
	if(!mpFile)
		return PrintStr("分配内存错误。", FileHandle);

	// 读取文件
	memcpy(mpFile, buff, 0x40);
	if (::ReadFile(FileHandle, (LPVOID)(mpFile + 0x40), FileSize, &dwRead, NULL)
		== FALSE || dwRead != FileSize - 0x40)
		return PrintStr("文件读取错误。", FileHandle);

	// 关闭句柄，防止泄露
	CloseHandle(FileHandle);
	printf("打开文件成功，文件大小：%lu，0x%p\n", FileSize, mpFile);

	//初始化PE结构体
	mPE = { FileSize,(DWORD)mpFile };
	return CheckPE(mpFile, &mPE);			//初始化PE
}

BOOL CPE::CheckPE(LPCBYTE pMem, LPMyPE pPE)
{
	auto pNT = NtHeader(pMem);				//PE头
	if (pNT->Signature != IMAGE_NT_SIGNATURE)
		return PrintStr("PE头错误。");

	auto& pFH = pNT->FileHeader;			//文件头
	printf("  文件头：\n区段的个数：%u\n扩展头大小：0x%X\n文件的标志：0x%X\n",
		pFH.NumberOfSections, pFH.SizeOfOptionalHeader, pFH.Characteristics);

	auto& pOH = pNT->OptionalHeader;		//扩展头
	printf("  扩展头：\n魔术字：0x%X\n代码大小：0x%X\n入口点处：【0x%X】\n\n" \
		"代码段起始：0x%X\n数据段起始：0x%X\n镜像大小：【0x%lX】\n" \
		"默认加载基址：0x%lX\n内存对齐：0x%lX\n文件对齐：0x%lX\n" \
		"所有头部大小：0x%lX\n区段偏移：->0x%p\nDLL标志：0x%X\n",
		pOH.Magic, pOH.SizeOfCode, pOH.AddressOfEntryPoint,
		pOH.BaseOfCode, pOH.BaseOfData, pOH.SizeOfImage,
		pOH.ImageBase, pOH.SectionAlignment, pOH.FileAlignment,
		pOH.SizeOfHeaders, pMem + pOH.SizeOfHeaders, pOH.DllCharacteristics);

	//保存PE结构体内存地址
	pPE->NumberOfSections = &pFH.NumberOfSections;
	pPE->SizeOfHeaders = &pOH.SizeOfHeaders;
	pPE->SizeOfImage = &pOH.SizeOfImage;
	pPE->SectionAlignment = &pOH.SectionAlignment;
	pPE->FileAlignment = &pOH.FileAlignment;
	pPE->AddressOfEntryPoint = &pOH.AddressOfEntryPoint;
	return CheckSection(pPE);
}

BOOL CPE::CheckSection(LPMyPE pPE, PIMAGE_SECTION_HEADER* pOut)
{
	printf("  区段列表如下：\n");

	//获取区段头表
	auto pSEC = IMAGE_FIRST_SECTION(NtHeader((LPCBYTE)pPE->FileMemAddr));
	BYTE name[9] = {};
	for (WORD i = 0, max = *pPE->NumberOfSections; i < max; i++, ++pSEC)
	{
		if (pOut) continue;
		memcpy(name, pSEC->Name, 8);
		printf("%u\t%s\tVA：%08lX\tVSize：%08lX\t" \
			"RVA：%08lX\tRSize：%08lX\t特征：%lX\n",
			i + 1, name, pSEC->VirtualAddress, pSEC->Misc.VirtualSize,
			pSEC->PointerToRawData, pSEC->SizeOfRawData, pSEC->Characteristics);
	}
	if (pOut) *pOut = pSEC;
	--pSEC;

	// 计算最后一个段的大小
	DWORD dwSize = pSEC->PointerToRawData + pSEC->SizeOfRawData;
	return dwSize == pPE->FileSize;
}

BOOL CPE::AddSection(LPMyPE pPE, LPCSTR SavePath)
{
	if (pPE == 0)	pPE = &this->mPE;

	//初始化新区段
	auto buff = (PBYTE)HeapAlloc(mHeap, HEAP_ZERO_MEMORY, 0x200);
	if (!buff)
		return PrintStr("无法创建堆控制");
	memcpy(buff, "\xE9\x90\x90\x90\x90", 5);
	pPE->dwNewSECMemAddr = (DWORD)buff;
	pPE->dwNewSECSize = 0x200;

	//获取区段头表
	auto pSEC = IMAGE_FIRST_SECTION(NtHeader((LPCBYTE)pPE->FileMemAddr));
	auto& pSEC_NEW = pSEC[*pPE->NumberOfSections];		//得到新表头
	auto& pSEC_END = pSEC[*pPE->NumberOfSections - 1];	//得到末尾表头

	//计算偏移
	DWORD nSEC_RVA = pSEC_END.PointerToRawData + pSEC_END.SizeOfRawData;
	DWORD nSEC_VA = pSEC_END.VirtualAddress + pSEC_END.Misc.VirtualSize;
	nSEC_RVA = this->MathOffset(nSEC_RVA, *pPE->FileAlignment);
	nSEC_VA = this->MathOffset(nSEC_VA, *pPE->SectionAlignment);

	//设置偏移
	pSEC_NEW.VirtualAddress = nSEC_VA;
	pSEC_NEW.PointerToRawData = nSEC_RVA;
	pSEC_NEW.Misc.VirtualSize = 0x200;
	pSEC_NEW.SizeOfRawData = 0x200;
	pSEC_NEW.Characteristics = 0xE00000E0;
	memcpy(pSEC_NEW.Name, ".CO0kie", 8);

	//加壳代码
	if (!LoadDLL(pPE, &pSEC_NEW))
		return PrintStr("加载壳插件失败。");

	//设置镜像属性
	*pPE->NumberOfSections = *pPE->NumberOfSections + 1;
	*pPE->SizeOfImage = pSEC_NEW.VirtualAddress + pSEC_NEW.Misc.VirtualSize;
	*pPE->AddressOfEntryPoint = pSEC_NEW.VirtualAddress;


	//保存文件
	if (SavePath)
		return SaveFile(SavePath, pPE, buff);
	return TRUE;
}

BOOL CPE::SaveFile(LPCSTR FilePath, LPMyPE pPE, PBYTE buff)
{
	// 以只读的方式打开指定的文件，要求文件存在
	HANDLE FileHandle = CreateFileA(FilePath, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("新文件打开错误。");

	//写入文件
	DWORD dwLen = 0, dwLen2 = 0, dwRet = 0;
	WriteFile(FileHandle, (LPVOID)pPE->FileMemAddr, pPE->FileSize, &dwLen, 0);
	WriteFile(FileHandle, (LPVOID)pPE->dwNewSECMemAddr, pPE->dwNewSECSize, &dwLen2, 0);
	CloseHandle(FileHandle);

	//判断成功
	dwRet = dwLen + dwLen2 == pPE->FileSize + pPE->dwNewSECSize;
	printf("  写入文件：%s\n", dwRet ? "成功" : "失败");
	CheckSection(pPE);
	return dwRet;
}

BOOL CPE::LoadDLL(LPMyPE pPE, PIMAGE_SECTION_HEADER pNewSEC)
{
	// 以不执行 DllMain 的方式将 Dll 装载到内存中
	auto Base = (LPCBYTE)LoadLibraryExA(DllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (!Base)
		return PrintStr("插件加载失败。");

	//获取头表信息
	auto pOH = NtHeader(Base)->OptionalHeader;
	auto pSEC = IMAGE_FIRST_SECTION(NtHeader(Base));
	
	//创建缓冲区
	if (pPE->dwNewSECMemAddr)
		HeapFree(mHeap, 0, (LPVOID)pPE->dwNewSECMemAddr);

	DWORD dwSize = pSEC->Misc.VirtualSize;
	auto pMem = (LPBYTE)HeapAlloc(mHeap, 0, dwSize);
	auto pDLL = Base + pSEC->VirtualAddress;
	if (!pMem || dwSize < 0x200)
		return PrintStr("分配新区段堆空间错误。");
	memcpy(pMem, pDLL, pSEC->Misc.VirtualSize);

	//调整计算偏移
	DWORD start = (DWORD)GetProcAddress((HMODULE)Base, "start") - (DWORD)Base;
	start -= pSEC->VirtualAddress + 27;
	memcpy(pMem + 15, &pNewSEC->VirtualAddress, 4);
	memcpy(pMem + 23, &start, 4);

	//调整PE结构
	pPE->dwNewSECMemAddr = (DWORD)pMem;
	pPE->dwNewSECSize = dwSize;
	pNewSEC->Misc.VirtualSize = dwSize;
	pNewSEC->SizeOfRawData = dwSize;
 	return TRUE;
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
