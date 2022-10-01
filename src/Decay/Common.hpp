#pragma once

#include <fstream>
#include <iostream>
#include <functional>
#include <string>
#include <strings.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <optional>
#include <filesystem>
#include "glm/glm.hpp"

#include <stb_image_write.h>


#pragma region D_ASSERT / R_ASSERT

#ifdef DEBUG
/// Debug-only assert
inline constexpr void D_ASSERT(bool condition, const std::string& msg)
{
    if(!condition) [[unlikely]]
    {
        std::cerr << msg << std::endl;
        throw std::runtime_error(msg);
    }
}
/// Debug-only assert
inline constexpr void D_ASSERT(bool condition)
{
    if(!condition) [[unlikely]]
    {
        std::cerr << "Assertion failed" << std::endl;
        throw std::runtime_error("Assertion failed");
    }
}
#else
/// Debug-only assert
inline constexpr void D_ASSERT(bool condition, const std::string& msg) {}
/// Debug-only assert
inline constexpr void D_ASSERT(bool condition) {}
#endif

/// Always active assert
inline constexpr void R_ASSERT(bool condition, const std::string& msg)
{
    if(!condition) [[unlikely]]
    {
        std::cerr << msg << std::endl;
        throw std::runtime_error(msg);
    }
}
/// Always active assert
inline constexpr void R_ASSERT(bool condition)
{
    if(!condition) [[unlikely]]
    {
        std::cerr << "Assertion failed" << std::endl;
        throw std::runtime_error("Assertion failed");
    }
}

#pragma endregion

namespace Decay
{
    class MemoryBuffer : public std::streambuf
    {
    public:
        MemoryBuffer(char* begin, char* end)
         : m_Begin(begin), m_End(end), m_Size(end - begin)
        {
            this->setg(m_Begin, m_Begin, m_End);
            this->setp(m_Begin, m_End);
        }
        MemoryBuffer(char* begin, std::size_t size)
         : MemoryBuffer(begin, begin + size)
        {
        }

    private:
        char* m_Begin;
        char* m_End;

        std::size_t m_Size;

        std::size_t m_Offset_g = 0; // IN
        std::size_t m_Offset_p = 0; // OUT

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
                    if(mode & std::ios_base::in)
                        offset = m_Offset_g + relative;
                    if(mode & std::ios_base::out)
                        offset = m_Offset_p + relative;
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
            if(offset < 0 || offset > m_Size)
                throw std::runtime_error("Offset out of bounds");

            if(mode & std::ios_base::in)
            {
                m_Offset_g = offset;
                setg(m_Begin + m_Offset_g, m_Begin + m_Offset_g, m_End);

                return m_Offset_g;
            }

            if(mode & std::ios_base::out)
            {
                m_Offset_p = offset;
                setp(m_Begin + m_Offset_p, m_End);

                return m_Offset_p;
            }

            return -1;
        }
    };

