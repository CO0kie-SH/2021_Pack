#include "CPE2.h"

BOOL ReadFile(LPCSTR Path, DWORD* pFileSize)
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
	auto mpFile = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize * sizeof(BYTE));
	if (!mpFile)
		return PrintStr("分配内存错误。", FileHandle);

	// 读取文件
	memcpy(mpFile, buff, 0x40);
	if (::ReadFile(FileHandle, (LPVOID)(mpFile + 0x40), FileSize, &dwRead, NULL)
		== FALSE || dwRead != FileSize - 0x40)
		return PrintStr("文件读取错误。", FileHandle);

	// 关闭句柄，防止泄露
	CloseHandle(FileHandle);
	printf("打开文件成功，文件大小：%lu，0x%p\n", FileSize == dwRead, mpFile);
	*pFileSize = FileSize;
	return (int)mpFile;
}

BOOL SaveFile(LPCSTR Path, LPCCH pMem, DWORD pFileSize)
{
	// 以只读的方式打开指定的文件，要求文件存在
	HANDLE FileHandle = CreateFileA(Path, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("新文件打开错误。");
	DWORD dwLen = 0;
	WriteFile(FileHandle, pMem, pFileSize, &dwLen, 0);
	// 关闭句柄，防止泄露
	CloseHandle(FileHandle);
	return pFileSize == dwLen;
}

CPE2::CPE2(LPCBYTE pFile, DWORD Size)
{
	this->pFile = (DWORD)pFile;
	this->mFileSize = Size;
	mpNT = NtHeader(pFile);
	if (mpNT->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBoxA(NULL, "这不是一个有效的PE文件", "错误", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}
	mpOH = &mpNT->OptionalHeader;
	mpPack = (HMODULE)GetBase();
	Lz4Mem();
}

CPE2::~CPE2()
{
}

BOOL CPE2::Lz4Mem()
{
	//MyLz4 lz4 = {
	//	TRUE,(LPCH)pFile,0,mFileSize
	//};
	//BOOL bCom = Lz4Compress(&lz4);
	//int src_file_size = lz4.oldSize;
	//MyLz4 lz42 = {
	//	FALSE,lz4.newAddr
	//};
	//BOOL bCom2 = Lz4Compress(&lz42);
	//SaveFile("D:\\cacheD\\202101\\2021_Pack\\Debug\\demo.lz4",
	//	lz4.newAddr, lz4.newSize + 8);
	return 0;
}

PIMAGE_NT_HEADERS CPE2::NtHeader(LPCBYTE pFile)
{
	PLONG pAdd = ((PLONG)pFile + 15);
	return (PIMAGE_NT_HEADERS)(pFile + *pAdd);
}