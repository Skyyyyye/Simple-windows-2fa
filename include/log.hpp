#pragma once
#include <windows.h>

#include <string>

class Log
{
public:
    static int Error(const std::wstring& Title, const std::wstring& message, UINT type = MB_OK | MB_ICONERROR);

private:
};