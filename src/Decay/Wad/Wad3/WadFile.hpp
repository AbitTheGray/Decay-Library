#pragma once

#include <filesystem>
#include <vector>
#include <map>
#include "glm/glm.hpp"
#include <iostream>

#include "Decay/Common.hpp"

namespace Decay::Wad::Wad3
{
    class WadFile
    {
    public:
        WadFile() = default;
        explicit WadFile(const std::filesystem::path& filename);

        ~WadFile();

    public:
        // https://developer.valvesoftware.com/wiki/WAD
        enum class ItemType : uint8_t
        {
            /// Simple image with any size.
            Image = 0x42, // 66
            /// Image with mipmap levels.
            Texture = 0x43, // 67
            /// 256 ASCII characters.
            Font = 0x46, // 70
        };
        struct Item
        {
            std::string Name;
            ItemType Type;
            std::size_t Size;
            void* Data;
        };

    private:
        static const std::size_t EntryNameLength = 16;
        struct WadEntry
        {
            int32_t Offset;
            int32_t DiskSize;
            /// Uncompressed size
            int32_t Size;
            int8_t Type;
            /// Not implemented in official code
            bool Compression;
            /// Not used
            /// Referred to in official code as `pad1` and `pad2`
            int16_t Dummy;

            /// Must be null-terminated.
            char Name[EntryNameLength];

            [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, EntryNameLength); }
            inline void Name_str(const std::string& val) { Str2Cstr(val, Name, EntryNameLength); }
        };

        static std::vector<WadEntry> ReadWadEntries(std::istream& stream);

    private:
        std::vector<Item> m_Items{};
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
#ifdef DEBUG
#   define WADPARSER_READ_ITEM(type, funcName, funcNameAll, funcNameAll_map) \
        [[nodiscard]] static type funcName(const Item& item);\
        \
        [[nodiscard]] inline type funcName(const std::string& name) const\
        {\
            for(const Item& item : m_Items)\
                if(item.Type == ItemType::Image)\
                    if(item.Name == name)\
                        return WadFile::funcName(item);\
            throw std::runtime_error("Item not found");\
        }\
        [[nodiscard]] inline type funcName(const char* name) const\
        {\
            for(const Item& item : m_Items)\
                if(item.Type == ItemType::type)\
                    if(item.Name == name)\
                        return WadFile::funcName(item);\
            throw std::runtime_error("Item not found");\
        }\
        [[nodiscard]] inline std::vector<type> funcNameAll() const\
        {\
            std::vector<type> result = {};\
            for(const Item& item : m_Items)\
            {\
                if(item.Type == ItemType::type)\
                {\
                    result.emplace_back(funcName(item));\
                }\
            }\
            return result;\
        }\
        [[nodiscard]] inline std::map<std::string, type> funcNameAll_map() const\
        {\
            std::map<std::string, type> result = {};\
            for(const Item& item : m_Items)\
            {\
                if(item.Type == ItemType::type)\
                {\
                    result.emplace(item.Name, funcName(item));\
                }\
            }\
            return result;\
        }
#else
#   define WADPARSER_READ_ITEM(type, funcName, funcNameAll, funcNameAll_map) \
        [[nodiscard]] static type funcName(const Item& item);\
        \
        [[nodiscard]] inline type funcName(const std::string& name) const\
        {\
            for(const Item& item : m_Items)\
                if(item.Type == ItemType::Image)\
                    if(item.Name == name)\
                        return WadFile::funcName(item);\
            throw std::runtime_error("Item not found");\
        }\
        [[nodiscard]] inline type funcName(const char* name) const\
        {\
            for(const Item& item : m_Items)\
                if(item.Type == ItemType::type)\
                    if(item.Name == name)\
                        return WadFile::funcName(item);\
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
        }\
        [[nodiscard]] inline std::map<std::string, type> funcNameAll_map() const\
        {\
            std::map<std::string, type> result = {};\
            for(const Item& item : m_Items)\
            {\
                if(item.Type == ItemType::type)\
                {\
                    try\
                    {\
                        result.emplace(item.Name, funcName(item));\
                    }\
                    catch(std::exception& ex)\
                    {\
                        std::cerr << "Problem in batch-loading from WAD during " << item.Name << ": " << ex.what() << std::endl;\
                    }\
                }\
            }\
            return result;\
        }
#endif

    // Image
    public:
        class Image
        {
        public:
            union
            {
                struct
                {
                    uint32_t Width, Height;
                };
                glm::u32vec2 Size{};
            };
            /// Length = Width * Height
            std::vector<uint8_t> Data{};
            std::vector<glm::u8vec3> Palette{};

