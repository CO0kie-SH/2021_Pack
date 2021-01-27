#pragma once
#include <iostream>
#include <Windows.h>

#define DllPath "D:\\cacheD\\202101\\2021_Pack\\Release\\Dll_ShellCode.dll"

BOOL PrintStr(LPCSTR Str, HANDLE FileHandle = 0);

typedef struct _MyPE			//PE�ļ�����
{
	DWORD	FileSize;			//�ļ���С
	DWORD	FileMemAddr;		//�ļ��ڴ��ַ
	LPWORD	NumberOfSections;	//��������
	LPDWORD	SizeOfHeaders;		//����ͷ����С������ƫ�ƣ�
	LPDWORD	SizeOfImage;		//�����С
	LPDWORD	SectionAlignment;	//�ڴ����
	LPDWORD	FileAlignment;		//�ļ�����
	LPDWORD	AddressOfEntryPoint;//��ʼ��ַ

	DWORD	dwNewSECMemAddr;	//�������ڴ�λ��
	DWORD	dwNewSECSize;		//�����δ�С
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
	DWORD MathOffset(DWORD Addr, DWORD Size);		//����ƫ��
	PIMAGE_NT_HEADERS NtHeader(LPCBYTE pFile);
protected:
	MyPE	mPE;
private:
	HANDLE	mHeap;
	LPBYTE	mpFile;
};

