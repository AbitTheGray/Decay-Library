#include "WadFile.hpp"

// Use to test images if you are getting weird results.
// Will replace palette for all images with HSL/HSV noise.
//#define WAD_PALETTE_DUMMY 1

// Use to replace all images by their palette.
//#define WAD_PALETTE_RESULT 1

#include <fstream>
#include <iostream>

#include <stb_image_write.h>

#include "../Bsp/BspTree.hpp"

#ifdef WAD_PALETTE_DUMMY
    #include <glm/gtx/color_space.hpp>
#endif

namespace Decay::Wad
{
    WadFile::WadFile(const std::filesystem::path& filename)
    {
        if(!std::filesystem::exists(filename))
            throw std::runtime_error("File not found");

        std::fstream file(filename, std::ios_base::binary | std::ios_base::in);

        // Magic Number
        {
            char magicNumber[4];
            file.read(reinterpret_cast<char*>(&magicNumber), sizeof(char) * 4);

            if(
                    magicNumber[0] == 'W' &&
                    magicNumber[1] == 'A' &&
                    magicNumber[2] == 'D'
                    )
            {
                if(magicNumber[3] == '2' || magicNumber[3] == '3')
                {
                    // There seem to be no difference
                }
                else
                    throw std::runtime_error("Invalid magic number, only WAD versions supported are 2 and 3");
            }
            else
                throw std::runtime_error("Invalid magic number");
        }

        uint32_t entriesCount;
        file.read(reinterpret_cast<char*>(&entriesCount), sizeof(entriesCount));

        if(entriesCount == 0) // No entries in the file
        {
            file.close(); // Just to make sure
            return;
        }

        /// Offset in file to Entry structure
        uint32_t entriesOffset;
        file.read(reinterpret_cast<char*>(&entriesOffset), sizeof(entriesOffset));
        assert(entriesOffset >= ((sizeof(uint32_t) * 2) + (sizeof(char) * 4)));

        file.seekg(entriesOffset);

        static const std::size_t EntryNameLength = 16;
        struct Entry
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

            [[nodiscard]] inline std::string GetName() const { return Cstr2Str(Name, EntryNameLength); }
        };

        std::vector<Entry> entries(entriesCount);
        file.read(reinterpret_cast<char*>(entries.data()), sizeof(Entry) * entriesCount);

