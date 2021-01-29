#include "pch.h"
#include "CData6.h"

HMODULE ghModule = 0;

DLLEXport void* GetBaseDLL()
{
	return ghModule;
}

DLLEXport NOINLine PIMAGE_NT_HEADERS NtHeader(DWORD Dos)
{
	return (PIMAGE_NT_HEADERS)(Dos +
		((PIMAGE_DOS_HEADER)Dos)->e_lfanew);
}

BOOL PrintStr(LPCSTR Str, HANDLE FileHandle = 0)
{
	//puts(Str);
	if (FileHandle)
		CloseHandle(FileHandle);
	return 0;
}

BOOL ReadFile6(LPCSTR Path, DWORD* pFileSize)
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
	//printf("打开文件成功，文件大小：%lu，0x%p\n", FileSize == dwRead, mpFile);
	*pFileSize = FileSize;
	return (int)mpFile;
}

BOOL SaveFile6(LPCSTR Path, LPCCH pMem, DWORD pFileSize)
{
	// 以只读的方式打开指定的文件，要求文件存在
	HANDLE FileHandle = CreateFileA(Path, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("新文件打开错误。");
	DWORD dwLen = 0;
	WriteFile(FileHandle, pMem, pFileSize, &dwLen, 0);
	CloseHandle(FileHandle);
	return pFileSize == dwLen;
}

void XorMem6(PBYTE oldAddr, PBYTE newAddr, DWORD Size, DWORD Key)
{
	PBYTE bkey = (PBYTE)&Key;
	for (DWORD i = 0; i < Size; i++)
	{
		newAddr[i] = oldAddr[i] ^ bkey[0] ^ bkey[1] ^ bkey[2] ^ bkey[3];
	}
}

BOOL Lz4Compress6(LPMyLz4 pLZ4)
{
	int oldSize, newSize, isCompress = pLZ4->Compress;
	char* pBuff, * oldAddr, * newAddr = 0;
	auto hHeap = GetProcessHeap();

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
	pBuff = (PCH)HeapAlloc(hHeap, 0, newSize);
	if (!pBuff)	return 0;

	if (isCompress)
	{
		newSize = LZ4_compress(
			oldAddr,/*压缩前的数据*/
			pBuff,	/*压缩后的数据*/
			oldSize	/*文件原始大小*/);
		newAddr = (PCH)HeapAlloc(hHeap, 0, newSize + 8);
		if (newAddr)	//分配地址成功
		{
			LPDWORD size = (LPDWORD)newAddr;
			size[0] = oldSize;
			size[1] = newSize;
			memcpy(newAddr + 8, pBuff, newSize);
			pLZ4->newAddr = newAddr;
			pLZ4->newSize = newSize;
		}
		HeapFree(hHeap, 0, pBuff);
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