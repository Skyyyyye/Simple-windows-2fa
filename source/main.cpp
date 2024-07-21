
#include <cpprest/oauth1.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include <chrono>
#include <string>

#include "Log.hpp"
#include "auth.h"
#include "crypt.hpp"

typedef std::basic_string<TCHAR> tstring;

// The main window class name.
constexpr TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
constexpr TCHAR szTitle[] = _T("Authenticator");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void toClipboard(const std::string &s);

constexpr const char file[] = "enc2.txt";
constexpr int key = 1234;

constexpr int saveButtID = 10;
constexpr int copyButtonID = 11;

HWND keyWindow;
HWND OTPWindow;
HWND hWnd = nullptr;
uint32_t otp;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        Log::Error(L"Fatal startup Error", L"failed call of RegisterClassEx", MB_OK | MB_ICONERROR);
        return 1;
    }

    hInst = hInstance;

    // The parameters to CreateWindowEx explained:
    // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          500, 120, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
    {
        Log::Error(L"Fatal startup Error", L"Call to CreateWindow failed!", MB_OK | MB_ICONERROR);

        return 1;
    }
    keyWindow =
        CreateWindow(TEXT("Edit"), TEXT("key"), WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 10, 310, 20, hWnd, NULL, NULL, NULL);
    CreateWindow(TEXT("Button"), TEXT("Save"), WS_CHILD | WS_VISIBLE | WS_BORDER, 330, 10, 40, 20, hWnd, (HMENU)saveButtID,
                 NULL, NULL);

    std::string authkey = encdec::decrypt(file, key);
    for (int i = 0; i < authkey.size(); i++)
    {
        if (authkey.at(i) == ' ' || authkey.at(i) == '\0')
        {
            authkey.erase(authkey.begin() + i);
        }
    }
    while (authkey.size() > 32)
    {
        authkey.pop_back();
    }
    const std::wstring temp = std::wstring(authkey.begin(), authkey.end());

    const LPCWSTR wideString = temp.c_str();
    SetWindowText(keyWindow, wideString);

    auto dur = std::chrono::system_clock::now().time_since_epoch();
    double ttime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(dur).count());

    otp = auth::generateToken(authkey, ttime);
    const std::wstring wstri = std::to_wstring(otp);
    const std::string stri = std::to_string(otp);
    toClipboard(stri);

    OTPWindow = CreateWindow(TEXT("Edit"), wstri.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 10, 40, 60, 20, hWnd,
                             NULL, NULL, NULL);

    CreateWindow(TEXT("Button"), TEXT("Copy"), WS_CHILD | WS_VISIBLE | WS_BORDER, 80, 40, 40, 20, hWnd, (HMENU)copyButtonID,
                 NULL, NULL);
    auto timer = CreateWindow(TEXT("Edit"), TEXT("timer"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 130, 40, 20, 20,
                              hWnd, NULL, NULL, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        // main loop

        dur = std::chrono::system_clock::now().time_since_epoch();
        ttime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(dur).count());

        const double value = (ttime / 30.0f);
        double whole = 0;
        const float fractional = std::modf(value, &whole);
        otp = auth::generateToken(authkey, ttime);

        if (fractional <= 0.01)
        {
            const std::wstring wstr = std::to_wstring(otp);
            const std::string str = std::to_string(otp);
            SetWindowText(OTPWindow, wstr.c_str());
            toClipboard(str);
        }
        int sec = static_cast<int>(fractional * 30);
        SetWindowText(timer, std::to_wstring(sec).c_str());
    }

    return static_cast<int>(msg.wParam);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_COMMAND:
            switch (wParam)
            {
                case saveButtID:
                {
                    std::wstring wString;
                    wString.resize(41);
                    GetWindowText(keyWindow, (LPWSTR)wString.data(), 41);
                    std::string str(wString.begin(), wString.end());
                    encdec::encrypt(file, str, key);

                    const auto dur = std::chrono::system_clock::now().time_since_epoch();
                    const auto ttime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(dur).count());
                    while (str.size() > 32)
                    {
                        str.pop_back();
                    }
                    otp = auth::generateToken(str, ttime);
                    const std::wstring wstr1 = std::to_wstring(otp);
                    const std::string str1 = std::to_string(otp);
                    SetWindowText(OTPWindow, wstr1.c_str());
                    toClipboard(str1);
                }
                break;
                case copyButtonID:
                {
                    const std::string str1 = std::to_string(otp);
                    toClipboard(str1);
                }
                break;
                default:
                    // nothing
                    break;
            }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void toClipboard(const std::string &s)
{
    const HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
    memcpy(GlobalLock(hMem), s.data(), s.size() + 1);
    GlobalUnlock(hMem);
    OpenClipboard(nullptr);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}