#pragma region String operation/manipulation

    template<typename Tlen>
    [[nodiscard]] inline std::string Cstr2Str(const char* cstr, Tlen maxLength) noexcept
    {
        static_assert(std::is_integral<Tlen>::value, "maxLength must be numeric");

        for(std::size_t i = 0; i < maxLength; i++)
            if(cstr[i] == '\0')
                return std::string(cstr, i);
        return std::string();
    }
    template<typename Tlen>
    inline void Str2Cstr(const std::string& str, char* cstr, Tlen maxLength) noexcept
    {
        static_assert(std::is_integral<Tlen>::value, "maxLength must be numeric");

        int i = 0;
        for(; i < str.size() && i < maxLength - 1; i++)
            cstr[i] = str[i];
        cstr[i] = '\0';
        for(; i < maxLength; i++)
            cstr[i] = '\0';
    }

    /// Checks whenever provided character is UTF-8 character (works for both first and any following characters).
    [[nodiscard]] inline static bool IsUtf8Char(char c) { return (c & 0xb1000'0000) != 0; }
    /// Get total number of UTF-8 characters (for one codepoint of value) from 1st character.
    /// Returns `0` for invalid input.
    [[nodiscard]] inline static int GetUtf8Chars(char c) noexcept
    {
        if(!IsUtf8Char(c))
            return 1;
        if((c & 0xb1100'0000) == 0xb1000'0000)
            return 0;
        if((c & 0xb1110'0000) == 0xb1100'0000)
            return 2;
        if((c & 0xb1111'0000) == 0xb1110'0000)
            return 3;
        if((c & 0xb1111'1000) == 0xb1111'0000)
            return 4;
        return 0;
    }

    template<typename T>
    [[nodiscard]] inline bool IsMultipleOf2(T value)
    {
        static_assert(std::is_integral<T>::value, "value must be numeric");

        if(value == 0)
            return true;
        if((value & (value - 1)) == 0)
            return true;
        return false;
    }

    [[nodiscard]] inline bool StringCaseInsensitiveEqual(const std::string& str0, const std::string& str1)
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
    [[nodiscard]] inline bool StringCaseInsensitiveEqual(const std::string& str0, const char* str1)
    {
#ifdef _WIN32
        // May not be needed as MinGW supports `strcasecmp`
        return stricmp(str0.data(), str1) == 0;
#else
        return strcasecmp(str0.data(), str1) == 0;
#endif
    }
    [[nodiscard]] inline bool StringCaseInsensitiveEqual(const std::string_view& str0, const std::string_view& str1)
    {
        if(str0.length() != str1.length())
            return false;

        // Temporary as `stricmp` and `strcasecmp` don't support std::string_view (const char* + length)
        return StringCaseInsensitiveEqual(std::string(str0), std::string(str1));
    }
    [[nodiscard]] inline bool StringCaseInsensitiveEqual(const std::string_view& str0, const char* str1)
    {
        // Temporary as `stricmp` and `strcasecmp` don't support std::string_view (const char* + length)
        return StringCaseInsensitiveEqual(std::string(str0), str1);
    }

    [[nodiscard]] inline std::string ToLowerAscii(std::string value) noexcept
    {
        for(int i = 0; i < value.length();)
        {
            char& c = value[i];
            int utf8chars = GetUtf8Chars(c);
            if(utf8chars == 1) // ASCII
            {
                if(c >= 'A' && c <= 'Z')
                    c += 'a' - 'A';
            }
            i += utf8chars;
        }
        return value;
    }
    [[nodiscard]] inline std::string ToUpperAscii(std::string value) noexcept
    {
        for(int i = 0; i < value.length();)
        {
            char& c = value[i];
            int utf8chars = GetUtf8Chars(c);
            if(utf8chars == 1) // ASCII
            {
                if(c >= 'a' && c <= 'z')
                    c -= 'a' - 'A';
            }
            i += utf8chars;
        }
        return value;
    }

#pragma endregion

#pragma region Write functions by file extension

    /// Extension always starts with period.
    /// Supported extensions:
    /// - .png
    /// - .bmp
    /// - .tga
    /// - .jpg / .jpeg (maximum quality)
    /// - .bin (uint32_t width, uint32_t width, uint8_t components, uint32_t... rgba_data)
    [[nodiscard]] inline std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec4* rgba)> ImageWriteFunction_RGBA(const std::string& extension)
    {
        if(extension == ".png" || extension == ".PNG")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* rgba) -> void
            {
                // Write to file
                stbi_write_png(
                    path,
                    width,
                    height,
                    4,
                    rgba,
                    static_cast<int32_t>(width) * 4
                );
            };
        }
        else if(extension == ".bmp" || extension == ".BMP")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* rgba) -> void
            {
                stbi_write_bmp(
                    path,
                    width,
                    height,
                    4,
                    rgba
                );
            };
        }
        else if(extension == ".tga" || extension == ".TGA")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* rgba) -> void
            {
                stbi_write_tga(
                    path,
                    width,
                    height,
                    4,
                    rgba
                );
            };
        }
        else if(extension == ".jpg" || extension == ".JPG" || extension == ".jpeg" || extension == ".JPEG")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* rgba) -> void
            {
                stbi_write_jpg(
                    path,
                    width,
                    height,
                    4,
                    rgba,
                    100 // 0 = minimum, 100 = maximum
                );
            };
        }
        else if(extension == ".bin" || extension == ".BIN")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* rgba) -> void
            {
                std::ofstream out(path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

                out.write(reinterpret_cast<const char*>(&width), sizeof(width));
                out.write(reinterpret_cast<const char*>(&height), sizeof(height));

                static const uint8_t components = 4;
                out.write(reinterpret_cast<const char*>(&components), sizeof(components));

                out.write(reinterpret_cast<const char*>(rgba), static_cast<std::size_t>(width) * height * 4);

                out.flush();
            };
        }
        else
            throw std::runtime_error("Unsupported texture extension for export");
    }

    /// Extension always starts with period.
    /// Supported extensions:
    /// - .png
    /// - .bmp
    /// - .tga
    /// - .jpg / .jpeg (maximum quality
    /// - .bin (uint32_t width, uint32_t width, uint8_t components, uint24_t... rgb_data)
    [[nodiscard]] inline std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec3* rgb)> ImageWriteFunction_RGB(const std::string& extension)
    {
        if(extension == ".png" || extension == ".PNG")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec3* rgb) -> void
            {
                // Write to file
                stbi_write_png(
                    path,
                    width,
                    height,
                    3,
                    rgb,
                    static_cast<int32_t>(width) * 3
                );
            };
        }
        else if(extension == ".bmp" || extension == ".BMP")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec3* rgb) -> void
            {
                stbi_write_bmp(
                    path,
                    width,
                    height,
                    3,
                    rgb
                );
            };
        }
        else if(extension == ".tga" || extension == ".TGA")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec3* rgb) -> void
            {
                stbi_write_tga(
                    path,
                    width,
                    height,
                    3,
                    rgb
                );
            };
        }
        else if(extension == ".jpg" || extension == ".JPG" || extension == ".jpeg" || extension == ".JPEG")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec3* rgb) -> void
            {
                stbi_write_jpg(
                    path,
                    width,
                    height,
                    3,
                    rgb,
                    100 // 0 = minimum, 100 = maximum
                );
            };
        }
        else if(extension == ".bin" || extension == ".BIN")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec3* rgb) -> void
            {
                std::ofstream out(path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

                out.write(reinterpret_cast<const char*>(&width), sizeof(width));
                out.write(reinterpret_cast<const char*>(&height), sizeof(height));

                static const uint8_t components = 3;
                out.write(reinterpret_cast<const char*>(&components), sizeof(components));

                out.write(reinterpret_cast<const char*>(rgb), static_cast<std::size_t>(width) * height * components);

                out.flush();
            };
        }
        else
            throw std::runtime_error("Unsupported texture extension for export");
    }

