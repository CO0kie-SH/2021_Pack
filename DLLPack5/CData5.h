#pragma once
#include <Windows.h>

#define NOINLine __declspec(noinline) 
#define DLLEXport __declspec(dllexport) 


// ע�ᵼ������
EXTERN_C DLLEXport void* GetBase();

// ע��ȫ�ֺ���
#pragma region ע��ȫ�ֺ���

// �޸��ض�λ
NOINLine BOOL /*__stdcall*/ FixLOC(DWORD OldImageBase, DWORD NewImageBase,
	DWORD RVA, DWORD newTEXT, DWORD MaxSize);

// ��ӡ�ַ���
BOOL PrintStr(LPCSTR Str, HANDLE FileHandle = 0);

#pragma endregion

// ע��ȫ�ֱ���
extern HMODULE ghModule;

// ע�Ṥ����
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

