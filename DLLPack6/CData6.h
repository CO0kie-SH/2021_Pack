#pragma once
#include <Windows.h>
#include "lz4_6.h"
#include "AES.h"
#include "CDbg6.h"

#define NOINLine __declspec(noinline)
#define DLLEXport __declspec(dllexport)



// 压缩结构体
typedef struct _MyLz4	//压缩结构体
{
	BOOL  Compress;		//压缩方式
	LPCH oldAddr;		//旧地址
	LPCH newAddr;		//新地址
	DWORD oldSize;		//旧大小
	DWORD newSize;		//新大小
}MyLz4, * LPMyLz4;


EXTERN_C DLLEXport void* GetBaseDLL();
EXTERN_C DLLEXport void TextAES(BOOL un, LPBYTE mem, int Len, DWORD key1);
EXTERN_C NOINLine BOOL /*__stdcall*/ FixLOC(DWORD OldImageBase, DWORD NewImageBase,
	DWORD RVA, DWORD newTEXT, DWORD MaxSize);
extern DLLEXport BOOL ReadFile6(LPCSTR Path, DWORD* pFileSize);
extern DLLEXport BOOL SaveFile6(LPCSTR Path, LPCCH pMem, DWORD pFileSize);
extern DLLEXport NOINLine PIMAGE_NT_HEADERS NtHeader(DWORD Dos);
extern DLLEXport NOINLine BOOL Lz4Compress6(LPMyLz4 pLZ4);
extern DLLEXport NOINLine void XorMem6(PBYTE oldAddr, PBYTE newAddr, DWORD Size, DWORD Key);
extern HMODULE ghModule;

// 解密函数的时候使用的 OPCODE
constexpr const char* gIAT = "\xB8\x00\x00\x00\x00\x05\x34\x12\x00\x00\x50\xC3";

class CData6
{
};

