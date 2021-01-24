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
	if (mpFile)
		free(mpFile);
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
	mpFile = (LPBYTE)malloc(FileSize * sizeof(BYTE));
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
	return CheckPE(mpFile);					//初始化PE
}

BOOL CPE::CheckPE(LPCBYTE pMem)
{
	auto pNT = NtHeader(pMem);				//PE头
	if (NtHeader(mpFile)->Signature != IMAGE_NT_SIGNATURE)
		return PrintStr("PE头错误。");


	auto& pFH = pNT->FileHeader;			//文件头
	printf("  文件头：\n区段的个数：%u\n扩展头大小：0x%X\n文件的标志：0x%X\n",
		pFH.NumberOfSections, pFH.SizeOfOptionalHeader, pFH.Characteristics);

	auto& pOH = pNT->OptionalHeader;
	printf("  扩展头：\n魔术字：0x%X\n代码大小：0x%X\n入口点处：【0x%X】\n\n" \
		"代码段起始：0x%X\n数据段起始：0x%X\n镜像大小：【0x%lX】\n" \
		"默认加载基址：0x%lX\n内存对齐：0x%lX\n文件对齐：0x%lX\n" \
		"所有头部大小：0x%lX\n区段偏移：->0x%p\nDLL标志：0x%X\n",
		pOH.Magic, pOH.SizeOfCode, pOH.AddressOfEntryPoint,
		pOH.BaseOfCode, pOH.BaseOfData, pOH.SizeOfImage,
		pOH.ImageBase, pOH.SectionAlignment, pOH.FileAlignment,
		pOH.SizeOfHeaders, pMem + pOH.SizeOfHeaders, pOH.DllCharacteristics);

	//保存PE结构体内存地址
	mPE.NumberOfSections = &pFH.NumberOfSections;
	mPE.SizeOfHeaders = &pOH.SizeOfHeaders;
	mPE.SizeOfImage = &pOH.SizeOfImage;
	mPE.SectionAlignment = &pOH.SectionAlignment;
	mPE.FileAlignment = &pOH.FileAlignment;
	mPE.AddressOfEntryPoint = &pOH.AddressOfEntryPoint;
	return CheckSection(&mPE);
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
	PBYTE buff = (PBYTE)HeapAlloc(mHeap, HEAP_ZERO_MEMORY, 200);
	if (!buff)
		return PrintStr("无法创建堆控制");
	memcpy(buff, "\x90\x90\x90\x90", 4);

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
		return PrintStr("文件打开错误。");

	//写入文件
	DWORD dwLen = 0, dwLen2 = 0, dwRet = 0;
	WriteFile(FileHandle, (LPVOID)pPE->FileMemAddr, pPE->FileSize, &dwLen, 0);
	WriteFile(FileHandle, (LPVOID)buff, 0x200, &dwLen2, 0);
	CloseHandle(FileHandle);

	//判断成功
	dwRet = dwLen + dwLen2 == pPE->FileSize + 0x200;
	printf("  写入文件：%s\n", dwRet ? "成功" : "失败");
	CheckSection(pPE);
	return dwRet;
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
