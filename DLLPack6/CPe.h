#pragma once
#include <time.h>
class CPe
{
public:
	// 解析PE
	bool Analysis_Pe(char* path);
	bool Analysis_Pe(char* pMem, int size);
	// 获取DOS头
	PIMAGE_DOS_HEADER  GetDosHeader();
	//获取NT头
	PIMAGE_NT_HEADERS  GetNtHeader();
	// 获取文件头
	PIMAGE_FILE_HEADER GetFileHeader();
	// 获取扩展头
	PIMAGE_OPTIONAL_HEADER GetOptionalHeander();
	// 获取数据目录表
	PIMAGE_DATA_DIRECTORY GetDataDirectory(int index);
	// 区段头表
	PIMAGE_SECTION_HEADER GetSectionHeader();
	// 获取缓冲区
	PVOID GetPeBuff();

	DWORD RvaToOffset(DWORD dwRva);

	// 时间转换
	tm PeFileTimeToLocalTime(DWORD dwtime);

	// 获取重定位表
	PIMAGE_BASE_RELOCATION GetRelocation();
	// 修复重定位表
	bool FixRelocation(DWORD NewBase);









private:
	char* m_pbuf;
	int   m_size;

};

