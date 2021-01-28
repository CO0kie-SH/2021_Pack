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
	_In_                          size_t      _Size);

// �ַ���
constexpr const char* gszGetProcAddress = "GetProcAddress";
constexpr const char* gszLoadLibraryA = "LoadLibraryA";
constexpr const char* gszGetModuleHandleA = "GetModuleHandleA";
constexpr const char* gszExitProcess = "ExitProcess";
constexpr const char* gszNTDLL = "NTDLL";
constexpr const char* gszmemcpy = "memcpy";
constexpr const char* gszVirtualAlloc = "VirtualAlloc";
constexpr const char* gszVirtualProtect = "VirtualProtect";

// ���ܺ�����ʱ��ʹ�õ� OPCODE
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


typedef struct _MyLz4	//ѹ���ṹ��
{
	BOOL  Compress;		//ѹ����ʽ
	LPCH oldAddr;		//�ɵ�ַ
	LPCH newAddr;		//�µ�ַ
	DWORD oldSize;		//�ɴ�С
	DWORD newSize;		//�´�С
}MyLz4, * LPMyLz4;

// ��̬�Ļ�ȡ�Լ���Ҫ�õ��ĺ�����ע���������ֻ��Ҫ���������ʱ���ȡ�Լ���API
	//	ʱʹ�ã������ȡ LoadLibrary GetProcAddress
DWORD MyGetProcAddress(DWORD Module, LPCSTR FunName);

//EXTERN_C __declspec(dllexport) void* GetBase();
extern __declspec(dllexport) BOOL Lz4Compress(LPMyLz4 pLZ4);

class CData
{
};

