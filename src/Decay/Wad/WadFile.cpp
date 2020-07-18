#define GLM_FORCE_SWIZZLE
#include "WadFile.hpp"

// Use to test images if you are getting weird results.
// Will replace palette for all images with HSL/HSV noise.
//#define WAD_PALETTE_DUMMY 1

// Use to replace all images by their palette.
//#define WAD_PALETTE_RESULT 1

#include <fstream>
#include <iostream>

#include <stb_image_write.h>
#include <stb_image.h>

#include "../Bsp/BspTree.hpp"

#ifdef WAD_PALETTE_DUMMY
    #include <glm/gtx/color_space.hpp>
#endif

namespace Decay::Wad
{
    std::vector<WadFile::WadEntry> WadFile::ReadWadEntries(std::istream& stream)
    {
        // Magic Number
        {
            char magicNumber[4];
            stream.read(reinterpret_cast<char*>(&magicNumber), sizeof(char) * 4);

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
        stream.read(reinterpret_cast<char*>(&entriesCount), sizeof(entriesCount));

        if(entriesCount == 0) // No entries in the file
            return {};

        /// Offset in file to WadEntry structure
        uint32_t entriesOffset;
        stream.read(reinterpret_cast<char*>(&entriesOffset), sizeof(entriesOffset));
        assert(entriesOffset >= ((sizeof(uint32_t) * 2) + (sizeof(char) * 4)));

        stream.seekg(entriesOffset);

        std::vector<WadEntry> entries(entriesCount);
        stream.read(reinterpret_cast<char*>(entries.data()), sizeof(WadEntry) * entriesCount);

        return entries;
    }

    WadFile::WadFile(const std::filesystem::path& filename)
    {
        if(!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename))
            throw std::runtime_error("File not found");

        std::fstream file(filename, std::ios_base::binary | std::ios_base::in);

        std::vector<WadEntry> entries = ReadWadEntries(file);
        if(entries.empty())
            return;

        m_Items.resize(entries.size());

        for(const WadEntry& entry : entries)
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

    void WadFile::AddToFile(const std::filesystem::path& filename, const std::vector<Item>& items)
    {
        if(!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename))
            throw std::runtime_error("File not found");

        char magic[4];
        std::vector<WadEntry> entries;
        std::vector<void*> entryData;
        {
            std::ifstream in(filename, std::ios_base::binary | std::ios_base::in);

            // Store magic number
            in.read(magic, sizeof(magic));
            in.seekg(0);

            entries = ReadWadEntries(in);
            if(entries.empty())
                return;

            entryData.resize(entries.size());

            for(const WadEntry& entry : entries)
            {
                in.seekg(entry.Offset);

                std::size_t dataLength = entry.DiskSize;
                void* data = std::malloc(dataLength);
                in.read(static_cast<char*>(data), dataLength);

                entryData.emplace_back(data);
            }

            in.close();
        }
        int originalEntryCount = entries.size();

        // Add items to entries
        {
            for(const Item& item : items)
            {
                WadEntry entry = {};
                entry.Compression = false;
                entry.Type = static_cast<int8_t>(item.Type);

                entry.DiskSize = item.Size;
                entry.Size = item.Size;

                // Copy name
                if(item.Name.length() > 15)
                    throw std::runtime_error("Entry name too long");
                std::copy(item.Name.c_str(), item.Name.c_str() + item.Name.size(), entry.Name);
                std::fill(entry.Name + item.Name.size(), entry.Name + 16, '\0');

                entries.emplace_back(entry);
                entryData.emplace_back(item.Data);
            }
        }

        std::ofstream out(filename, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

        // Write original magic number
        out.write(magic, sizeof(magic));

        // Write item count
        {
            uint32_t itemCount = entries.size();
            out.write(reinterpret_cast<const char*>(&itemCount), sizeof(itemCount));
        }

        // Write entry offset
        uint32_t entriesOffset = sizeof(char[4]) + sizeof(uint32_t) + sizeof(uint32_t); // magic + itemCount + entriesOffset
        out.write(reinterpret_cast<const char*>(&entriesOffset), sizeof(entriesOffset));

        // Write entries
        out.write(reinterpret_cast<const char*>(entries.data()), sizeof(WadEntry) * entries.size());

        // Write entry data
        std::vector<uint32_t> offsets(entries.size());
        for(int i = 0; i < entries.size(); i++)
        {
            offsets.emplace_back(out.tellp());

            out.write(static_cast<const char*>(entryData[i]), entries[i].Size);
        }

        // Override Offset in Entries
        for(int i = 0; i < entries.size(); i++)
        {
            out.seekp(entriesOffset + sizeof(WadEntry) * i + offsetof(WadEntry, Offset));

            out.write(reinterpret_cast<const char*>(offsets.data() + i), sizeof(uint32_t));
        }

        // Free allocated memory from existing data
        for(int i = 0; i < originalEntryCount; i++)
            free(entryData[i]);
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
        uint16_t paletteSize;
        in.read(reinterpret_cast<char*>(&paletteSize), sizeof(paletteSize));
        if(paletteSize == 0)
            throw std::runtime_error("Empty Palette");
        if(paletteSize > 256)
            throw std::runtime_error("Palette size too big");

        // Read palette
        image.Palette.resize(paletteSize);
        in.read(reinterpret_cast<char*>(image.Palette.data()), paletteSize);

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

    WadFile::Image WadFile::Image::FromFile(const std::filesystem::path& filename)
    {
        if(!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename))
            throw std::runtime_error("File not found");

        int width, height, originalChannels;
        glm::u8vec4* data = reinterpret_cast<glm::u8vec4*>(stbi_load(filename.string().c_str(), &width, &height, &originalChannels, 4));

        if(data == nullptr)
            throw std::runtime_error("Failed to load image file");
        if(width == 0 || height == 0)
            throw std::runtime_error("Loaded empty image");

        Image image = Image();
        image.Width = width;
        image.Height = height;
        image.Data.resize(width * height);
        image.Palette.resize(256);

        bool transparent = false;
        for(std::size_t i = 0; i < width * height; i++)
        {
            glm::u8vec3 rgb = data[i].rgb();
            if(data[i].a == 0xFFu)
            {
                transparent = true;
                image.Data[i] = 255;
                continue;
            }

            auto paletteIterator = std::find(image.Palette.begin(), image.Palette.end(), rgb);
            if(paletteIterator == image.Palette.end())
            { // Not found
                if(image.Palette.size() == (transparent ? 255 : 256))
                    throw std::runtime_error("Exceeded palette size");

                image.Data[i] = image.Palette.size();

                image.Palette.emplace_back(rgb);
            }
            else
                image.Data[i] = paletteIterator->length();
        }

        if(transparent)
        {
            for(int pi = image.Palette.size(); pi < 255; pi++)
                image.Palette[pi] = {0x00u, 0x00u, 0x00u};
            image.Palette[255] = {0x00u, 0x00u, 0xFFu};
        }
        else
            image.Palette.shrink_to_fit();

#ifdef DEBUG
        std::cout << "Loaded image from '" << filename << "' with palette size " << image.Palette.size();
        if(transparent)
            std::cout << " including transparency";
        std::cout << std::endl;
#endif

        stbi_image_free(data);

        return image;
    }

    static std::vector<uint8_t> GenerateMipMap(const glm::u32vec2& dimension, const std::vector<uint8_t>& image)
    {
        if(dimension.x == 1 && dimension.y == 1)
            throw std::runtime_error("Cannot generate mip-map for 1x1 image");
        if(dimension.x == 0 || dimension.y == 0)
            throw std::runtime_error("Cannot generate mip-map for image with invalid size");

        std::vector<uint8_t> low(image.size() / 4);

        int li = 0;
        int ii = 0;
        for(int y = 0; y < dimension.y; y++)
        {
            if(y % 2 == 1)
            {
                ii += dimension.x;
                continue;
            }

            for(int x = 0; x < dimension.x; x += 2, ii += 2)
                low[li++] = image[ii];
        }

        return low;
    }

    WadFile::Texture WadFile::Texture::FromFile(const std::filesystem::path& filename)
    {
        if(!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename))
            throw std::runtime_error("File not found");

        int width, height, originalChannels;
        glm::u8vec4* data = reinterpret_cast<glm::u8vec4*>(stbi_load(filename.string().c_str(), &width, &height, &originalChannels, 4));

        if(data == nullptr)
            throw std::runtime_error("Failed to load image file");
        if(width == 0 || height == 0)
            throw std::runtime_error("Loaded empty image");

        Texture texture = Texture();
        texture.Width = width;
        texture.Height = height;
        texture.MipMapData[0].resize(width * height);
        texture.Palette.resize(256);

        bool transparent = false;
        for(std::size_t i = 0; i < width * height; i++)
        {
            glm::u8vec3 rgb = data[i].rgb();
            if(data[i].a == 0xFFu)
            {
                transparent = true;
                texture.MipMapData[0][i] = 255;
                continue;
            }

            auto paletteIterator = std::find(texture.Palette.begin(), texture.Palette.end(), rgb);
            if(paletteIterator == texture.Palette.end())
            { // Not found
                if(texture.Palette.size() == (transparent ? 255 : 256))
                    throw std::runtime_error("Exceeded palette size");

                texture.MipMapData[0][i] = texture.Palette.size();

                texture.Palette.emplace_back(rgb);
            }
            else
                texture.MipMapData[0][i] = paletteIterator->length();
        }

        // Generate Mip-Maps
        {
            texture.MipMapDimensions[0] = glm::u32vec2(texture.Width, texture.Height);
            texture.MipMapDimensions[1] = glm::u32vec2(texture.Width / 2u, texture.Height / 2u);
            texture.MipMapDimensions[2] = glm::u32vec2(texture.Width / 4u, texture.Height / 4u);
            texture.MipMapDimensions[3] = glm::u32vec2(texture.Width / 8u, texture.Height / 8u);

            texture.MipMapData[1] = GenerateMipMap(texture.MipMapDimensions[0], texture.MipMapData[0]);
            texture.MipMapData[2] = GenerateMipMap(texture.MipMapDimensions[1], texture.MipMapData[1]);
            texture.MipMapData[3] = GenerateMipMap(texture.MipMapDimensions[2], texture.MipMapData[2]);
        }

        if(transparent)
        {
            for(int pi = texture.Palette.size(); pi < 255; pi++)
                texture.Palette[pi] = {0x00u, 0x00u, 0x00u};
            texture.Palette[255] = {0x00u, 0x00u, 0xFFu};
        }
        else
            texture.Palette.shrink_to_fit();

#ifdef DEBUG
        std::cout << "Loaded texture from '" << filename << "' with palette size " << texture.Palette.size();
        if(transparent)
            std::cout << " including transparency";
        std::cout << std::endl;
#endif

        stbi_image_free(data);

        return texture;
    }

