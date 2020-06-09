#pragma once

#include <filesystem>
#include <vector>
#include <glm/glm.hpp>

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
    };
}
