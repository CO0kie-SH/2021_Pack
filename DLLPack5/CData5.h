#pragma once
#include <Windows.h>

#define NOINLine __declspec(noinline) 
#define DLLEXport __declspec(dllexport) 


// 注册导出函数
EXTERN_C DLLEXport void* GetBase();

// 注册全局函数
#pragma region 注册全局函数

// 修复重定位
NOINLine BOOL /*__stdcall*/ FixLOC(DWORD OldImageBase, DWORD NewImageBase,
	DWORD RVA, DWORD newTEXT, DWORD MaxSize);

// 打印字符串
BOOL PrintStr(LPCSTR Str, HANDLE FileHandle = 0);

#pragma endregion

// 注册全局变量
extern HMODULE ghModule;

// 注册工具类
class CData5;
class CWind;
class CCMyFile;




class CData5
{
public:
	CData5();

private:

};

class Wind
{
public:
	Wind();

private:

};

class CMyFile
{
public:
	CMyFile();
	DWORD ReadOldEXE();
private:

};