    WadFile::Item WadFile::Image::AsItem(std::string name) const
    {
        Item item = {};
        item.Name = std::move(name);
        item.Type = ItemType::Image;

        item.Size = sizeof(Width) + sizeof(Height) +
                    sizeof(uint8_t) * Data.size() +
                    sizeof(uint16_t) + sizeof(glm::u8vec3) * Palette.size();
        item.Data = malloc(item.Size);

        MemoryBuffer itemDataBuffer(
                reinterpret_cast<char*>(item.Data),
                item.Size
        );
        std::ostream out(&itemDataBuffer);

        out.write(reinterpret_cast<const char*>(&Width), sizeof(Width));
        out.write(reinterpret_cast<const char*>(&Height), sizeof(Height));

        out.write(reinterpret_cast<const char*>(Data.data()), Data.size());

        short paletteSize = Palette.size();
        out.write(reinterpret_cast<const char*>(&paletteSize), sizeof(paletteSize));
        out.write(reinterpret_cast<const char*>(Palette.data()), sizeof(glm::u8vec3) * Palette.size());

        return item;
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
        uint16_t paletteSize;
        in.read(reinterpret_cast<char*>(&paletteSize), sizeof(paletteSize));
        if(paletteSize == 0)
            throw std::runtime_error("Empty Palette");
        if(paletteSize > 256)
            throw std::runtime_error("Palette size too big");

        // Read palette
        font.Palette.resize(paletteSize);
        in.read(reinterpret_cast<char*>(font.Palette.data()), paletteSize);

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

        // Palette size after last MipMap level
        uint16_t paletteSize;
        in.read(reinterpret_cast<char*>(&paletteSize), sizeof(paletteSize));
        if(paletteSize == 0)
            throw std::runtime_error("Empty Palette");
        if(paletteSize > 256)
            throw std::runtime_error("Palette size too big");

        // Palette
        texture.Palette.resize(paletteSize);
        in.read(reinterpret_cast<char*>(texture.Palette.data()), sizeof(glm::u8vec3) * paletteSize);

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

        std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec4* rgba)> writeFunc = ImageWriteFunction_RGBA(extension);

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

