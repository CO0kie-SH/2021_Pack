//#include "CPE5.h"
//
//
//CPE5::CPE5()
//{
//	// ��ȡ�ļ�
//	DWORD FileSize = 0;
//	auto pFile = (PBYTE)ReadFile5("..\\Debug\\demo.exe", &FileSize);
//	
//	// �����ļ�
//	auto pXorMem = (PBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize);
//	if (pXorMem == 0)	return;
//	XorMem5(pFile, pXorMem, FileSize, 0x34333231);
//
//	// ѹ���ļ�
//	MyLz4 lz4 = {
//		TRUE,(LPCH)pXorMem,0,FileSize
//	};
//	BOOL bCom = Lz4Compress(&lz4);
//	if (!bCom)	return;
//
//	// ��������ļ�����
//	SaveFile5("..\\Debug\\demo.xor.lz4",
//		lz4.newAddr, lz4.newSize + 8);
//
//	SaveEXE(pFile, &lz4);
//}
//
//CPE5::~CPE5()
//{
//}
//
//BOOL CPE5::SaveEXE(LPCBYTE oldFile, LPMyLz4 lz4)
//{
//	auto fNT = NtHeader((PIMAGE_DOS_HEADER)oldFile);
//	auto fOH = &fNT->OptionalHeader;
//
//	auto dNT = NtHeader((PIMAGE_DOS_HEADER)GetBase());
//	auto dOH = &dNT->OptionalHeader;
//
//	DWORD fileCode = fOH->SizeOfHeaders;	//��ͷ�ֽ���
//	DWORD nowSize = lz4->oldSize;			//�ڴ����ֵ
//	DWORD dllSize = dOH->SizeOfImage - dOH->BaseOfCode;	//DLL�ռ��С
//	// �ܴ�С = ԭͷ�ļ� + DLL��С + 8�� + ѹ���� 
//	DWORD newSize = fileCode + dllSize + 8 + lz4->newSize;
//
//	// �ж϶����
//	if (newSize > nowSize)
//	{
//		MessageBoxA(0, "�ڴ����С���޷�����", 0, 0);
//		return 0;
//	}
//	LPBYTE newMem = (LPBYTE)lz4->oldAddr;
//	LPBYTE dllCode = (LPBYTE)GetBase()+ dOH->BaseOfCode;
//
//	// ����ڴ�
//	ZeroMemory(newMem, newSize);
//
//	// �����ļ�ͷ
//	memcpy(newMem, oldFile, fileCode);
//	newMem += fileCode;
//
//	// ����DLL��
//	memcpy(newMem, dllCode, dllSize);
//	newMem += dllSize;
//
//	// ����DLL��
//	memcpy(newMem, lz4->newAddr, lz4->newSize + 8);
//	if (RePE(lz4->oldAddr, newSize) == 0)	return 0;
//
//	BOOL bSave = SaveFile5("..\\Debug\\demo.new.exe",
//		lz4->oldAddr, newSize);
//	return bSave;
//}
//
//BOOL CPE5::RePE(LPCH newMem, DWORD newSize)
//{
//	auto dNT = NtHeader((PIMAGE_DOS_HEADER)GetBase());
//	auto dOH = &dNT->OptionalHeader;
//	auto& dDir = dOH->DataDirectory;
//
//	auto nNT = NtHeader((PIMAGE_DOS_HEADER)newMem);
//	auto nOH = &nNT->OptionalHeader;
//	auto& nDir = nOH->DataDirectory;
//	auto nSEC = IMAGE_FIRST_SECTION(nNT);
//
//	DWORD tmp = nNT->FileHeader.NumberOfSections,
//		IATVA = dDir[1].VirtualAddress,
//		IATSize = dDir[1].Size;
//	ZeroMemory(nSEC + 1, sizeof(IMAGE_SECTION_HEADER) * tmp);
//	nNT->FileHeader.NumberOfSections = 1;
//	ZeroMemory(nDir, sizeof(IMAGE_DATA_DIRECTORY) * 16);
//
//	//DLL�ǵĵ����
//	nDir[1].VirtualAddress = IATVA;
//	nDir[1].Size = IATSize;
//
//	// ��չͷ�޸�
//	tmp = (DWORD)GetProcAddress((HMODULE)GetBase(), "start");
//	nOH->BaseOfData = 0;
//	nOH->SizeOfCode = 0;
//	nOH->AddressOfEntryPoint = tmp - dOH->ImageBase;
//	tmp = newSize - nOH->SizeOfHeaders;		//��ȥ���׵Ĵ�С
//	nOH->SizeOfImage = tmp + nOH->BaseOfCode;
//
//	// ���������޸�
//	memcpy(nSEC->Name, ".CO0kie", 8);
//	nSEC->SizeOfRawData = tmp;
//	nSEC->Misc.VirtualSize = tmp;
//	return TRUE;
//}
