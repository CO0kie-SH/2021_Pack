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
	//1. 打开文件
	HANDLE hFile = CreateFileA(
		path,
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	//2. 获取文件大小
	m_size = GetFileSize(hFile, NULL);
	m_pbuf = new char[m_size] {0};
	//3. 读取文件
	ReadFile(hFile, m_pbuf, m_size, &dwRealSize, NULL);

	PIMAGE_DOS_HEADER pdos = GetDosHeader();
	if (pdos->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return false;
	}
	// 获取NT
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
	//1 先获取各种头部的地址
	PIMAGE_NT_HEADERS32 pNT32 = GetNtHeader();
	PIMAGE_FILE_HEADER pFileHeader = &pNT32->FileHeader;
	PIMAGE_SECTION_HEADER pSeciton = IMAGE_FIRST_SECTION(pNT32);
	//2 循环遍历区段信息
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		//1. 判断是否在某一个区段中
		// 注意：我们判断范围的时候，应该使用的是文件大小
		if (dwRva >= pSeciton->VirtualAddress &&
			dwRva < pSeciton->VirtualAddress + pSeciton->SizeOfRawData)
		{
			//2. 在区段中
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
	// 转换OFFSET
	DWORD Rva = GetDataDirectory(5)->VirtualAddress;
	if (Rva == 0)
		return NULL;

	DWORD offset = RvaToOffset(Rva);
	return (PIMAGE_BASE_RELOCATION)(offset + m_pbuf);

}


bool CPe::FixRelocation(DWORD NewImageBase)
{
	//每一个重定位数据都是一个结构体，记录类型，与偏移
	typedef struct TYPEOFFSET {
		WORD Offset : 12;			//一页种的偏移
		WORD Type : 4;			//重定位数据类型，3表示这个数据需要重定位
	}*PTYPEOFFSET;


	//1. 获取重定位表
	PIMAGE_BASE_RELOCATION pRelocation = GetRelocation();
	//2. 遍历重定位
	//重定位表是以一项全为零结尾的数
	while (pRelocation->SizeOfBlock != 0)
	{
		//3.遍历重定位项
		//重定位项是以0x1000页为一块，每一块负责一页
		//每一页有有多少块 (sizeblock - 8) /2
		DWORD dwCount = (pRelocation->SizeOfBlock - 8) / 2;
		for (int i = 0; i < dwCount; i++)
		{
			PTYPEOFFSET pBlock = (PTYPEOFFSET)(pRelocation + 1);
			//这个数据需要重定位吗？
			if (pBlock[i].Type == 3)
			{
				//需要重定位数据的位置  （RVA）
				DWORD RvaOffset = pRelocation->VirtualAddress + pBlock[i].Offset;
				//文件种的位置  (FOA)
				DWORD FoaOffset = RvaToOffset(RvaOffset);
				//需要重定位的数据据是
				DWORD Data = *(DWORD*)(FoaOffset + m_pbuf);
				// 修复数据  将数据 - 默认加载基址 + 新加载基址
				DWORD NewData = Data - 0x00400000 + NewImageBase;
				*(DWORD*)(FoaOffset + m_pbuf) = NewData;
			}
		}
		//找到下一个重定位表
		pRelocation =
			(PIMAGE_BASE_RELOCATION)((DWORD)pRelocation + pRelocation->SizeOfBlock);
	}
	return true;
}