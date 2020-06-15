#pragma once

#include <iostream>

#include <string>
#include <strings.h>

namespace Decay
{
    class MemoryBuffer : public std::streambuf
    {
    public:
        MemoryBuffer(char* begin, char* end)
         : m_Begin(begin), m_End(end), m_Size(end - begin)
        {
            this->setg(m_Begin, m_Begin, m_End);
        }
        MemoryBuffer(char* begin, std::size_t size) : MemoryBuffer(begin, begin + size)
        {
        }

    private:
        char* m_Begin;
        char* m_End;
        std::size_t m_Size;
        std::size_t m_Offset = 0;

    protected:
        pos_type seekoff(off_type relative, std::ios_base::seekdir direction, std::ios_base::openmode mode) override
        {
            std::size_t offset;
            switch(direction)
            {
                case std::_S_beg:
                    offset = relative;
                    break;
                case std::_S_cur:
                    offset = m_Offset + relative;
                    break;
                case std::_S_end:
                    offset = m_Size - relative;
                    break;
                default:
                    throw std::runtime_error("Unexpected seekoff direction");
                    break;
            }

            return seekpos(offset, mode);
        }

        pos_type seekpos(pos_type offset, std::ios_base::openmode mode) override
        {
            if(mode & std::ios_base::in)
            {
                if(offset < 0 || offset > m_Size)
                    throw std::runtime_error("Offset out of bounds");

                m_Offset = offset;
                setg(m_Begin + m_Offset, m_Begin + m_Offset, m_End);
            }

            if(mode & std::ios_base::out)
                std::cerr << "Trying to seek in `out` direction" << std::endl;

            return m_Offset;
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

    inline bool StringCaseInsensitiveEqual(const std::string& str0, const std::string& str1)
    {
        if(str0.length() != str1.length())
            return false;

#ifdef _WIN32
        // May not be needed as MinGW supports `strcasecmp`
        return stricmp(str0.data(), str1.data()) == 0;
#else
        return strcasecmp(str0.data(), str1.data()) == 0;
#endif
    }
}
