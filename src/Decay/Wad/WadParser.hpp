#pragma once

#include <filesystem>
#include <vector>
#include <glm/glm.hpp>
#include <iostream>

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
            MipMapTexture = 0x43,
            /// 256 ASCII characters.
            Font = 0x46,
        };
        struct Item
        {
            std::string Name;
            ItemType Type;
            std::size_t Size;
            void* Data;
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
        WADPARSER_GET_COUNT(GetTextureCount, MipMapTexture)
        WADPARSER_GET_COUNT(GetFontCount, Font)

    public:
        struct Image
        {
            int32_t Width, Height;
            /// Length = Width * Height
            std::vector<uint8_t> Data;
            std::vector<glm::i8vec3> Palette;

        public:
            [[nodiscard]] inline std::vector<glm::i8vec3> AsPixels() const
            {
                std::vector<glm::i8vec3> pixels(Data.size());
                for(std::size_t i = 0; i < Data.size(); i++)
                    pixels[i] = Palette[Data[i]];
                return pixels;
            }
            void WritePng(const std::filesystem::path& filename) const;
        };

        [[nodiscard]] static Image ReadImage(const Item& item);

        [[nodiscard]] inline Image ReadImage(const std::string& name) const
        {
            for(const Item& item : m_Items)
                if(item.Type == ItemType::Image)
                    if(item.Name == name)
                        return ReadImage(item);

            throw std::runtime_error("Item not found");
        }
        [[nodiscard]] inline Image ReadImage(const char* name) const
        {
            for(const Item& item : m_Items)
                if(item.Type == ItemType::Image)
                    if(item.Name == name)
                        return ReadImage(item);

            throw std::runtime_error("Item not found");
        }
        [[nodiscard]] inline std::vector<Image> ReadAllImages() const
        {
            std::vector<Image> images = {};

            for(const Item& item : m_Items)
            {
                if(item.Type == ItemType::Image)
                {
                    try
                    {
                        images.emplace_back(ReadImage(item));
                    }
                    catch(std::exception& ex)
                    {
                        std::cerr << "Problem in batch-loading of images from WAD during " << item.Name << ": " << ex.what() << std::endl;
                    }
                }
            }

            return images;
        }
    };
}
