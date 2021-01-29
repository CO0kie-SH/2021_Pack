#include "pch.h"
#include "CWind6.h"

LPCTSTR lpszAppName = L"请输入";//窗口名称
HINSTANCE g_hInstance;	//密码窗口实例句柄
HWND hEdit;				//输入密码窗口
BOOL bSuccess;			//密码验证
HWND gHWND;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char GetKey[5];
	auto hInstance = g_hInstance;
	switch (uMsg)    //消息选择
	{
	case WM_CREATE:
	{
		CreateWindowExW(0, L"button", L"确定", WS_CHILD | WS_VISIBLE,
			168, 0, 40, 40, hWnd, (HMENU)10001, hInstance, NULL);
		//CreateWindowEx(0, L"button", L"取消", WS_CHILD | WS_VISIBLE,
		//	270, 115, 60, 30, hWnd, (HMENU)10002, g_hInstance, NULL);
		hEdit = CreateWindowExW(0, L"edit", L"4位数", WS_CHILD | WS_VISIBLE | WS_BORDER,
			8, 0, 160, 40, hWnd, (HMENU)10003, hInstance, NULL);
		//HWND hBit = CreateWindowExW(0, L"static", L"密码", WS_CHILD | WS_VISIBLE,
		//	0, 0, 20, 36, hWnd, (HMENU)10004, hInstance, NULL);
		//bSuccess = FALSE;
		break;
	}
	case  WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case 10001:
			GetWindowTextA(hEdit, GetKey, 5);
			//如果密码等于123
			//if (strcmp(GetKey, "1234") == 0)
			if (strlen(GetKey) == 4)
			{
				ShowWindow(gHWND, SW_HIDE);
				PostQuitMessage(*((LPDWORD)GetKey));
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
		//MessageBoxA(hWnd, "左键按下", "提示", MB_OK);
		break;
	case WM_DESTROY:    //当前窗口被销毁时
		PostQuitMessage(0);    //发出退出程序的消息
		return 0;    //如果处理了该消息, 就返回 0 来告知 Windows
	case WM_PAINT:    //当窗口需要重绘时
	{
		PAINTSTRUCT ps = { 0 };
		BeginPaint(hWnd, &ps);    //开始重绘
		EndPaint(hWnd, &ps);    //结束重绘
		break;
	}
	case WM_CLOSE:    //当用户点击窗口右上角的关闭按钮时
		DestroyWindow(hWnd);    //销毁窗口
		break;
	}
	return(DefWindowProcW(hWnd, uMsg, wParam, lParam));
}


CWind6::CWind6()
{
}

CWind6::~CWind6()
{
}

DWORD CWind6::Create()
{
	CDbg6 dbg6;
	MSG            msg;//消息结构
	WNDCLASS       wc;//窗口类型结构
	HWND           hWnd;//窗口句柄
	auto hInstance = g_hInstance;

	//定义窗口类型
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursorW(NULL, IDC_HAND);

	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);            //窗口背景, 这里使用灰色背景
	wc.lpszMenuName = NULL;
	wc.lpszClassName = lpszAppName;

	//注册该窗口类型
	if (RegisterClassW(&wc) == 0)
		return 0;

	//创建应用程序主窗口
	hWnd = CreateWindowExW(
		0,
		lpszAppName,
		lpszAppName,
		WS_OVERLAPPEDWINDOW,
		360, 360,
		260, 80,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	//如果窗口创建失败，退出程序
	if (hWnd == NULL)
		return false;
	gHWND = hWnd;
	//显示窗口
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	//启动消息循环，直到窗口被关闭
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		//分发消息至相应的窗口的消息处理回调函数
		DispatchMessageW(&msg);
	}
	return (DWORD)msg.wParam;
}
