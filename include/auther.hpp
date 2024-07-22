#pragma once
#include <windows.h>

#include <string>

#include "tools.hpp"
class Auther
{
public:
    Auther(const HWND& hWnd, std::string& authkey, int buttonId, int copyButtonID, int deleteButtonID, vec2<int> position,
           double time);
    ~Auther();
    void Update(double timeSinceEpoch, const int sec);
    void UpdateOTP();
    bool ProcessButton(WPARAM param);
    std::string GetKey() { return authkey; };
    void Delete();

private:
    uint32_t otp;

    HWND authkeyWindow = nullptr;
    HWND OTPWindow = nullptr;
    HWND timerWindow = nullptr;
    HWND saveWindow = nullptr;
    HWND deleteWindow = nullptr;
    HWND copyWindow = nullptr;
    int buttonID = -1;
    int copybuttonID = -1;
    int deleteButtonID = -1;
    std::string authkey;
};
