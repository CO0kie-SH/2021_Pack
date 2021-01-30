#pragma once
#include <time.h>
class CPe
{
public:
	// ����PE
	bool Analysis_Pe(char* path);
	bool Analysis_Pe(char* pMem, int size);
	// ��ȡDOSͷ
	PIMAGE_DOS_HEADER  GetDosHeader();
	//��ȡNTͷ
	PIMAGE_NT_HEADERS  GetNtHeader();
	// ��ȡ�ļ�ͷ
	PIMAGE_FILE_HEADER GetFileHeader();
	// ��ȡ��չͷ
	PIMAGE_OPTIONAL_HEADER GetOptionalHeander();
	// ��ȡ����Ŀ¼��
	PIMAGE_DATA_DIRECTORY GetDataDirectory(int index);
	// ����ͷ��
	PIMAGE_SECTION_HEADER GetSectionHeader();
	// ��ȡ������
	PVOID GetPeBuff();

	DWORD RvaToOffset(DWORD dwRva);

	// ʱ��ת��
	tm PeFileTimeToLocalTime(DWORD dwtime);

	// ��ȡ�ض�λ��
	PIMAGE_BASE_RELOCATION GetRelocation();
	// �޸��ض�λ��
	bool FixRelocation(DWORD NewBase);









private:
	char* m_pbuf;
	int   m_size;

};

