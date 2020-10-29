// LineDrawDummyClient.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "framework.h"
#include "LineDrawDummyClient.h"
#include <windowsx.h>
#include <iostream>
#include "RingBuffer.h"
#include "SocketLog.h"
#include <Windows.h>

#define MAX_LOADSTRING 100
#define WM_NETWORK (WM_USER+1)
#define SERVER_PORT 25000
#define SERVER_IP L"192.168.10.58"
//#define SERVER_IP L"192.168.10.35"
#define DUMMY_CNT 100
#define RAND_RANGE 1000

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//---------------------------------------------------------------------------
struct Header
{
    unsigned short len;
};
struct DrawPacket
{
    int startX;
    int startY;
    int endX;
    int endY;
};

struct Dummy
{
    SOCKET socket;
    int prevX;
    int prevY;
    bool bConnected;
    RingBuffer recvRingBuffer;
    RingBuffer sendRingBuffer;
};
int g_bDraw = false;


HWND g_hWnd;
Dummy g_Dummy[DUMMY_CNT] = { 0, };

void Network_Init(HWND hWnd);
void SelectProcess(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void SendPacket(Header* header, char* payload, int payloadSize, Dummy* dummy);
void SendEvent(SOCKET socket);
void RecvEvent(SOCKET socket);
void DrawLine(int startX, int startY, int endX, int endY);

void Disconnect(Dummy* dummy);

void DummyControl();
Header g_Header;
DrawPacket g_DrawPacket;

WCHAR g_DebugString[128];

HDC g_HDC;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LINEDRAWDUMMYCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LINEDRAWDUMMYCLIENT));

    MSG msg;

    Network_Init(g_hWnd);
    g_HDC = GetDC(g_hWnd);
    long long curTime = GetTickCount64();

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        DummyControl();

        /*if (GetTickCount64() - curTime > 500)
        {
            curTime = GetTickCount64();
            DummyControl();
        }*/
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LINEDRAWDUMMYCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LINEDRAWDUMMYCLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_NETWORK:
        SelectProcess(hWnd, message, wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void Network_Init(HWND hWnd)
{
    WSAData wsaData;
    SOCKADDR_IN serverAddr;

    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        ERROR_LOG(L"WSAStart up() error", hWnd);
    }

    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    InetPton(AF_INET, SERVER_IP, &serverAddr.sin_addr.S_un.S_addr);

    for (int i = 0; i < DUMMY_CNT; i++)
    {
        g_Dummy[i].socket = socket(AF_INET, SOCK_STREAM, 0);
        if (g_Dummy[i].socket == INVALID_SOCKET)
        {
            ERROR_LOG(L"socket() error", hWnd);
        }
        if (0 != WSAAsyncSelect(g_Dummy[i].socket, hWnd, WM_NETWORK, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE))
        {
            ERROR_LOG(L"WSAAsyncSelect() error", hWnd);
        }
        if (0 != connect(g_Dummy[i].socket, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)))
        {
            ERROR_LOG(L"connect() error", g_hWnd);
        }
    }
}

void SelectProcess(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WSAGETSELECTERROR(lParam))
    {
        ERROR_LOG_SELECT(L"SelectProcess() error", hWnd, WSAGETSELECTERROR(lParam));
        closesocket(wParam);
        return;
    }

    switch (WSAGETSELECTEVENT(lParam))
    {
    case FD_CONNECT:
        for (int i = 0; i < DUMMY_CNT; i++)
        {
            if (g_Dummy[i].socket == wParam)
            {
                g_Dummy[i].bConnected = true;
                break;
            }
        }
        break;
    case FD_READ:
        RecvEvent(wParam);
        break;
    case FD_WRITE:
        SendEvent(wParam);
        break;
    case FD_CLOSE:
        for (int i = 0; i < DUMMY_CNT; i++)
        {
            if (g_Dummy[i].socket == wParam)
            {
                closesocket(g_Dummy[i].socket);
                break;
            }
        }
        break;
    }
}

void SendPacket(Header* header, char* payload, int payloadSize, Dummy* dummy)
{

    int enQRtn = dummy->sendRingBuffer.Enqueue((char*)header, sizeof(Header));
    enQRtn = dummy->sendRingBuffer.Enqueue(payload, payloadSize);

    SendEvent(dummy->socket);
}