    WadFile::Item WadFile::Texture::AsItem() const
    {
        Item item = {};
        item.Name = Name;
        item.Type = ItemType::Texture;

        item.Size = sizeof(Width) + sizeof(Height) +
                    sizeof(uint8_t) * (MipMapData[0].size() + MipMapData[1].size() + MipMapData[2].size() + MipMapData[3].size()) +
                    sizeof(uint16_t) + sizeof(glm::u8vec3) * Palette.size();
        item.Data = malloc(item.Size);

        MemoryBuffer itemDataBuffer(
                reinterpret_cast<char*>(item.Data),
                item.Size
        );
        std::ostream out(&itemDataBuffer);

        out.write(reinterpret_cast<const char*>(&Width), sizeof(Width));
        out.write(reinterpret_cast<const char*>(&Height), sizeof(Height));

        out.write(reinterpret_cast<const char*>(MipMapData[0].data()), sizeof(uint8_t) * MipMapData[0].size());
        out.write(reinterpret_cast<const char*>(MipMapData[1].data()), sizeof(uint8_t) * MipMapData[1].size());
        out.write(reinterpret_cast<const char*>(MipMapData[2].data()), sizeof(uint8_t) * MipMapData[2].size());
        out.write(reinterpret_cast<const char*>(MipMapData[3].data()), sizeof(uint8_t) * MipMapData[3].size());

        short paletteSize = Palette.size();
        out.write(reinterpret_cast<const char*>(&paletteSize), sizeof(paletteSize));
        out.write(reinterpret_cast<const char*>(Palette.data()), sizeof(glm::u8vec3) * Palette.size());

        return item;
    }

    WadFile::Item WadFile::Font::AsItem(std::string name) const
    {
        Item item = {};
        item.Name = std::move(name);
        item.Type = ItemType::Font;

        item.Size = sizeof(Width) + sizeof(Height) +
                    sizeof(FontChar) * CharacterCount +
                    sizeof(uint8_t) * Data.size() +
                    sizeof(uint16_t) + sizeof(glm::u8vec3) * Palette.size();
        item.Data = malloc(item.Size);

        MemoryBuffer itemDataBuffer(
                reinterpret_cast<char*>(item.Data),
                item.Size
        );
        std::ostream out(&itemDataBuffer);

        out.write(reinterpret_cast<const char*>(&Width), sizeof(Width));
        out.write(reinterpret_cast<const char*>(&Height), sizeof(Height));

        out.write(reinterpret_cast<const char*>(Characters), sizeof(FontChar) * CharacterCount);

        out.write(reinterpret_cast<const char*>(Data.data()), Data.size());

        short paletteSize = Palette.size();
        out.write(reinterpret_cast<const char*>(&paletteSize), sizeof(paletteSize));
        out.write(reinterpret_cast<const char*>(Palette.data()), sizeof(glm::u8vec3) * Palette.size());

        return item;
    }
}