        public:
            Image() = default;
            Image(uint32_t width, uint32_t height) : Width(width), Height(height) {}
            explicit Image(std::istream&);

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
                        pixels[i] = glm::u8vec4(0x00, 0x00, 0xFF, 0x00); // Transparent
                    else
                        pixels[i] = glm::u8vec4(Palette[paletteIndex], 0xFF); // Solid
                }
                return pixels;
            }
            void WriteRgbPng(const std::filesystem::path& filename) const
            {
                std::vector<glm::u8vec3> pixels = AsRgb();
                auto func = ImageWriteFunction_RGB(filename.extension());
                func(filename.string().c_str(), Width, Height, pixels.data());
            }
            void WriteRgbaPng(const std::filesystem::path& filename) const
            {
                std::vector<glm::u8vec4> pixels = AsRgba();
                auto func = ImageWriteFunction_RGBA(filename.extension());
                func(filename.string().c_str(), Width, Height, pixels.data());
            }


        public:
            /// Converts Image to raw WAD-ready Item
            /// Caller has to deallocate Item.Data
            [[nodiscard]] virtual Item AsItem(std::string name) const;
        };

        WADPARSER_READ_ITEM(Image, ReadImage, ReadAllImages, ReadAllImages_Map)

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
            uint32_t RowCount{};
            uint32_t RowHeight{};
            static const std::size_t CharacterCount = 256;
            FontChar Characters[CharacterCount]{};

        public:
            Font() = default;
            Font(uint32_t width, uint32_t height) : Image(width, height) {}
            explicit Font(std::istream&);

        public:
            [[nodiscard]] Item AsItem(std::string name) const override;

            void WriteCharacterPngs(const std::filesystem::path& dir) const;
        };

        WADPARSER_READ_ITEM(Font, ReadFont, ReadAllFonts, ReadAllFonts_Map)

    // Texture
    public:
        struct Texture
        {
            static const std::size_t MaxNameLength = 16;
            std::string Name;
            union
            {
                struct
                {
                    uint32_t Width, Height;
                };
                glm::u32vec2 Size{};
            };
            static_assert(sizeof(Size) == sizeof(Width) + sizeof(Height));

            static const std::size_t MipMapLevels = 4;
            glm::u32vec2 MipMapDimensions[MipMapLevels]{};
            std::vector<uint8_t> MipMapData[MipMapLevels]{};

            std::vector<glm::u8vec3> Palette{};

        public:
            Texture() = default;
            Texture(std::string name, uint32_t width, uint32_t height);
            Texture(std::string name, glm::u32vec2 size);
            Texture(std::istream&);

        public:
            [[nodiscard]] inline std::vector<glm::u8vec3> AsRgb(std::size_t level = 0) const
            {
                R_ASSERT(level < MipMapLevels, "Requested mip-map level is too high");

                std::vector<glm::u8vec3> pixels(MipMapData[level].size());
                for(std::size_t i = 0; i < MipMapData[level].size(); i++)
                    pixels[i] = Palette[MipMapData[level][i]];
                return pixels;
            }
            [[nodiscard]] inline std::vector<glm::u8vec4> AsRgba(std::size_t level = 0) const
            {
                R_ASSERT(level < MipMapLevels, "Requested mip-map level is too high");

                bool paletteTransparent = Palette[255] == glm::u8vec3(0x00, 0x00, 0xFF);

                std::vector<glm::u8vec4> pixels(MipMapData[level].size());
                for(std::size_t i = 0; i < MipMapData[level].size(); i++)
                {
                    auto paletteIndex = MipMapData[level][i];

                    if(paletteIndex == 255 && paletteTransparent)
                        pixels[i] = glm::u8vec4(0x00, 0x00, 0xFF, 0x00); // Transparent
                    else
                        pixels[i] = glm::u8vec4(Palette[paletteIndex], 0xFF); // Solid
                }
                return std::move(pixels);
            }
            void WriteRgbPng(const std::filesystem::path& filename, std::size_t level = 0) const;
            void WriteRgbaPng(const std::filesystem::path& filename, std::size_t level = 0) const;

        public:
            //[[nodiscard]] static Texture FromFile(const std::filesystem::path& filename);
            [[nodiscard]] Item AsItem() const;
            [[nodiscard]] inline bool HasData() const noexcept { return !MipMapData[0].empty(); }
            [[nodiscard]] inline Texture CopyWithoutData() const { return Texture(Name, Size); }

        };

        WADPARSER_READ_ITEM(Texture, ReadTexture, ReadAllTextures, ReadAllTextures_Map)

        void ExportTextures(const std::filesystem::path& directory, const std::string& extension = ".png") const;

        static std::size_t AddToFile(const std::filesystem::path& filename, const std::vector<Item>& items);
        static std::size_t AddToFile(
                const std::filesystem::path& filename,
                const std::vector<Texture>& textures,
                const std::map<std::string, Font>& fonts,
                const std::map<std::string, Image>& images
        )
        {
            std::vector<Item> items = {};
            items.reserve(textures.size() + fonts.size() + images.size());

            for(auto& texture : textures)
                if(texture.HasData())
                    items.emplace_back(texture.AsItem());

            for(auto& it : fonts)
                items.emplace_back(it.second.AsItem(it.first));

            for(auto& it : images)
                items.emplace_back(it.second.AsItem(it.first));

            std::size_t count = AddToFile(filename, items);

            //for(auto& item : items)
            //    free(item.Data);

            return count;
        }
    };
}
