#pragma once
#include <Windows.h>
//#include <iostream>
#include "lz4_5.h"

#define NOINLine __declspec(noinline) 
#define DLLEXport __declspec(dllexport) 


#pragma region 函数指针
// 创建一个函数指针类型并创建指针
using PGetProcAddress = FARPROC(WINAPI*)(
	_In_ HMODULE hModule,
	_In_ LPCSTR lpProcName);

using PLoadLibraryA = HMODULE(WINAPI*)(
	_In_ LPCSTR lpLibFileName);

using PGetModuleHandleA = HMODULE(WINAPI*)(
	_In_opt_ LPCSTR lpModuleName);

using PExitProcess = void(WINAPI*)(
	_In_ UINT uExitCode);

using PVirtualAlloc = LPVOID(WINAPI*)(
	_In_opt_ LPVOID lpAddress,
	_In_     SIZE_T dwSize,
	_In_     DWORD flAllocationType,
	_In_     DWORD flProtect);

using PVirtualProtect = BOOL(WINAPI*)(
	_In_  LPVOID lpAddress,
	_In_  SIZE_T dwSize,
	_In_  DWORD flNewProtect,
	_Out_ PDWORD lpflOldProtect);

using Pmemcpy = BOOL(__cdecl*)(
	_Out_writes_bytes_all_(_Size) void* _Dst,
	_In_reads_bytes_(_Size)       void const* _Src,
	_In_                          size_t      _Size
	);

using Pmemset = void* (__cdecl*)(
	_Out_writes_bytes_all_(_Size) void* _Dst,
	_In_                          int    _Val,
	_In_                          size_t _Size
	);

using Pstrcmp = int (__cdecl*)(
		_In_z_ char const* _Str1,
		_In_z_ char const* _Str2
	);

using PGetProcessHeap = HANDLE(WINAPI*)(
	VOID);

using PHeapAlloc = LPVOID(WINAPI*)(
	_In_ HANDLE hHeap,
	_In_ DWORD dwFlags,
	_In_ SIZE_T dwBytes
	);

using PHeapFree = BOOL(WINAPI*)(
	_Inout_ HANDLE hHeap,
	_In_ DWORD dwFlags,
	__drv_freesMem(Mem) _Frees_ptr_opt_ LPVOID lpMem
	);

// 结构体
typedef struct _MyBases
{
	HMODULE k32;
	HMODULE u32;
	HMODULE ntdll;
	HMODULE exeBase;
}MyBases, * LPMyBases;

// 结构体
typedef struct _MyKe32
{
	PGetProcAddress		pGetProcAddress = 0;
	PLoadLibraryA		pLoadLibraryA = 0;
	PGetModuleHandleA	pGetModuleHandleA = 0;
	PExitProcess		pExitProcess = 0;
}MyKe32, * LPMyKe32;

// 结构体
typedef struct _MyNTDLL
{
	Pmemset		pmemset = 0;
	Pmemcpy		pmemcpy = 0;
	Pstrcmp		pstrcmp = 0;
}MyNTDLL, * LPMyNTDLL;


// 字符串
constexpr const char* gszGetProcAddress = "GetProcAddress";
constexpr const char* gszLoadLibraryA = "LoadLibraryA";
constexpr const char* gszGetModuleHandleA = "GetModuleHandleA";
constexpr const char* gszExitProcess = "ExitProcess";
constexpr const char* gszNTDLL = "NTDLL";
constexpr const char* gszmemcpy = "memcpy";
constexpr const char* gszmemset = "memset";
constexpr const char* gszstrcmp = "strcmp";
constexpr const char* gszVirtualAlloc = "VirtualAlloc";
constexpr const char* gszVirtualProtect = "VirtualProtect";
constexpr const char* gszGetProcessHeap = "GetProcessHeap";
constexpr const char* gszHeapAlloc = "HeapAlloc";
constexpr const char* gszHeapFree = "HeapFree";
constexpr const char* gszUSER32 = "USER32";

#pragma endregion

// 解密函数的时候使用的 OPCODE
constexpr const char* OpCode = "\xB8\x00\x00\x00\x00\x83\xC0\x01\xFF\xE0";

// 压缩结构体
typedef struct _MyLz4	//压缩结构体
{
	BOOL  Compress;		//压缩方式
	LPCH oldAddr;		//旧地址
	LPCH newAddr;		//新地址
	DWORD oldSize;		//旧大小
	DWORD newSize;		//新大小
}MyLz4, * LPMyLz4;


// 注册导出函数
EXTERN_C DLLEXport void* GetBase();
extern DLLEXport BOOL ReadFile5(LPCSTR Path, DWORD* pFileSize);
extern DLLEXport BOOL SaveFile5(LPCSTR Path, LPCCH pMem, DWORD pFileSize);
extern DLLEXport void XorMem5(PBYTE oldAddr, PBYTE newAddr, DWORD Size, DWORD Key);
extern DLLEXport BOOL Lz4Compress(LPMyLz4 pLZ4);
extern DLLEXport PIMAGE_NT_HEADERS NtHeader(PIMAGE_DOS_HEADER Dos);

// 注册全局函数
#pragma region 注册全局函数

// 修复重定位
EXTERN_C NOINLine BOOL /*__stdcall*/ FixLOC(DWORD OldImageBase, DWORD NewImageBase,
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
	NOINLine CData5();
	NOINLine int myStrcmp(LPSTR str1, LPSTR str2);

public:
	HMODULE k32;
	HMODULE u32;
	HMODULE ntdll;
	HMODULE exeBase;

	MyKe32	K32s;
	MyNTDLL	NTs;
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

