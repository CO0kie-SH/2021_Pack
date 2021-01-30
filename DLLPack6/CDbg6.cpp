#include "pch.h"
#include "CDbg6.h"
#include <winternl.h>
#pragma comment(lib,"ntdll.lib")

CDbg6::CDbg6()
{
	CheckVMfile();
	bool bRet = TRUE;

	// 查询PEB
	_asm
	{
		mov eax, fs: [0x30] ;//找到PEB
		mov al, byte ptr ds : [eax + 0x2] ; // 取出一个字节
		mov bRet, al;
	}
	if (bRet) {
		MessageBoxA(0, "PEB被调试。", 0, 0);
		ExitProcess(1);
		ExitThread(2);
	}

	// 查询标志位
	UINT NtGlobalFlag = 0;
	__asm
	{
		mov eax, dword ptr fs : [0x30] ;		// 获取PEB地址
		mov eax, dword ptr ds : [eax + 0x68] ;	// 获取PEB.NtGloba...
		mov NtGlobalFlag, eax;
	}
	if( NtGlobalFlag == 0x70)
	{
		MessageBoxA(0, "NtGloba被调试。", 0, 0);
		ExitProcess(3);
		ExitThread(4);
	}

	//从WindowsXP开始，将为调试的进程创建一个“调试对象”。
	//以下就是检查当前进程调试对象的例子：
	HANDLE hProcessDebugObject = 0;
	NTSTATUS status = NtQueryInformationProcess(
		GetCurrentProcess(), 			// 目标进程句柄
		(PROCESSINFOCLASS)0x1E, 		// 查询信息类型  ProcessDebugObjectHandle=0x1E
		&hProcessDebugObject, 			// 输出查询信息
		sizeof(hProcessDebugObject),	// 查询类型大小
		NULL); 							// 实际返回大小
	if (status && hProcessDebugObject)
	{
		MessageBoxA(0, "NtQueryInformationProcess被调试0x1E。", 0, 0);
		ExitProcess(5);
		ExitThread(6);
	}
	BOOL bProcessDebugFlag = 0;
	status = NtQueryInformationProcess(
		GetCurrentProcess(), 			// 目标进程句柄
		(PROCESSINFOCLASS)0x1F, 		// 查询信息类型  ProcessDebugFlags=0x1F
		&bProcessDebugFlag, 			// 输出查询信息
		sizeof(bProcessDebugFlag),		// 查询类型大小
		NULL); 							// 实际返回大小
	if(status && NULL != bProcessDebugFlag)
	{
		MessageBoxA(0, "NtQueryInformationProcess被调试0x1F。", 0, 0);
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
		if (IDOK == MessageBoxA(0, "存在虚拟机文件。", 0, MB_OKCANCEL))
		{
			ExitProcess(9);
			ExitThread(10);
		}
	}
	return bRet;
}
