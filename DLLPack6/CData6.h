#pragma once
#include <Windows.h>
#include "lz4_6.h"
#include "AES.h"
#include "CDbg6.h"

#define NOINLine __declspec(noinline)
#define DLLEXport __declspec(dllexport)



// ѹ���ṹ��
typedef struct _MyLz4	//ѹ���ṹ��
{
	BOOL  Compress;		//ѹ����ʽ
	LPCH oldAddr;		//�ɵ�ַ
	LPCH newAddr;		//�µ�ַ
	DWORD oldSize;		//�ɴ�С
	DWORD newSize;		//�´�С
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

// ���ܺ�����ʱ��ʹ�õ� OPCODE
constexpr const char* gIAT = "\xB8\x00\x00\x00\x00\x05\x34\x12\x00\x00\x50\xC3";

class CData6
{
};

