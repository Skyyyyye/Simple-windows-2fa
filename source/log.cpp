#include "log.hpp"

#include <tchar.h>

int Log::Error(const std::wstring& Title, const std::wstring& message, UINT type)
{
    return MessageBox(NULL, Title.c_str(), message.c_str(), type);
}
