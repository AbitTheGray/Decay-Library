#pragma once

#include <filesystem>
#include <vector>
#include <glm/glm.hpp>
#include <iostream>

#include "../Common.hpp"

namespace Decay::Wad
{
    class WadParser
    {
    public:
        explicit WadParser(const std::filesystem::path& filename);

        ~WadParser();

    public:
        // https://developer.valvesoftware.com/wiki/WAD
        enum class ItemType : uint8_t
        {
            /// Simple image with any size.
            Image = 0x42,
            /// Image with mipmap levels.
            Texture = 0x43,
            /// 256 ASCII characters.
            Font = 0x46,
        };
        struct Item
        {
            std::string Name;
            ItemType Type;
            std::size_t Size;
            void* Data;

        public:
            [[nodiscard]] std::istream DataAsStream() const
            {
                MemoryBuffer itemDataBuffer(
                        reinterpret_cast<char*>(Data),
                        reinterpret_cast<char*>(Data) + Size
                );
                return std::istream(&itemDataBuffer);
            }
        };

    private:
        std::vector<Item> m_Items = {};
    public:
        [[nodiscard]] inline const std::vector<Item>& GetItems() const noexcept { return m_Items; }

    public:
#define WADPARSER_GET_COUNT(funcName, itemType) \
        [[nodiscard]] inline std::size_t funcName() const noexcept\
        {\
            std::size_t count = 0;\
            for(const Item& item : m_Items)\
                if(item.Type == ItemType::itemType)\
                    count++;\
            return count;\
        }\

        WADPARSER_GET_COUNT(GetImageCount, Image)
        WADPARSER_GET_COUNT(GetTextureCount, Texture)
        WADPARSER_GET_COUNT(GetFontCount, Font)

    public:
#define WADPARSER_READ_ITEM(type, funcName, funcNameAll) \
        [[nodiscard]] static type funcName(const Item& item);\
        \
        [[nodiscard]] inline type funcName(const std::string& name) const\
        {\
            for(const Item& item : m_Items)\
                if(item.Type == ItemType::Image)\
                    if(item.Name == name)\
                        return WadParser::funcName(item);\
            throw std::runtime_error("Item not found");\
        }\
        [[nodiscard]] inline type funcName(const char* name) const\
        {\
            for(const Item& item : m_Items)\
                if(item.Type == ItemType::type)\
                    if(item.Name == name)\
                        return WadParser::funcName(item);\
            throw std::runtime_error("Item not found");\
        }\
        [[nodiscard]] inline std::vector<type> funcNameAll() const\
        {\
            std::vector<type> result = {};\
            for(const Item& item : m_Items)\
            {\
                if(item.Type == ItemType::type)\
                {\
                    try\
                    {\
                        result.emplace_back(funcName(item));\
                    }\
                    catch(std::exception& ex)\
                    {\
                        std::cerr << "Problem in batch-loading from WAD during " << item.Name << ": " << ex.what() << std::endl;\
                    }\
                }\
            }\
            return result;\
        }

    // Image
    public:
        class Image
        {
        public:
            uint32_t Width, Height;
            /// Length = Width * Height
            std::vector<uint8_t> Data;
            std::vector<glm::u8vec3> Palette;

        public:
            [[nodiscard]] inline std::vector<glm::u8vec3> AsRgb() const
            {
                std::vector<glm::u8vec3> pixels(Data.size());
                for(std::size_t i = 0; i < Data.size(); i++)
                    pixels[i] = Palette[Data[i]];
                return pixels;
            }
            [[nodiscard]] inline std::vector<glm::u8vec4> AsRgba() const
            {
                bool paletteTransparent = Palette.size() == 256 && Palette[255] == glm::u8vec3(0x00, 0x00, 0xFF);

                std::vector<glm::u8vec4> pixels(Data.size());
                for(std::size_t i = 0; i < Data.size(); i++)
                {
                    auto paletteIndex = Data[i];

                    if(paletteIndex == 255 && paletteTransparent)
                        pixels[i] = glm::u8vec4(0x00, 0x00, 0xFF, 0xFF); // Transparent
                    else
                        pixels[i] = glm::u8vec4(Palette[paletteIndex], 0x00); // Solid
                }
                return pixels;
            }
            void WriteRgbPng(const std::filesystem::path& filename) const;
            void WriteRgbaPng(const std::filesystem::path& filename) const;
        };

        WADPARSER_READ_ITEM(Image, ReadImage, ReadAllImages)

    // Font
    public:
        struct FontChar
        {
            uint16_t Offset;
            uint16_t Width;
        };
        static_assert(sizeof(FontChar) == sizeof(uint32_t));
        class Font : public Image
        {
        public:
            uint32_t RowCount;
            uint32_t RowHeight;
            static const std::size_t CharacterCount = 256;
            FontChar Characters[CharacterCount];
        };

        WADPARSER_READ_ITEM(Font, ReadFont, ReadAllFonts)

    // Texture
    public:
        struct Texture
        {
            static const std::size_t MaxNameLength = 16;
            std::string Name;
            uint32_t Width, Height;
            static const std::size_t MipMapLevels = 4;
            glm::u32vec2 MipMapDimensions[MipMapLevels];
            std::vector<uint8_t> MipMapData[MipMapLevels];
            static const std::size_t PaletteSize = 256;
            std::array<glm::u8vec3, PaletteSize> Palette;

        public:
            [[nodiscard]] inline std::vector<glm::u8vec3> AsRgb(std::size_t level = 0) const
            {
                assert(level < MipMapLevels);

                std::vector<glm::u8vec3> pixels(MipMapData[level].size());
                for(std::size_t i = 0; i < MipMapData[level].size(); i++)
                    pixels[i] = Palette[MipMapData[level][i]];
                return pixels;
            }
            [[nodiscard]] inline std::vector<glm::u8vec4> AsRgba(std::size_t level = 0) const
            {
                assert(level < MipMapLevels);

                bool paletteTransparent = Palette[255] == glm::u8vec3(0x00, 0x00, 0xFF);

                std::vector<glm::u8vec4> pixels(MipMapData[level].size());
                for(std::size_t i = 0; i < MipMapData[level].size(); i++)
                {
                    auto paletteIndex = MipMapData[level][i];

                    if(paletteIndex == 255 && paletteTransparent)
                        pixels[i] = glm::u8vec4(0x00, 0x00, 0xFF, 0xFF); // Transparent
                    else
                        pixels[i] = glm::u8vec4(Palette[paletteIndex], 0x00); // Solid
                }
                return pixels;
            }
            void WriteRgbPng(const std::filesystem::path& filename, std::size_t level = 0) const;
            void WriteRgbaPng(const std::filesystem::path& filename, std::size_t level = 0) const;
        };

        WADPARSER_READ_ITEM(Texture, ReadTexture, ReadAllTextures)
    };
}
