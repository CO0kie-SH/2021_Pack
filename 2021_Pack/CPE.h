#pragma once
#include <iostream>
#include <Windows.h>

#define DllPath "C:\\Users\\CY-Pro13\\source\\repos\\2021_Pack\\Release\\Dll_ShellCode.dll"
#define DllPath2 "D:\\cacheD\\mini_pack\\mini_pack - v0.01 ��������\\test\\mini_stub.dll"


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
	BOOL CheckSection(LPMyPE pPE, PIMAGE_SECTION_HEADER* pOut = 0);
	BOOL AddSection(LPMyPE pPE = 0, LPCSTR SavePath = 0);
	BOOL SaveFile(LPCSTR FilePath, LPMyPE pPE, PBYTE buff);
	BOOL LoadDLL(LPMyPE pPE, PIMAGE_SECTION_HEADER pNewSEC);
protected:
	DWORD MathOffset(DWORD Addr, DWORD Size);		//����ƫ��
	PIMAGE_NT_HEADERS NtHeader(LPCBYTE pFile);
protected:
	MyPE	mPE;
private:
	HANDLE	mHeap;
	LPBYTE	mpFile;
};