        for(const Entry& entry : entries)
        {
            file.seekg(entry.Offset);

            std::size_t dataLength = entry.DiskSize;
            void* data = std::malloc(dataLength);
            file.read(static_cast<char*>(data), dataLength);

            if(entry.Compression)
            {
                throw std::runtime_error("Not Supported");

                std::size_t dataRawLength = entry.Size;
                void* dataRaw = std::malloc(dataRawLength);

                //TODO Uncompress `dataRaw` into `data`
                // Not implemented inside GoldSrc
                // but it seems they decided on LZSS (Lempel–Ziv–Storer–Szymanski, https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski)

                std::free(data);

                dataLength = dataRawLength;
                data = dataRaw;
            }

            m_Items.emplace_back(Item{entry.GetName(), static_cast<ItemType>(entry.Type), dataLength, data});
        }
    }

    WadFile::~WadFile()
    {
        for(Item& item : m_Items)
        {
            std::free(item.Data);
            item.Data = nullptr;
        }
    }

    WadFile::Image WadFile::ReadImage(const WadFile::Item& item)
    {
        MemoryBuffer itemDataBuffer(reinterpret_cast<char*>(item.Data), item.Size);
        std::istream in(&itemDataBuffer);

        Image image = {};
        in.read(reinterpret_cast<char*>(&image.Width), sizeof(image.Width));
        in.read(reinterpret_cast<char*>(&image.Height), sizeof(image.Height));
        assert(image.Width > 0);
        assert(image.Height > 0);

        // Calculate data length
        std::size_t dataLength = static_cast<std::size_t>(image.Width) * image.Height;
        assert(dataLength > 0);

        // Read data
        image.Data.resize(dataLength);
        in.read(reinterpret_cast<char*>(image.Data.data()), dataLength);

        // Read palette length
        uint16_t paletteLength;
        in.read(reinterpret_cast<char*>(&paletteLength), sizeof(paletteLength));
        assert(paletteLength > 0);
        assert(paletteLength <= 256);

        // Read palette
        image.Palette.resize(paletteLength);
        in.read(reinterpret_cast<char*>(image.Palette.data()), paletteLength);

#ifdef WAD_PALETTE_DUMMY
        for(std::size_t i = 0, pi = 0; i < 360 && pi < paletteLength; i += 360 / paletteLength, pi++)
        {
            double hue = i;
            double saturation = 90 + std::rand() / (double)RAND_MAX * 10;
            double lightness = 50 + std::rand() / (double)RAND_MAX * 10;
            glm::dvec3 hsv = {hue, saturation, lightness};
            glm::dvec3 rgb = glm::rgbColor(hsv);

            image.Palette[pi] = {rgb.r, rgb.g, rgb.b};
        }
#endif

#ifdef WAD_PALETTE_RESULT
        image.Width = 16;
        image.Height = paletteLength / 16 + (paletteLength % 16 ? 1 : 0);

        image.Data.resize(image.Width * image.Height);
        for(std::size_t i = 0; i < paletteLength; i++)
            image.Data[i] = i;

        for(std::size_t i = paletteLength; i < image.Width * image.Height; i++)
            image.Data[i] = paletteLength - 1;
#endif

        return image;
    }

    void WadFile::Image::WriteRgbPng(const std::filesystem::path& filename) const
    {
        std::vector<glm::u8vec3> pixels = AsRgb();
        assert(pixels.size() == Width * Height);
        assert(Width <= std::numeric_limits<int32_t>::max() / 3);
        stbi_write_png(filename.string().c_str(), Width, Height, 3, pixels.data(), static_cast<int32_t>(Width) * 3);
    }
    void WadFile::Image::WriteRgbaPng(const std::filesystem::path& filename) const
    {
        std::vector<glm::u8vec4> pixels = AsRgba();
        assert(pixels.size() == Width * Height);
        assert(Width <= std::numeric_limits<int32_t>::max() / 4);
        stbi_write_png(filename.string().c_str(), Width, Height, 4, pixels.data(), static_cast<int32_t>(Width) * 4);
    }

    WadFile::Font WadFile::ReadFont(const WadFile::Item& item)
    {
        MemoryBuffer itemDataBuffer(reinterpret_cast<char*>(item.Data), item.Size);
        std::istream in(&itemDataBuffer);

        Font font = {};

        // Dimensions
        {
            glm::u32vec4 dimensions; // width, height, row count + height
            in.read(reinterpret_cast<char*>(&dimensions), sizeof(dimensions));
            font.Width = dimensions.x;
            font.Height = dimensions.y;
            font.RowCount = dimensions.z;
            font.RowHeight = dimensions.w;
        }
        assert(font.Width == 256); //FIXME It is defined that fonts have Width=256 ( https://developer.valvesoftware.com/wiki/WAD ) but 2 tested fonts did not have
        assert(font.RowCount > 0);
        assert(font.RowHeight > 0);
        assert(font.Height == font.RowCount * font.RowHeight);

        // Character offsets
        in.read(reinterpret_cast<char*>(&font.Characters), sizeof(FontChar) * Font::CharacterCount);

        // Calculate data length
        std::size_t dataLength = static_cast<std::size_t>(font.Width) * font.Height;
        assert(dataLength > 0);

        // Read data
        font.Data.resize(dataLength);
        in.read(reinterpret_cast<char*>(font.Data.data()), dataLength);

        // Read palette length
        uint16_t paletteLength;
        in.read(reinterpret_cast<char*>(&paletteLength), sizeof(paletteLength));
        assert(paletteLength > 0);
        assert(paletteLength <= 256);

        // Read palette
        font.Palette.resize(paletteLength);
        in.read(reinterpret_cast<char*>(font.Palette.data()), paletteLength);

#ifdef WAD_PALETTE_DUMMY
        for(std::size_t i = 0, pi = 0; i < 360 && pi < paletteLength; i += 360 / paletteLength, pi++)
        {
            double hue = i;
            double saturation = 90 + std::rand() / (double)RAND_MAX * 10;
            double lightness = 50 + std::rand() / (double)RAND_MAX * 10;
            glm::dvec3 hsv = {hue, saturation, lightness};
            glm::dvec3 rgb = glm::rgbColor(hsv);

            font.Palette[pi] = {rgb.r, rgb.g, rgb.b};
        }
#endif

        return font;
    }

    WadFile::Texture WadFile::ReadTexture(const WadFile::Item& item)
    {
        MemoryBuffer itemDataBuffer(reinterpret_cast<char*>(item.Data), item.Size);
        std::istream in(&itemDataBuffer);

        Texture texture = {};

        // Name
        {
            char name[Texture::MaxNameLength];
            in.read(name, Texture::MaxNameLength);
            texture.Name = Cstr2Str(name, Texture::MaxNameLength);
            assert(texture.Name.length() > 0);
#ifdef DEBUG
            if(item.Name != texture.Name)
            {
                std::cerr << "Item and texture names do not match: " << item.Name << " != " << texture.Name << std::endl;
                assert(StringCaseInsensitiveEqual(item.Name, texture.Name));
            }
#endif
        }

        // Dimensions
        in.read(reinterpret_cast<char*>(&texture.Width), sizeof(Texture::Width) + sizeof(Texture::Height));
        assert(texture.Width >= (1u << Texture::MipMapLevels));
        assert(texture.Height >= (1u << Texture::MipMapLevels));
        if(!IsMultipleOf2(texture.Width))
            assert(texture.Width % 16 == 0);
        if(!IsMultipleOf2(texture.Height))
            assert(texture.Height % 16 == 0);

        // Offsets
        uint32_t mipMapOffsets[Texture::MipMapLevels];
        in.read(reinterpret_cast<char*>(mipMapOffsets), sizeof(uint32_t) * Texture::MipMapLevels);

        for(std::size_t level = 0; level < Texture::MipMapLevels; level++)
        {
            in.seekg(mipMapOffsets[level]);

            texture.MipMapDimensions[level] = {
                    texture.Width >> level,
                    texture.Height >> level
            };
            std::size_t dataLength = static_cast<std::size_t>(texture.MipMapDimensions[level].x) * texture.MipMapDimensions[level].y;

            std::vector<uint8_t>& data = texture.MipMapData[level];
            data.resize(dataLength);
            in.read(reinterpret_cast<char*>(data.data()), dataLength);
        }
        assert(texture.Width == texture.MipMapDimensions[0].x);
        assert(texture.Height == texture.MipMapDimensions[0].y);

        // 2 Dummy bytes
        // after last MipMap level
        uint8_t dummy[2];
        in.read(reinterpret_cast<char*>(dummy), sizeof(uint8_t) * 2);
        if(dummy[0] != 0x00u)
            std::cerr << "Texture dummy[0] byte not equal to 0x00 but " << static_cast<uint32_t>(dummy[0]) << " was read." << std::endl;
        if(dummy[1] != 0x01u)
            std::cerr << "Texture dummy[1] byte not equal to 0x01 but " << static_cast<uint32_t>(dummy[1]) << " was read." << std::endl;

        // Palette
        in.read(reinterpret_cast<char*>(texture.Palette.data()), sizeof(glm::u8vec3) * Texture::PaletteSize);

#ifdef WAD_PALETTE_DUMMY
        for(std::size_t i = 0, pi = 0; i < 360 && pi < Texture::PaletteSize; i += 360 / Texture::PaletteSize, pi++)
        {
            double hue = i;
            double saturation = 90 + std::rand() / (double)RAND_MAX * 10;
            double lightness = 50 + std::rand() / (double)RAND_MAX * 10;
            glm::dvec3 hsv = {hue, saturation, lightness};
            glm::dvec3 rgb = glm::rgbColor(hsv);

            texture.Palette[pi] = {rgb.r, rgb.g, rgb.b};
        }
#endif

        return texture;
    }

    void WadFile::ExportTextures(const std::filesystem::path& directory, const std::string& extension) const
    {
        if(!std::filesystem::exists(directory))
            std::filesystem::create_directory(directory);

        assert(extension.size() > 1);
        assert(extension[0] == '.');

        std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data)> writeFunc = Bsp::BspTree::GetImageWriteFunction(extension);

        for(auto& texture : ReadAllTextures())
        {
            std::vector<glm::u8vec4> rgba = texture.AsRgba();

            writeFunc(
                    (directory / (texture.Name + extension)).string().c_str(),
                    texture.Width,
                    texture.Height,
                    rgba.data()
            );
        }
    }

    void WadFile::Texture::WriteRgbPng(const std::filesystem::path& filename, std::size_t level) const
    {
        assert(level < MipMapLevels);

        std::vector<glm::u8vec3> pixels = AsRgb(level);
        glm::u32vec2 dimension = MipMapDimensions[level];

        assert(pixels.size() == dimension.x * dimension.y);
        assert(dimension.x <= std::numeric_limits<int32_t>::max() / 3);

        stbi_write_png(
                filename.string().c_str(),
                dimension.x,
                dimension.y,
                3,
                pixels.data(),
                static_cast<int32_t>(dimension.x) * 3
                );
    }

    void WadFile::Texture::WriteRgbaPng(const std::filesystem::path& filename, std::size_t level) const
    {
        assert(level < MipMapLevels);

        std::vector<glm::u8vec4> pixels = AsRgba(level);
        glm::u32vec2 dimension = MipMapDimensions[level];

        assert(pixels.size() == dimension.x * dimension.y);
        assert(dimension.x <= std::numeric_limits<int32_t>::max() / 4);

        stbi_write_png(
                filename.string().c_str(),
                dimension.x,
                dimension.y,
                4,
                pixels.data(),
                static_cast<int32_t>(dimension.x) * 4
        );
    }
}
