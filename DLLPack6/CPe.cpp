#include "pch.h"
#include "CPe.h"


bool CPe::Analysis_Pe(char* path)
{
	if (path == 0)
	{
		char str[MAX_PATH];
		if (GetModuleFileNameA(NULL, str, MAX_PATH)
			== 0)
			return false;
		path = str;
	}


	DWORD dwRealSize;
	//1. ���ļ�
	HANDLE hFile = CreateFileA(
		path,
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	//2. ��ȡ�ļ���С
	m_size = GetFileSize(hFile, NULL);
	m_pbuf = new char[m_size] {0};
	//3. ��ȡ�ļ�
	ReadFile(hFile, m_pbuf, m_size, &dwRealSize, NULL);

	PIMAGE_DOS_HEADER pdos = GetDosHeader();
	if (pdos->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return false;
	}
	// ��ȡNT
	PIMAGE_NT_HEADERS pNt = GetNtHeader();
	if (pNt->Signature != IMAGE_NT_SIGNATURE)
	{
		return false;
	}
	return true;

	CloseHandle(hFile);
}

bool CPe::Analysis_Pe(char* pMem, int size)
{
	m_size = size;
	m_pbuf = pMem;
	return true;
}

PIMAGE_DOS_HEADER  CPe::GetDosHeader()
{
	return (IMAGE_DOS_HEADER*)m_pbuf;
}

PIMAGE_NT_HEADERS  CPe::GetNtHeader()
{
	return (PIMAGE_NT_HEADERS)(GetDosHeader()->e_lfanew + m_pbuf);
}

PIMAGE_FILE_HEADER CPe::GetFileHeader()
{
	return &GetNtHeader()->FileHeader;
}

PIMAGE_OPTIONAL_HEADER CPe::GetOptionalHeander()
{
	return &GetNtHeader()->OptionalHeader;
}

PIMAGE_DATA_DIRECTORY CPe::GetDataDirectory(int index)
{
	return &GetOptionalHeander()->DataDirectory[index];
}

PIMAGE_SECTION_HEADER CPe::GetSectionHeader()
{
	PIMAGE_NT_HEADERS pNt = GetNtHeader();
	return IMAGE_FIRST_SECTION(pNt);
}

PVOID CPe::GetPeBuff()
{
	return (PVOID)m_pbuf;
}


DWORD CPe::RvaToOffset(DWORD dwRva)
{
	//1 �Ȼ�ȡ����ͷ���ĵ�ַ
	PIMAGE_NT_HEADERS32 pNT32 = GetNtHeader();
	PIMAGE_FILE_HEADER pFileHeader = &pNT32->FileHeader;
	PIMAGE_SECTION_HEADER pSeciton = IMAGE_FIRST_SECTION(pNT32);
	//2 ѭ������������Ϣ
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		//1. �ж��Ƿ���ĳһ��������
		// ע�⣺�����жϷ�Χ��ʱ��Ӧ��ʹ�õ����ļ���С
		if (dwRva >= pSeciton->VirtualAddress &&
			dwRva < pSeciton->VirtualAddress + pSeciton->SizeOfRawData)
		{
			//2. ��������
			DWORD dwChazhi = dwRva - pSeciton->VirtualAddress;

			return pSeciton->PointerToRawData + dwChazhi;
		}
		pSeciton++;
	}
}

tm CPe::PeFileTimeToLocalTime(DWORD dwtime)
{
	tm tm1;
	__time64_t  ftime1 = (__time64_t)dwtime;
	_localtime64_s(&tm1, &ftime1);
	return tm1;

}

PIMAGE_BASE_RELOCATION CPe::GetRelocation()
{
	// ת��OFFSET
	DWORD Rva = GetDataDirectory(5)->VirtualAddress;
	if (Rva == 0)
		return NULL;

	DWORD offset = RvaToOffset(Rva);
	return (PIMAGE_BASE_RELOCATION)(offset + m_pbuf);

}


bool CPe::FixRelocation(DWORD NewImageBase)
{
	//ÿһ���ض�λ���ݶ���һ���ṹ�壬��¼���ͣ���ƫ��
	typedef struct TYPEOFFSET {
		WORD Offset : 12;			//һҳ�ֵ�ƫ��
		WORD Type : 4;			//�ض�λ�������ͣ�3��ʾ���������Ҫ�ض�λ
	}*PTYPEOFFSET;


	//1. ��ȡ�ض�λ��
	PIMAGE_BASE_RELOCATION pRelocation = GetRelocation();
	//2. �����ض�λ
	//�ض�λ������һ��ȫΪ���β����
	while (pRelocation->SizeOfBlock != 0)
	{
		//3.�����ض�λ��
		//�ض�λ������0x1000ҳΪһ�飬ÿһ�鸺��һҳ
		//ÿһҳ���ж��ٿ� (sizeblock - 8) /2
		DWORD dwCount = (pRelocation->SizeOfBlock - 8) / 2;
		for (int i = 0; i < dwCount; i++)
		{
			PTYPEOFFSET pBlock = (PTYPEOFFSET)(pRelocation + 1);
			//���������Ҫ�ض�λ��
			if (pBlock[i].Type == 3)
			{
				//��Ҫ�ض�λ���ݵ�λ��  ��RVA��
				DWORD RvaOffset = pRelocation->VirtualAddress + pBlock[i].Offset;
				//�ļ��ֵ�λ��  (FOA)
				DWORD FoaOffset = RvaToOffset(RvaOffset);
				//��Ҫ�ض�λ�����ݾ���
				DWORD Data = *(DWORD*)(FoaOffset + m_pbuf);
				// �޸�����  ������ - Ĭ�ϼ��ػ�ַ + �¼��ػ�ַ
				DWORD NewData = Data - 0x00400000 + NewImageBase;
				*(DWORD*)(FoaOffset + m_pbuf) = NewData;
			}
		}
		//�ҵ���һ���ض�λ��
		pRelocation =
			(PIMAGE_BASE_RELOCATION)((DWORD)pRelocation + pRelocation->SizeOfBlock);
	}
	return true;
}