void SendEvent(SOCKET socket)
{
    int dummyIDX = 0;

    for (int i = 0; i < DUMMY_CNT; i++)
    {
        if (g_Dummy[i].bConnected)
        {
            if (g_Dummy[i].socket == socket)
            {
                dummyIDX = i;
            }
        }
    }

    char buffer[10000];

    while (g_Dummy[dummyIDX].sendRingBuffer.GetUsedSize() != 0)
    {
        int peekRtn = g_Dummy[dummyIDX].sendRingBuffer.Peek(buffer, g_Dummy[dummyIDX].sendRingBuffer.GetDirectDequeueSize());
        int sendRtn = send(g_Dummy[dummyIDX].socket, buffer, peekRtn, 0);
        if (sendRtn <= 0)
        {
            ERROR_LOG(L"send Error", g_hWnd);
            break;
        }
        g_Dummy[dummyIDX].sendRingBuffer.MoveFront(sendRtn);
    }
}

void RecvEvent(SOCKET socket)
{
    int searchIndex = 0;

    for (int i = 0; i < DUMMY_CNT; i++)
    {
        if (socket == g_Dummy[i].socket)
        {
            searchIndex = i;
            break;
        }
    }
    char buffer[30000];
     
    int recvRtn = recv(g_Dummy[searchIndex].socket, buffer, 30000, 0);

    if (recvRtn <= 0)
    {
        ERROR_LOG(L"Recv() Error", g_hWnd);
        Disconnect(&g_Dummy[searchIndex]);
    }

    //int searchIndex = 0;

    //for (int i = 0; i < DUMMY_CNT; i++)
    //{
    //    if (socket == g_Dummy[i].socket)
    //    {
    //        searchIndex = i;
    //        break;
    //    }
    //}

    //int enQDirectSize = g_Dummy[searchIndex].recvRingBuffer.GetDirectEnqueueSize();

    //int recvRtn = recv(g_Dummy[searchIndex].socket,g_Dummy[searchIndex].recvRingBuffer.GetRearBufferPtr(), enQDirectSize, 0);

    //if (recvRtn <= 0)
    //{
    //    ERROR_LOG(L"Recv() Error", g_hWnd);
    //    Disconnect(&g_Dummy[searchIndex]);
    //}

    ////g_Dummy[searchIndex].recvRingBuffer.MoveRear(recvRtn);

    //g_Dummy[searchIndex].recvRingBuffer.ClearBuffer();

    //while (true)
    //{
    //    Header header;
    //    DrawPacket drawPacket;

    //    if (g_Dummy[searchIndex].recvRingBuffer.GetUsedSize() < sizeof(header))
    //    {
    //        break;
    //    }
    //    int peekRtn = g_Dummy[searchIndex].recvRingBuffer.Peek((char*)&header, sizeof(header));

    //    if (g_Dummy[searchIndex].recvRingBuffer.GetUsedSize() < sizeof(header)+header.len)
    //    {
    //        break;
    //    }
    //    g_Dummy[searchIndex].recvRingBuffer.MoveFront(sizeof(header));
    //    
    //    int deqRtn = g_Dummy[searchIndex].recvRingBuffer.Dequeue((char*)&drawPacket, sizeof(DrawPacket));

    //    //DrawLine(drawPacket.startX, drawPacket.startY, drawPacket.endX, drawPacket.endY);
    //}

}

void DrawLine(int startX, int startY, int endX, int endY)
{
    //HDC hdc = GetDC(g_hWnd);
    MoveToEx(g_HDC, startX, startY, NULL);
    LineTo(g_HDC, endX, endY);
   // ReleaseDC(g_hWnd, hdc);
}

void Disconnect(Dummy* dummy)
{
    if (dummy->bConnected == false)
    {
        return;
    }

    dummy->recvRingBuffer.ClearBuffer();
    dummy->sendRingBuffer.ClearBuffer();
    closesocket(dummy->socket);
    dummy->prevX = 0;
    dummy->prevY = 0;
    dummy->bConnected = false;
}

void DummyControl()
{
    for (int i = 0; i < DUMMY_CNT; i++)
    {
        if (g_Dummy[i].bConnected)
        {
            g_Dummy[i].prevX = rand() % RAND_RANGE;
            g_Dummy[i].prevY = rand() % RAND_RANGE;

            Header header;
            header.len = sizeof(DrawPacket);

            DrawPacket drawPacket;
            drawPacket.startX = g_Dummy[i].prevX;
            drawPacket.startY = g_Dummy[i].prevY;
            drawPacket.endX = rand() % RAND_RANGE;
            drawPacket.endY = rand() % RAND_RANGE;

            SendPacket(&header, (char*)&drawPacket, sizeof(DrawPacket), &g_Dummy[i]);
          
            
        }
    }
}
