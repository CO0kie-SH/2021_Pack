#pragma once
#include "lz4.h"


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
	_In_                          size_t      _Size);

// 字符串
constexpr const char* gszGetProcAddress = "GetProcAddress";
constexpr const char* gszLoadLibraryA = "LoadLibraryA";
constexpr const char* gszGetModuleHandleA = "GetModuleHandleA";
constexpr const char* gszExitProcess = "ExitProcess";
constexpr const char* gszNTDLL = "NTDLL";
constexpr const char* gszmemcpy = "memcpy";
constexpr const char* gszVirtualAlloc = "VirtualAlloc";
constexpr const char* gszVirtualProtect = "VirtualProtect";

// 解密函数的时候使用的 OPCODE
constexpr const char* OpCode = "\xB8\x00\x00\x00\x00\x83\xC0\x01\xFF\xE0";

typedef struct _MyWAPI
{
	HMODULE				ke32 = 0;
	HMODULE				ntdll = 0;
	PGetProcAddress		pGetProcAddress = 0;
	PLoadLibraryA		pLoadLibraryA = 0;
	PGetModuleHandleA	pGetModuleHandleA = 0;
	PExitProcess		pExitProcess = 0;
	Pmemcpy				pmemcpy = 0;
	PVirtualAlloc		pVirtualAlloc = 0;
	PVirtualProtect		pVirtualProtect = 0;
}MyWAPI, * LPMyWAPI;


typedef struct _MyLz4	//压缩结构体
{
	BOOL  Compress;		//压缩方式
	LPCH oldAddr;		//旧地址
	LPCH newAddr;		//新地址
	DWORD oldSize;		//旧大小
	DWORD newSize;		//新大小
}MyLz4, * LPMyLz4;

// 动态的获取自己需要用到的函数，注意这个函数只需要在最起初的时候获取自己的API
	//	时使用，例如获取 LoadLibrary GetProcAddress
DWORD MyGetProcAddress(DWORD Module, LPCSTR FunName);

//EXTERN_C __declspec(dllexport) void* GetBase();
extern __declspec(dllexport) BOOL Lz4Compress(LPMyLz4 pLZ4);

class CData
{
};