#pragma endregion

#pragma region map/vector contains
#   pragma region vector+vector
    template<typename T>
    [[nodiscard]] inline bool ContainsAll(const std::vector<T>& inside, const std::vector<T>& values)
    {
        for(const auto& val : values)
        {
            bool found = false;
            for(const auto& in : inside)
            {
                if(in == val)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                return false;
        }
        return true;
    }
    template<typename T>
    [[nodiscard]] inline bool IsSame(const std::vector<T>& inside, const std::vector<T>& values)
    {
        if(inside.size() != values.size())
            return false;
        return ContainsAll(inside, values);
    }
#   pragma endregion
#   pragma region vector+set
    template<typename T>
    [[nodiscard]] inline bool ContainsAll(const std::vector<T>& inside, const std::set<T>& values)
    {
        for(const auto& val : values)
        {
            bool found = false;
            for(const auto& in : inside)
            {
                if(in == val)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                return false;
        }
        return true;
    }
    template<typename T>
    [[nodiscard]] inline bool IsSame(const std::vector<T>& inside, const std::set<T>& values)
    {
        if(inside.size() != values.size())
            return false;
        return ContainsAll(inside, values);
    }
#   pragma endregion
#   pragma region map+map
    template<typename TK, typename TV>
    [[nodiscard]] inline bool ContainsAll(const std::map<TK, TV>& inside, const std::map<TK, TV>& values)
    {
        for(const auto& valKv : values)
        {
            auto inIt = inside.find(valKv.first);
            if(inIt == inside.end())
                return false;
            if(inIt->second != valKv.second)
                return false;
        }
        return true;
    }
    template<typename TK, typename TV>
    [[nodiscard]] inline bool IsSame(const std::map<TK, TV>& inside, const std::map<TK, TV>& values)
    {
        if(inside.size() != values.size())
            return false;
        return ContainsAll(inside, values);
    }
#   pragma endregion
#   pragma region map+map
    template<typename TK, typename TV>
    [[nodiscard]] inline bool ContainsAll(const std::unordered_map<TK, TV>& inside, const std::unordered_map<TK, TV>& values)
    {
        for(const auto& valKv : values)
        {
            auto inIt = inside.find(valKv.first);
            if(inIt == inside.end())
                return false;
            if(inIt->second != valKv.second)
                return false;
        }
        return true;
    }
    template<typename TK, typename TV>
    [[nodiscard]] inline bool IsSame(const std::unordered_map<TK, TV>& inside, const std::unordered_map<TK, TV>& values)
    {
        if(inside.size() != values.size())
            return false;
        return ContainsAll(inside, values);
    }
#   pragma endregion
#pragma endregion
}
