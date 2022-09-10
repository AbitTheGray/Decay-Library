#pragma once

#include <fstream>
#include <iostream>
#include <functional>
#include <string>
#include <strings.h>

#include <stb_image_write.h>

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
        MemoryBuffer(void* begin, char* end)
         : MemoryBuffer(reinterpret_cast<char*>(begin), reinterpret_cast<char*>(end))
        {
        }

        MemoryBuffer(char* begin, std::size_t size)
         : MemoryBuffer(begin, begin + size)
        {
        }
        MemoryBuffer(void* begin, std::size_t size)
         : MemoryBuffer(reinterpret_cast<char*>(begin), size)
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

    template<typename Tlen>
    inline std::string Cstr2Str(const char* cstr, Tlen maxLength)
    {
        static_assert(std::is_integral<Tlen>::value, "maxLength must be numeric");

        for(std::size_t i = 0; i < maxLength; i++)
            if(cstr[i] == '\0')
                return std::string(cstr, i);
        return std::string();
    }

    template<typename T>
    inline bool IsMultipleOf2(T value)
    {
        static_assert(std::is_integral<T>::value, "value must be numeric");

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
    inline bool StringCaseInsensitiveEqual(const std::string_view& str0, const std::string_view& str1)
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

    /// Extension always starts with period.
    /// Supported extensions:
    /// - .png
    /// - .bmp
    /// - .tga
    /// - .jpg / .jpeg (maximum quality)
    /// - .raw (uint32_t width, uint32_t width, uint8_t components, uint32_t... rgba_data)
    inline std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec4* rgba)> ImageWriteFunction_RGBA(const std::string& extension)
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
        else if(extension == ".raw" || extension == ".RAW")
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
    /// - .raw (uint32_t width, uint32_t width, uint8_t components, uint24_t... rgb_data)
    inline std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec3* rgb)> ImageWriteFunction_RGB(const std::string& extension)
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
        else if(extension == ".raw" || extension == ".RAW")
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
}
