//#include "CPE5.h"
//
//
//CPE5::CPE5()
//{
//	// 读取文件
//	DWORD FileSize = 0;
//	auto pFile = (PBYTE)ReadFile5("..\\Debug\\demo.exe", &FileSize);
//	
//	// 加密文件
//	auto pXorMem = (PBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize);
//	if (pXorMem == 0)	return;
//	XorMem5(pFile, pXorMem, FileSize, 0x34333231);
//
//	// 压缩文件
//	MyLz4 lz4 = {
//		TRUE,(LPCH)pXorMem,0,FileSize
//	};
//	BOOL bCom = Lz4Compress(&lz4);
//	if (!bCom)	return;
//
//	// 保存加密文件备份
//	SaveFile5("..\\Debug\\demo.xor.lz4",
//		lz4.newAddr, lz4.newSize + 8);
//
//	SaveEXE(pFile, &lz4);
//}
//
//CPE5::~CPE5()
//{
//}
//
//BOOL CPE5::SaveEXE(LPCBYTE oldFile, LPMyLz4 lz4)
//{
//	auto fNT = NtHeader((PIMAGE_DOS_HEADER)oldFile);
//	auto fOH = &fNT->OptionalHeader;
//
//	auto dNT = NtHeader((PIMAGE_DOS_HEADER)GetBase());
//	auto dOH = &dNT->OptionalHeader;
//
//	DWORD fileCode = fOH->SizeOfHeaders;	//段头字节数
//	DWORD nowSize = lz4->oldSize;			//内存最大值
//	DWORD dllSize = dOH->SizeOfImage - dOH->BaseOfCode;	//DLL空间大小
//	// 总大小 = 原头文件 + DLL大小 + 8字 + 压缩包 
//	DWORD newSize = fileCode + dllSize + 8 + lz4->newSize;
//
//	// 判断堆溢出
//	if (newSize > nowSize)
//	{
//		MessageBoxA(0, "内存过于小，无法容纳", 0, 0);
//		return 0;
//	}
//	LPBYTE newMem = (LPBYTE)lz4->oldAddr;
//	LPBYTE dllCode = (LPBYTE)GetBase()+ dOH->BaseOfCode;
//
//	// 清空内存
//	ZeroMemory(newMem, newSize);
//
//	// 复制文件头
//	memcpy(newMem, oldFile, fileCode);
//	newMem += fileCode;
//
//	// 复制DLL段
//	memcpy(newMem, dllCode, dllSize);
//	newMem += dllSize;
//
//	// 复制DLL段
//	memcpy(newMem, lz4->newAddr, lz4->newSize + 8);
//	if (RePE(lz4->oldAddr, newSize) == 0)	return 0;
//
//	BOOL bSave = SaveFile5("..\\Debug\\demo.new.exe",
//		lz4->oldAddr, newSize);
//	return bSave;
//}
//
//BOOL CPE5::RePE(LPCH newMem, DWORD newSize)
//{
//	auto dNT = NtHeader((PIMAGE_DOS_HEADER)GetBase());
//	auto dOH = &dNT->OptionalHeader;
//	auto& dDir = dOH->DataDirectory;
//
//	auto nNT = NtHeader((PIMAGE_DOS_HEADER)newMem);
//	auto nOH = &nNT->OptionalHeader;
//	auto& nDir = nOH->DataDirectory;
//	auto nSEC = IMAGE_FIRST_SECTION(nNT);
//
//	DWORD tmp = nNT->FileHeader.NumberOfSections,
//		IATVA = dDir[1].VirtualAddress,
//		IATSize = dDir[1].Size;
//	ZeroMemory(nSEC + 1, sizeof(IMAGE_SECTION_HEADER) * tmp);
//	nNT->FileHeader.NumberOfSections = 1;
//	ZeroMemory(nDir, sizeof(IMAGE_DATA_DIRECTORY) * 16);
//
//	//DLL壳的导入表
//	nDir[1].VirtualAddress = IATVA;
//	nDir[1].Size = IATSize;
//
//	// 扩展头修改
//	tmp = (DWORD)GetProcAddress((HMODULE)GetBase(), "start");
//	nOH->BaseOfData = 0;
//	nOH->SizeOfCode = 0;
//	nOH->AddressOfEntryPoint = tmp - dOH->ImageBase;
//	tmp = newSize - nOH->SizeOfHeaders;		//除去段首的大小
//	nOH->SizeOfImage = tmp + nOH->BaseOfCode;
//
//	// 区段属性修改
//	memcpy(nSEC->Name, ".CO0kie", 8);
//	nSEC->SizeOfRawData = tmp;
//	nSEC->Misc.VirtualSize = tmp;
//	return TRUE;
//}
