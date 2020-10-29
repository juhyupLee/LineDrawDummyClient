#pragma once
#define ERROR_LOG(string)			PrintErrorLog(string,__LINE__)
#define ERROR_LOG(string,handle)	 PrintErrorLog(string,handle,__LINE__)
#define ERROR_LOG_SELECT(string,handle,value)	 PrintErrorLog(string,handle,value,__LINE__)
void PrintErrorLog(const wchar_t* str, int line);
void PrintErrorLog(const wchar_t* str, HWND hWnd, int line);
void PrintErrorLog(const wchar_t* str, HWND hWnd, int value, int line);

void WindowDebug1(const WCHAR* str, int value);
void WindowDebug2(const WCHAR* str, int value);

extern WCHAR g_TextoutBuffer1[256];
extern WCHAR g_TextoutBuffer2[256];
extern WCHAR g_TextoutBuffer3[256];