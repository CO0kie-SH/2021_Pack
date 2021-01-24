#pragma once
#include <iostream>
#include <Windows.h>

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
}MyPE, * LPMyPE;


class CPE
{
public:
	CPE();
	~CPE();

	BOOL ReadFile(LPCSTR Path);
	BOOL CheckPE(LPCBYTE pMem);
	BOOL CheckSection(LPMyPE pPE, PIMAGE_SECTION_HEADER* pOut = 0);
	BOOL AddSection(LPMyPE pPE = 0, LPCSTR SavePath = 0);
	BOOL SaveFile(LPCSTR FilePath, LPMyPE pPE, PBYTE buff);
protected:
	DWORD MathOffset(DWORD Addr, DWORD Size);		//����ƫ��
	PIMAGE_NT_HEADERS NtHeader(LPCBYTE pFile);
protected:
	MyPE	mPE;
private:
	HANDLE	mHeap;
	LPBYTE	mpFile;
};

