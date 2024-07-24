#include "auther.hpp"

#include <chrono>

#include "auth.h"
#include "crypt.hpp"

Auther::Auther(const HWND& hWnd, std::string& authkey, int buttonId, int copyButtonID, int deleteButtonID, vec2<int> position,
               double time)
{
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
    this->authkey = authkey;
    this->buttonID = buttonId;
    this->copybuttonID = copyButtonID;
    this->deleteButtonID = deleteButtonID;

    const std::wstring temp = std::wstring(authkey.begin(), authkey.end());
    authkeyWindow = CreateWindow(TEXT("Edit"), temp.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER, position.x, position.y, 310, 20,
                                 hWnd, NULL, NULL, NULL);
    saveWindow = CreateWindow(TEXT("Button"), TEXT("Apply"), WS_CHILD | WS_VISIBLE | WS_BORDER, position.x + 320, position.y,
                              50, 20, hWnd, (HMENU)buttonId, NULL, NULL);

    otp = auth::generateToken(authkey, time);

    OTPWindow = CreateWindow(TEXT("Edit"), std::to_wstring(otp).c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
                             position.x, position.y + 30, 60, 20, hWnd, NULL, NULL, NULL);

    copyWindow = CreateWindow(TEXT("Button"), TEXT("Copy"), WS_CHILD | WS_VISIBLE | WS_BORDER, position.x + 70, position.y + 30,
                              40, 20, hWnd, (HMENU)copyButtonID, NULL, NULL);
    timerWindow = CreateWindow(TEXT("Edit"), TEXT("timer"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, position.x + 120,
                               position.y + 30, 20, 20, hWnd, NULL, NULL, NULL);
#ifdef SINGLE
    const std::string str = std::to_string(otp);
    toClipboard(str);
#elif MULTI
    deleteWindow = CreateWindow(TEXT("Button"), TEXT("Delete"), WS_CHILD | WS_VISIBLE | WS_BORDER, position.x + 380, position.y,
                                60, 20, hWnd, (HMENU)deleteButtonID, NULL, NULL);
#endif
}

Auther::~Auther() {}

void Auther::Update(const double timeSinceEpoch, const int sec)
{
    SetWindowText(timerWindow, std::to_wstring(sec).c_str());
    InvalidateRect(timerWindow, NULL, FALSE);
    otp = auth::generateToken(authkey, timeSinceEpoch);
}

void Auther::UpdateOTP()
{
    const std::wstring wstr = std::to_wstring(otp);
    SetWindowText(OTPWindow, wstr.c_str());
    InvalidateRect(OTPWindow, NULL, FALSE);
#ifdef SINGLE
    const std::string str = std::to_string(otp);
    toClipboard(str);
#endif
}

bool Auther::ProcessButton(const WPARAM param)
{
    if (param == buttonID)
    {
        std::wstring wString;
        wString.resize(41);
        GetWindowText(authkeyWindow, (LPWSTR)wString.data(), 41);
        std::string str(wString.begin(), wString.end());

        authkey = str;
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
        const auto dur = std::chrono::system_clock::now().time_since_epoch();
        const auto ttime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(dur).count());

        otp = auth::generateToken(authkey, ttime);
        const std::wstring wstr1 = std::to_wstring(otp);
        const std::string str1 = std::to_string(otp);
        SetWindowText(OTPWindow, wstr1.c_str());
        InvalidateRect(OTPWindow, NULL, FALSE);
    }
    else if (param == copybuttonID)
    {
        const std::string str1 = std::to_string(otp);
        toClipboard(str1);
    }
    else if (param == deleteButtonID)
    {
        auto baller = MessageBox(NULL, L"You are about to delete a authentication key that is in no way recoverable!",
                                 L"Are you sure?", MB_YESNO | MB_ICONWARNING);
        switch (baller)
        {
            case IDYES:
                return true;
                break;
            case IDNO:
                break;
        }
    }
    return false;
}

void Auther::Delete()
{
    DestroyWindow(authkeyWindow);
    DestroyWindow(saveWindow);
    DestroyWindow(OTPWindow);
    DestroyWindow(copyWindow);
    DestroyWindow(timerWindow);
#ifdef MULTI
    DestroyWindow(deleteWindow);

#endif
}
