// alps.cpp : Defines the entry point for the application.
//

#include <ctime>

#include "alps.h"
#include "conversions.h"
#include "encoder.h"
#include "framework.h"
#include "networking.h"
#include "logger.h"
#include "monitorInfo.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name 

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    Logger::systemLogger.addLog(Logger::info, "Entering WinMain");
    Monitor::monitorInfo::monitor mInfo;
    Monitor::monitorInfo::monitorInfo(&mInfo);

    for (unsigned short int i = 0; i < mInfo.friendlyName.size(); i++) {
        Logger::systemLogger.addLog(Logger::info, Conversions::wvector_to_string(&mInfo.friendlyName, i));
        Logger::systemLogger.addLog(Logger::info, Conversions::rectVector_to_string(&mInfo.displayArea, i, DISPLAY));
        Logger::systemLogger.addLog(Logger::info, Conversions::rectVector_to_string(&mInfo.workArea, i, DISPLAY));
    }
 
    Encoding::cuda cuda_maker;

    Encoding::encoder encoder_maker(cuda_maker.device, cuda_maker.context);



   /* Networking::connection networking(7123);
    if (networking.sendMessage("Hello from host!")) {
        std::string message;
        if (networking.recieveMessage(message)) {
            Logger::systemLogger.addLog(Logger::info, "From main thread: " + message);
            Logger::systemLogger.addLog(Logger::info, message);
            char computerName[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
            GetComputerNameA(computerName, &size);
            std::string message = "Client Computer Name: ";
            message += computerName;
            if (networking.sendMessage(message.c_str())) {
                Logger::systemLogger.addLog(Logger::info, "Successfully sent client name");
            }
            else {
                Logger::systemLogger.addLog(Logger::error, "Failed to send client name");
                networking.~connection();
            }
        }
        
    }
    else {
        networking.~connection();
        Logger::systemLogger.addLog(Logger::error, "Closed networking as it did not work properly.");
    }*/

    /*Networking::encryption twofish;

    Twofish_Byte input_string[] = "Hello, Twofish!";
    Twofish_Byte encrypted_string[16];
    Twofish_Byte decrypted_string[16];

    twofish.encrypt(input_string, encrypted_string);

    Logger::systemLogger.addLog(Logger::info, encrypted_string);

    twofish.decrypt(encrypted_string, decrypted_string);

    Logger::systemLogger.addLog(Logger::info, decrypted_string);*/

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ALPS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ALPS));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ALPS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ALPS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
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
            // TODO: Add any drawing code that uses hdc here...

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
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
