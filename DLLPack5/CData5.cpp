#include "pch.h"
#include "CData5.h"
#include <iostream>


#pragma region ���⺯��
BOOL PrintStr(LPCSTR Str, HANDLE FileHandle)
{
	puts(Str);
	if (FileHandle)
		CloseHandle(FileHandle);
	return 0;
}

BOOL ReadFile(LPCSTR Path, DWORD* pFileSize)
{
	// ��ֻ���ķ�ʽ��ָ�����ļ���Ҫ���ļ�����
	HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("�ļ��򿪴���");


	// ��ȡ�ļ��Ĵ�С
	DWORD dwRead, FileSize = GetFileSize(FileHandle, &dwRead);
	if (dwRead || FileSize < 0x40)	//DOSͷ
		return PrintStr("�ļ���С����", FileHandle);

	// ��ȡ�ļ�
	BYTE buff[0x40];
	if (::ReadFile(FileHandle, (LPVOID)buff, 0x40, &dwRead, NULL)
		== FALSE || dwRead != 0x40 ||
		buff[0] != 0x4D ||
		buff[1] != 0x5A) {
		return PrintStr("DOSͷ����", FileHandle);
	}

	// ����ѿռ�
	auto mpFile = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize * sizeof(BYTE));
	if (!mpFile)
		return PrintStr("�����ڴ����", FileHandle);

	// ��ȡ�ļ�
	memcpy(mpFile, buff, 0x40);
	if (::ReadFile(FileHandle, (LPVOID)(mpFile + 0x40), FileSize, &dwRead, NULL)
		== FALSE || dwRead != FileSize - 0x40)
		return PrintStr("�ļ���ȡ����", FileHandle);

	// �رվ������ֹй¶
	CloseHandle(FileHandle);
	printf("���ļ��ɹ����ļ���С��%lu��0x%p\n", FileSize == dwRead, mpFile);
	*pFileSize = FileSize;
	return (int)mpFile;
}

BOOL SaveFile(LPCSTR Path, LPCCH pMem, DWORD pFileSize)
{
	// ��ֻ���ķ�ʽ��ָ�����ļ���Ҫ���ļ�����
	HANDLE FileHandle = CreateFileA(Path, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return PrintStr("���ļ��򿪴���");
	DWORD dwLen = 0;
	WriteFile(FileHandle, pMem, pFileSize, &dwLen, 0);
	// �رվ������ֹй¶
	CloseHandle(FileHandle);
	return pFileSize == dwLen;
}
#pragma endregion


CData5::CData5()
{
}
Wind::Wind()
{
}
CMyFile::CMyFile()
{
}

DWORD CMyFile::ReadOldEXE()
{
	return 0;
}
