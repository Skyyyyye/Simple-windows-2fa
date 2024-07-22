
#include <cpprest/oauth1.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include <chrono>
#include <string>

#include "Log.hpp"
#include "auther.hpp"
#include "crypt.hpp"

// To-Do:
// - add multi key support
// - add icon support

typedef std::basic_string<TCHAR> tstring;

// The main window class name.
constexpr TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
constexpr TCHAR szTitle[] = _T("Authenticator");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

constexpr char file[] = "enc2.txt";
constexpr int key = 3102;

HWND hWnd = nullptr;
std::vector<Auther> auths;
constexpr int NEWBUTTONID = 1;
constexpr int SAVEBUTTONID = 2;
vec2<int> location = vec2<int>(10, 10);

UINT NextID = 3;

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

    auto dur = std::chrono::system_clock::now().time_since_epoch();
    double ttime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(dur).count());
    auto authkeys = encdec::decrypt(file, key);
    int wsize = 120;

#ifdef MULTI
    auto keys = SplitString(authkeys, "\n");
    for (auto& key : keys)
    {
        wsize += 60;
    }
#endif

    hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          500, wsize, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
    {
        Log::Error(L"Fatal startup Error", L"Call to CreateWindow failed!", MB_OK | MB_ICONERROR);

        return 1;
    }

    // no need for new or save when theres one key
#ifdef MULTI
    CreateWindow(TEXT("Button"), TEXT("New"), WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 10, 40, 20, hWnd, (HMENU)NEWBUTTONID, NULL,
                 NULL);
    CreateWindow(TEXT("Button"), TEXT("Save"), WS_CHILD | WS_VISIBLE | WS_BORDER, 60, 10, 40, 20, hWnd, (HMENU)SAVEBUTTONID,
                 NULL, NULL);
    location.y += 30;
    int i = 0;
    for (auto& akey : keys)
    {
        if (i != keys.size() - 1)
        {
            auths.push_back(Auther(hWnd, akey, NextID, NextID + 1, NextID + 2, location, ttime));
            location.y += 70;
            NextID += 3;
            i++;
        }
    }
#elif SINGLE
    auths.push_back(Auther(hWnd, authkeys, NextID, NextID + 1, NextID + 2, location, ttime));
#endif

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        UpdateWindow(hWnd);

        TranslateMessage(&msg);
        DispatchMessage(&msg);
        // main loop

        dur = std::chrono::system_clock::now().time_since_epoch();
        ttime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(dur).count());

        const double value = (ttime / 30.0f);
        double whole = 0;
        const float fractional = std::modf(value, &whole);

        if (fractional <= 0.01)
        {
            for (auto& authkey : auths)
            {
                authkey.UpdateOTP();
            }
        }
        int sec = static_cast<int>(fractional * 30);
        for (auto& authkey : auths)
        {
            authkey.Update(ttime, sec);
        }
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
        {
            std::vector<int> deletables;
            for (int i = 0; i < auths.size(); ++i)
            {
                if (auths.at(i).ProcessButton(wParam))
                {
                    deletables.push_back(i);
                }
            }
#ifdef MULTI
            for (int i = 0; i < deletables.size(); ++i)
            {
                auths.at(deletables.at(i)).Delete();
                auths.erase(auths.begin() + deletables.at(i));
                location.y -= 70;
                NextID -= 3;
            }

            if (wParam == NEWBUTTONID)
            {
                auto dur = std::chrono::system_clock::now().time_since_epoch();
                auto ttime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(dur).count());
                std::string akey = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
                auths.push_back(Auther(hWnd, akey, NextID, NextID + 1, NextID + 2, location, ttime));
                location.y += 70;
                NextID += 3;
            }
            if (wParam == SAVEBUTTONID)
            {
                std::string allKeys;
                for (auto& auther : auths)
                {
                    allKeys.append(auther.GetKey());
                    allKeys.push_back('\n');
                }
                encdec::encrypt(file, allKeys, key);
            }
#endif
        }
        break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
