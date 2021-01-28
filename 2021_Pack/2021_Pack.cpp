// 2021_Pack.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "CPE2.h"
#define FPath "D:\\cacheD\\202101\\demo.exe"
#define FPath2 "D:\\cacheD\\202101\\demo.new.exe"


BOOL ReadFile(LPCSTR Path, DWORD* pFileSize);
BOOL SaveFile(LPCSTR Path, LPCCH pMem, DWORD pFileSize);

int main()
{
    std::cout << "Hello World!\n";
	DWORD FileSize = 0;
	LPCBYTE pFile = (LPCBYTE)ReadFile(FPath, &FileSize);
	if (pFile == 0 || FileSize < 0x400)
		return 0;
	CPE2 pe(pFile, FileSize);
}
