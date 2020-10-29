#include <stdio.h>
#include <WinSock2.h>
#include "LogManager.h"
#include "SocketLog.h"


WCHAR g_TextoutBuffer1[256];
WCHAR g_TextoutBuffer2[256];
WCHAR g_TextoutBuffer3[256];

void PrintErrorLog(const wchar_t* str, int line)
{
	//wprintf(L"%s  [ErrorCode:%d] [Line:%d]\n",str, WSAGetLastError(),line);
	wsprintf(g_TextoutBuffer3,L"%s  [ErrorCode:%d] [Line:%d]\n", str, WSAGetLastError(), line);
}
void PrintErrorLog(const wchar_t* str, HWND hWnd, int line)
{
	wsprintf(g_TextoutBuffer3, L"%s  [ErrorCode:%d] [Line:%d]\n", str, WSAGetLastError(), line);
	CLogManager::GetInstance()->PrintLog(g_TextoutBuffer3);
	//InvalidateRect(hWnd, NULL, TRUE);
}
//-----------------------------------------------------------
// AsyncSelect는 WSAGetLastError가 안먹히므로, 밖에서 lParam으로 Value를 넣어줘야함.
//-----------------------------------------------------------
void PrintErrorLog(const wchar_t* str, HWND hWnd,int value, int line)
{
	wsprintf(g_TextoutBuffer3, L"%s  [ErrorCode:%d] [Line:%d]\n", str, value, line);
	CLogManager::GetInstance()->PrintLog(g_TextoutBuffer3);
	//InvalidateRect(hWnd, NULL, TRUE);
}
void WindowDebug1(const WCHAR* str, int value)
{
	wsprintf(g_TextoutBuffer1, L"%s : %d", str, value);
}
void WindowDebug2(const WCHAR* str, int value)
{
	wsprintf(g_TextoutBuffer2, L"%s : %d", str, value);
}
