#pragma once
#include "lz4.h"


// ����һ������ָ�����Ͳ�����ָ��
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

// �ַ���
constexpr const char* gszGetProcAddress = "GetProcAddress";
constexpr const char* gszLoadLibraryA = "LoadLibraryA";
constexpr const char* gszGetModuleHandleA = "GetModuleHandleA";
constexpr const char* gszExitProcess = "ExitProcess";
constexpr const char* gszNTDLL = "NTDLL";
constexpr const char* gszmemcpy = "memcpy";
constexpr const char* gszmemset = "memset";
constexpr const char* gszVirtualAlloc = "VirtualAlloc";
constexpr const char* gszVirtualProtect = "VirtualProtect";
constexpr const char* gszGetProcessHeap = "GetProcessHeap";
constexpr const char* gszHeapAlloc = "HeapAlloc";
constexpr const char* gszHeapFree = "HeapFree";

// ���ܺ�����ʱ��ʹ�õ� OPCODE
constexpr const char* OpCode = "\xB8\x00\x00\x00\x00\x83\xC0\x01\xFF\xE0";

typedef struct _MyWAPI
{
	HMODULE				ke32 = 0;
	HMODULE				ntdll = 0;
	HANDLE				gHeap = 0;
	PGetProcAddress		pGetProcAddress = 0;
	PLoadLibraryA		pLoadLibraryA = 0;
	PGetModuleHandleA	pGetModuleHandleA = 0;
	PExitProcess		pExitProcess = 0;
	Pmemcpy				pmemcpy = 0;
	Pmemset				pmemset = 0;
	PVirtualAlloc		pVirtualAlloc = 0;
	PVirtualProtect		pVirtualProtect = 0;
	PGetProcessHeap		pGetProcessHeap = 0;
	PHeapAlloc			pHeapAlloc = 0;
	PHeapFree			pHeapFree = 0;
}MyWAPI, * PMyWAPI, * LPMyWAPI;


typedef struct _MyLz4	//ѹ���ṹ��
{
	BOOL  Compress;		//ѹ����ʽ
	LPCH oldAddr;		//�ɵ�ַ
	LPCH newAddr;		//�µ�ַ
	DWORD oldSize;		//�ɴ�С
	DWORD newSize;		//�´�С
}MyLz4, * LPMyLz4;

#define NOINLine __declspec(noinline) 

//EXTERN_C __declspec(dllexport) void* GetBase();
extern __declspec(dllexport) NOINLine BOOL Lz4Compress(LPMyLz4 pLZ4);
extern NOINLine BOOL LoadAPI();
extern PMyWAPI gAPI;

// ��̬�Ļ�ȡ�Լ���Ҫ�õ��ĺ�����ע���������ֻ��Ҫ���������ʱ���ȡ�Լ���API
	//	ʱʹ�ã������ȡ LoadLibrary GetProcAddress
NOINLine DWORD MyGetProcAddress(DWORD Module, LPCSTR FunName);


class CData
{
};

