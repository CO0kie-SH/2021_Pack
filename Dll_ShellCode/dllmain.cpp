// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <windows.h>

// 将 .text .rdata 和 .data 合并到 .text 区段
//#pragma comment(linker, "/merge:.data=.text") 
//#pragma comment(linker, "/merge:.rdata=.text")
#pragma comment(linker, "/section:.text,RWE")

// 去除括号内所有函数的名称粉碎机制，方便后续的调用和修改
extern "C"
{
	__declspec(dllexport) void start()
	{
		__asm
		{
			mov eax, [esp];
			mov eax, [esp];
			mov eax, [esp];
		}
	}
}

