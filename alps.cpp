// alps.cpp : Defines the entry point for the application.
//

#include <ctime>
#include <shlobj.h>

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
HWND startEncoding;                             // start button
HWND endEncoding;                               // end button
Encoding::cuda cuda_maker;
Monitor::monitorInfo::monitor mInfo;
Encoding::encoder encode;



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                UpdateCursorPosition(HWND hWnd, POINT &pt);
void                ToggleMenuItem(HMENU hMenu, UINT itemID);
bool                CheckToggle(HMENU hMenu, UINT itemID);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    Logger::systemLogger.addLog(Logger::info, "Entering WinMain");
    Monitor::monitorInfo::monitorInfo(&mInfo);

    for (unsigned short int i = 0; i < mInfo.friendlyName.size(); i++) {
        Logger::systemLogger.addLog(Logger::info, Conversions::wvector_to_string(&mInfo.friendlyName, i));
        Logger::systemLogger.addLog(Logger::info, Conversions::rectVector_to_string(&mInfo.displayArea, i, DISPLAY));
        Logger::systemLogger.addLog(Logger::info, Conversions::rectVector_to_string(&mInfo.workArea, i, DISPLAY));
    }
    
    encode = Encoding::encoder::encoder(cuda_maker.device, cuda_maker.context, Conversions::rectVector_to_int(&mInfo.displayArea, 0));
    encode.InitializeNVEncoder(cuda_maker.context, Conversions::rectVector_to_int(&mInfo.displayArea, 0));
    encode.AllocateBuffers(Conversions::rectVector_to_int(&mInfo.displayArea, 0), 0);

    // This struct should be sent to the encoder with every frame. How should this be implemented??
    /*NV_ENC_PIC_PARAMS frameParams = {};
    frameParams.version = NV_ENC_PIC_PARAMS_VER;
    frameParams.inputWidth = (uint32_t)Conversions::rectVector_to_int(&mInfo.displayArea, 0).right;
    frameParams.inputHeight = (uint32_t)Conversions::rectVector_to_int(&mInfo.displayArea, 0).bottom;
    frameParams.inputPitch = (uint32_t)Conversions::rectVector_to_int(&mInfo.displayArea, 0).right;*/

    
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

   startEncoding = CreateWindowW(L"BUTTON", // predefined class
       L"Start Encoding", // Button text
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // styles 
       10, // x pos
       10, // y pos
       100, // width
       30, // height
       hWnd, // parent window
       (HMENU)1001, // Button ID
       hInstance, // instance handle
       nullptr); // pointer not needed

   endEncoding = CreateWindowW(L"BUTTON",
       L"End Encoding",
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
       30,
       30,
       100,
       30,
       hWnd,
       (HMENU)1002,
       hInstance,
       nullptr);

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
    static RECT rect;
    static POINT pt;
    static PWSTR roaming = NULL;
    static HRESULT hr;

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
            case 1001:
                if (!encode.GetStatus()) {
                    MessageBox(hWnd, L"Starting encoding", L"Information", MB_OK);
                    encode.StartEncoding();
                }
                break;
            case 1002:
                if (encode.GetStatus()) {
                    MessageBox(hWnd, L"Ending encoding", L"Information", MB_OK);
                    encode.EndEncoding();
                }
                break;
            case ID_TOOLS_CURSORPOSITION:
                ToggleMenuItem(GetMenu(hWnd), LOWORD(wParam));
                break;

            case ID_TOOLS_LOGS:
                hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &roaming);
                if (SUCCEEDED(hr)) {
                    std::wstring subfolder = std::wstring(roaming) + L"\\ALPS\\";
                    ShellExecute(nullptr, L"open", subfolder.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                    CoTaskMemFree(roaming);
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_CREATE: 
        SetTimer(hWnd, 1, 100, NULL);
        break;

    case WM_TIMER:
        UpdateCursorPosition(hWnd, pt);
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...

            // Checking if the right portion of the menu has been checked
            if (CheckToggle(GetMenu(hWnd), ID_TOOLS_CURSORPOSITION)) {
                SetBkMode(hdc, OPAQUE);
                SetBkColor(hdc, RGB(220, 220, 220));
                SetTextColor(hdc, RGB(0, 0, 0));

                std::wstring posText = L"X: " + std::to_wstring(pt.x) + L" Y: " + std::to_wstring(pt.y);
                RECT textRect = { 10, rect.bottom - 30, 200, rect.bottom };
                DrawText(hdc, posText.c_str(), -1, &textRect, DT_LEFT);
            }
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &rect);
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

void UpdateCursorPosition(HWND hWnd, POINT &pt) {
    GetCursorPos(&pt);
    ScreenToClient(hWnd, &pt);

    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);
}

void ToggleMenuItem(HMENU hMenu, UINT itemID) {
    MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
    mii.fMask = MIIM_STATE;
    GetMenuItemInfo(hMenu, itemID, FALSE, &mii);
    
    if (mii.fState == MFS_CHECKED) {
        mii.fState = MFS_UNCHECKED;
        SetMenuItemInfo(hMenu, itemID, FALSE, &mii);
    }
    else if (mii.fState == MFS_UNCHECKED) {
        mii.fState = MFS_CHECKED;
        SetMenuItemInfo(hMenu, itemID, FALSE, &mii);
    }
}

bool CheckToggle(HMENU hMenu, UINT itemID) {
    MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
    mii.fMask = MIIM_STATE;
    GetMenuItemInfo(hMenu, itemID, FALSE, &mii);
    if (mii.fState == MFS_CHECKED) {
        return true;
    }
    return false;
}