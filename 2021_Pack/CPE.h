#pragma once
#include <iostream>
#include <Windows.h>

#define DllPath "D:\\cacheD\\202101\\2021_Pack\\Release\\Dll_ShellCode.dll"

BOOL PrintStr(LPCSTR Str, HANDLE FileHandle = 0);

typedef struct _MyPE			//PE文件解析
{
	DWORD	FileSize;			//文件大小
	DWORD	FileMemAddr;		//文件内存地址
	LPWORD	NumberOfSections;	//区段数量
	LPDWORD	SizeOfHeaders;		//所有头部大小（区段偏移）
	LPDWORD	SizeOfImage;		//镜像大小
	LPDWORD	SectionAlignment;	//内存对齐
	LPDWORD	FileAlignment;		//文件对齐
	LPDWORD	AddressOfEntryPoint;//起始地址

	DWORD	dwNewSECMemAddr;	//新区段内存位置
	DWORD	dwNewSECSize;		//新区段大小
}MyPE, * LPMyPE;


class CPE
{
public:
	CPE();
	~CPE();

	BOOL ReadFile(LPCSTR Path);
	BOOL CheckPE(LPCBYTE pMem, LPMyPE pPE);
	BOOL CheckSection(LPCBYTE Base);
	BOOL AddSection(LPMyPE pPE = 0, LPCSTR SavePath = 0);
	BOOL SaveFile(LPCSTR FilePath, LPMyPE pPE, PBYTE buff);
	BOOL LoadDLL(LPMyPE pPE, PIMAGE_SECTION_HEADER pNewSEC);
	BOOL FixDllStub(LPCBYTE Base, LPCBYTE pMem, DWORD dwNewSEC);
protected:
	DWORD MathOffset(DWORD Addr, DWORD Size);		//计算偏移
	PIMAGE_NT_HEADERS NtHeader(LPCBYTE pFile);
protected:
	MyPE	mPE;
private:
	HANDLE	mHeap;
	LPBYTE	mpFile;
};

