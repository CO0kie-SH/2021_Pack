// TestTLS.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

#pragma comment(linker, "/merge:.data=.text") 
#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/merge:.idata=.text")
//#pragma comment(linker, "/merge:.reloc=.text")
#pragma comment(linker, "/section:.text,RWE")

void print_consoleA(const char* szMsg)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(hStdout, szMsg, strlen(szMsg), NULL, NULL);
}


void NTAPI TLS_CALLBACK1(PVOID Dllhandle, DWORD Reason, PVOID Reserved)
{
	char szMsg[80] = { 0, };
	wsprintfA(szMsg, "TLS_CALLBACK1():\tDllHandle =%p,Reason=%d\n", Dllhandle, Reason);
	print_consoleA(szMsg);
	if (Reason == 0)
	{
		system("pause");
	}
}

#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_p_thread_callback_base")


extern "C" {
#pragma data_seg(".CRT$XLB")
	PIMAGE_TLS_CALLBACK p_thread_callback_base = TLS_CALLBACK1;
	// Reset the default section.
#pragma data_seg()
}

//新建线程
DWORD WINAPI ThreadProc(LPVOID lParam)
{
	std::cout << "ThreadProc() start\n";
	std::cout << "ThreadProc() end\n";
	return 0;
}

int main()
{
    std::cout << "Hello World!\n";
	auto hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	if (hThread)
	{
		WaitForSingleObject(hThread, 60 * 1000);
		CloseHandle(hThread);
	}

	std::cout << "Hello end!\n";
	system("pause");
}
