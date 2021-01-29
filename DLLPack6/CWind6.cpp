#include "pch.h"
#include "CWind6.h"

LPCTSTR lpszAppName = L"������";//��������
HINSTANCE g_hInstance;	//���봰��ʵ�����
HWND hEdit;				//�������봰��
BOOL bSuccess;			//������֤
HWND gHWND;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char GetKey[5];
	auto hInstance = g_hInstance;
	switch (uMsg)    //��Ϣѡ��
	{
	case WM_CREATE:
	{
		CreateWindowExW(0, L"button", L"ȷ��", WS_CHILD | WS_VISIBLE,
			168, 0, 40, 40, hWnd, (HMENU)10001, hInstance, NULL);
		//CreateWindowEx(0, L"button", L"ȡ��", WS_CHILD | WS_VISIBLE,
		//	270, 115, 60, 30, hWnd, (HMENU)10002, g_hInstance, NULL);
		hEdit = CreateWindowExW(0, L"edit", L"4λ��", WS_CHILD | WS_VISIBLE | WS_BORDER,
			8, 0, 160, 40, hWnd, (HMENU)10003, hInstance, NULL);
		//HWND hBit = CreateWindowExW(0, L"static", L"����", WS_CHILD | WS_VISIBLE,
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
			//����������123
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
		//MessageBoxA(hWnd, "�������", "��ʾ", MB_OK);
		break;
	case WM_DESTROY:    //��ǰ���ڱ�����ʱ
		PostQuitMessage(0);    //�����˳��������Ϣ
		return 0;    //��������˸���Ϣ, �ͷ��� 0 ����֪ Windows
	case WM_PAINT:    //��������Ҫ�ػ�ʱ
	{
		PAINTSTRUCT ps = { 0 };
		BeginPaint(hWnd, &ps);    //��ʼ�ػ�
		EndPaint(hWnd, &ps);    //�����ػ�
		break;
	}
	case WM_CLOSE:    //���û�����������ϽǵĹرհ�ťʱ
		DestroyWindow(hWnd);    //���ٴ���
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
	MSG            msg;//��Ϣ�ṹ
	WNDCLASS       wc;//�������ͽṹ
	HWND           hWnd;//���ھ��
	auto hInstance = g_hInstance;

	//���崰������
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursorW(NULL, IDC_HAND);

	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);            //���ڱ���, ����ʹ�û�ɫ����
	wc.lpszMenuName = NULL;
	wc.lpszClassName = lpszAppName;

	//ע��ô�������
	if (RegisterClassW(&wc) == 0)
		return 0;

	//����Ӧ�ó���������
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
	//������ڴ���ʧ�ܣ��˳�����
	if (hWnd == NULL)
		return false;
	gHWND = hWnd;
	//��ʾ����
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	//������Ϣѭ����ֱ�����ڱ��ر�
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		//�ַ���Ϣ����Ӧ�Ĵ��ڵ���Ϣ����ص�����
		DispatchMessageW(&msg);
	}
	return (DWORD)msg.wParam;
}
