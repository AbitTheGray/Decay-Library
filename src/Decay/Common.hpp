#pragma once

namespace Decay
{
    struct MemoryBuffer : std::streambuf
    {
        MemoryBuffer(char* begin, char* end)
        {
            this->setg(begin, begin, end);
        }
    };

    template<typename Tlen>
    inline std::string Cstr2Str(const char* cstr, Tlen maxLength)
    {
        static_assert(std::is_arithmetic<Tlen>::value, "maxLength must be numeric");

        for(std::size_t i = 0; i < maxLength; i++)
            if(cstr[i] == '\0')
                return std::string(cstr, i);
        return std::string();
    }

    template<typename T>
    inline bool IsMultipleOf2(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "value must be numeric");

        if(value == 0)
            return true;
        if((value & (value - 1)) == 0)
            return true;
        return false;
    }
}
