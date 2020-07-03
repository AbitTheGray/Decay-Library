#pragma once

#include <string>

inline static void CopyString(const std::string& str, char* dest, int maxLength)
{
    if(str.size() > maxLength)
    {
        std::fill(dest, dest + maxLength, '\0');
        return;
    }

    // Copy `str`
    std::copy(str.c_str(), str.c_str() + str.length(), dest);

    // Fill rest by '\0'
    // This prevents memory bleeding
    if(str.length() < maxLength)
        std::fill(dest + str.length(), dest + maxLength, '\0');
}
