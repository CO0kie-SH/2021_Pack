// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "CPack4.h"

#pragma comment(linker, "/merge:.data=.text") 
#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/merge:.reloc=.text")
#pragma comment(linker, "/section:.text,RWE")


//HMODULE ghModule = 0;

// 去除括号内所有函数的名称粉碎机制，方便后续的调用和修改
EXTERN_C
{
	// 创建一个裸函数作为新的 OEP，不会生成任何的其他代码
	//__declspec(dllexport) void* GetBase()
	//{
	//	return ghModule;
	//}
	// 创建一个裸函数作为新的 OEP，不会生成任何的其他代码
	__declspec(dllexport) void start()
	{
		//if (Base != 0x400000)
			//return;
		CPack4 pack4;
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//ghModule = hModule;
		start();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

