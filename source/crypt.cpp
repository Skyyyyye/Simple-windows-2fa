#include "crypt.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
void encdec::encrypt(const std::string& file, const std::string& content, int key)
{
    std::ofstream out;
    std::string encryptedString = content;
    for (char& t : encryptedString)
    {
        if (t != '\n')
        {
            t += key;
        }
    }
    out.open(file, std::fstream::out | std::ios::binary);
    out << encryptedString;
    out.close();
}

// Definition of decryption function
std::string encdec::decrypt(const std::string& file, int key)
{
    std::ifstream in;
    in.open(file, std::fstream::in | std::ios::binary);

    std::stringstream sstr;
    sstr << in.rdbuf();
    std::string str = sstr.str();

    for (char& t : str)
    {
        if (t != '\n')
        {
            t -= key;
        }
    }
    in.close();
    return str;
}