#include "pch.h"
#include "CDbg6.h"
#include <winternl.h>
#pragma comment(lib,"ntdll.lib")

CDbg6::CDbg6()
{
	CheckVMfile();
	bool bRet = TRUE;

	// ��ѯPEB
	_asm
	{
		mov eax, fs: [0x30] ;//�ҵ�PEB
		mov al, byte ptr ds : [eax + 0x2] ; // ȡ��һ���ֽ�
		mov bRet, al;
	}
	if (bRet) {
		MessageBoxA(0, "PEB�����ԡ�", 0, 0);
		ExitProcess(1);
		ExitThread(2);
	}

	// ��ѯ��־λ
	UINT NtGlobalFlag = 0;
	__asm
	{
		mov eax, dword ptr fs : [0x30] ;		// ��ȡPEB��ַ
		mov eax, dword ptr ds : [eax + 0x68] ;	// ��ȡPEB.NtGloba...
		mov NtGlobalFlag, eax;
	}
	if( NtGlobalFlag == 0x70)
	{
		MessageBoxA(0, "NtGloba�����ԡ�", 0, 0);
		ExitProcess(3);
		ExitThread(4);
	}

	//��WindowsXP��ʼ����Ϊ���ԵĽ��̴���һ�������Զ��󡱡�
	//���¾��Ǽ�鵱ǰ���̵��Զ�������ӣ�
	HANDLE hProcessDebugObject = 0;
	NTSTATUS status = NtQueryInformationProcess(
		GetCurrentProcess(), 			// Ŀ����̾��
		(PROCESSINFOCLASS)0x1E, 		// ��ѯ��Ϣ����  ProcessDebugObjectHandle=0x1E
		&hProcessDebugObject, 			// �����ѯ��Ϣ
		sizeof(hProcessDebugObject),	// ��ѯ���ʹ�С
		NULL); 							// ʵ�ʷ��ش�С
	if (status && hProcessDebugObject)
	{
		MessageBoxA(0, "NtQueryInformationProcess������0x1E��", 0, 0);
		ExitProcess(5);
		ExitThread(6);
	}
	BOOL bProcessDebugFlag = 0;
	status = NtQueryInformationProcess(
		GetCurrentProcess(), 			// Ŀ����̾��
		(PROCESSINFOCLASS)0x1F, 		// ��ѯ��Ϣ����  ProcessDebugFlags=0x1F
		&bProcessDebugFlag, 			// �����ѯ��Ϣ
		sizeof(bProcessDebugFlag),		// ��ѯ���ʹ�С
		NULL); 							// ʵ�ʷ��ش�С
	if(status && NULL != bProcessDebugFlag)
	{
		MessageBoxA(0, "NtQueryInformationProcess������0x1F��", 0, 0);
		ExitProcess(7);
		ExitThread(8);
	}
}

CDbg6::~CDbg6()
{
}

DWORD CDbg6::CheckVMfile()
{
	DWORD bRet = GetFileAttributesA("C:\\Program Files\\VMware\\VMware Tools");
	if (bRet != INVALID_FILE_ATTRIBUTES)
	{
		if (IDOK == MessageBoxA(0, "����������ļ���", 0, MB_OKCANCEL))
		{
			ExitProcess(9);
			ExitThread(10);
		}
	}
	return bRet;
}
