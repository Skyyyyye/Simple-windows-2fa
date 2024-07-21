#pragma once
#include <string>

class encdec
{
public:
    static void encrypt(const std::string& file, const std::string& content, int key);
    static std::string decrypt(const std::string& file, int key);

private:
};
