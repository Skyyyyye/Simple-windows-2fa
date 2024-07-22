#pragma once
#include <vector>

template <typename T>
struct vec2
{
    T x = NULL;
    T y = NULL;
};

inline void toClipboard(const std::string& s)
{
    const HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
    memcpy(GlobalLock(hMem), s.data(), s.size() + 1);
    GlobalUnlock(hMem);
    OpenClipboard(nullptr);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

inline std::vector<std::string> SplitString(const std::string& input, const std::string& delim)
{
    std::vector<std::string> result;
    size_t pos = 0, pos2 = 0;
    while ((pos2 = input.find(delim, pos)) != std::string::npos)
    {
        result.push_back(input.substr(pos, pos2 - pos));
        pos = pos2 + 1;
    }

    result.push_back(input.substr(pos));

    return result;
}