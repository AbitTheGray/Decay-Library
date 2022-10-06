#define GLM_FORCE_SWIZZLE
#include "WadFile.hpp"

#include <iostream>

#include <glm/gtx/color_space.hpp>

namespace Decay::Wad::Wad3
{
    std::vector<WadFile::EntryHeader> WadFile::ReadWadEntries(std::istream& stream)
    {
        // Magic1 Number
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

        /// Offset in file to EntryHeader structure
        uint32_t entriesOffset;
        stream.read(reinterpret_cast<char*>(&entriesOffset), sizeof(entriesOffset));
        R_ASSERT(entriesOffset >= ((sizeof(uint32_t) * 2) + (sizeof(char) * 4)), "Offset of entry inside WAD file starts too soon");

        stream.seekg(entriesOffset);

        std::vector<EntryHeader> entries(entriesCount);
        stream.read(reinterpret_cast<char*>(entries.data()), sizeof(EntryHeader) * entriesCount);

        return entries;
    }

    WadFile::WadFile(std::istream& in)
    {
        auto startOffset = in.tellg();

        std::vector<EntryHeader> entries = ReadWadEntries(in);
        if(entries.empty())
            return;

        m_Items.resize(entries.size());

        for(const EntryHeader& entry : entries)
        {
            in.seekg((int64_t)startOffset + entry.Offset, std::ios_base::beg);

            std::size_t dataLength = entry.DiskSize;
            std::vector<uint8_t> data(dataLength);
            in.read(reinterpret_cast<char*>(data.data()), data.size());

            if(entry.Compression)
            {
                throw std::runtime_error("WAD entry compression is not supported");

                std::size_t dataRawLength = entry.Size;

                //TODO Uncompress `dataRaw` into `data`
                // Not implemented inside GoldSrc
                // but it seems they decided on LZSS (Lempel–Ziv–Storer–Szymanski, https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski)
            }

            m_Items.emplace_back(entry.Name_str(), entry.Type, std::move(data));
        }
    }

#pragma region Image
    WadFile::Image::Image(std::istream& in)
    {
        in.read(reinterpret_cast<char*>(&Width), sizeof(Width));
        in.read(reinterpret_cast<char*>(&Height), sizeof(Height));
        R_ASSERT(Width > 0, "Invalid image width: " << (int)Width)
        R_ASSERT(Height > 0, "Invalid image height: " << (int)Height);

        // Calculate data length
        std::size_t dataLength = static_cast<std::size_t>(Width) * Height;
        R_ASSERT(dataLength > 0, "Invalid image data pixel count: " << (int)dataLength);
        R_ASSERT(dataLength == Width * Height, "Invalid image data pixel count: " << (int)dataLength << " (expected " << (Width * Height) << ")");

        // Read data
        Data.resize(dataLength);
        in.read(reinterpret_cast<char*>(Data.data()), dataLength);

        // Read palette length
        uint16_t paletteSize;
        in.read(reinterpret_cast<char*>(&paletteSize), sizeof(paletteSize));
        if(paletteSize == 0)
            throw std::runtime_error("Empty Palette");
        if(paletteSize > 256)
            throw std::runtime_error("Palette size too big");

        // Read palette
        Palette.resize(paletteSize);
        in.read(reinterpret_cast<char*>(Palette.data()), sizeof(glm::u8vec3) * paletteSize);
    }
    inline decltype(WadFile::Image::Palette) CreateRainbowPalette()
    {
        decltype(WadFile::Image::Palette) palette{};
        palette.resize(256);

        for(int i = 0; i < palette.size(); i++)
        {
            if(i < 32) // Grayscale
            {
                // 0 <= i < 32
                palette[i] = { i * 8, i * 8, i * 8 };
            }
            else if(i < 64) // Low-lightness
            {
                // 32 <= i < 64
                double hue = (i - 32) % 32;
                // ((i - 64) / 64) = 0 to 3
                double saturation = 64 + ((i - 32) / 32.0f) * 48;
                double lightness = 32;
                glm::dvec3 hsv = { hue, saturation, lightness };
                glm::dvec3 rgb = glm::rgbColor(hsv);

                palette[i] = { rgb.r, rgb.g, rgb.b };
            }
            else // i >= 64
            {
                // 64 <= i < 256
                double hue = ((i - 64) % 64) * (360 / 3); // 0, 1, 2
                // (i - 64) = 0 to 191
                // (i - 64) / 3 = 0 to 63
                double saturation = 128 + ((i - 64) / 3) * 2;
                // (i - 64) = 0 to 191
                // (i - 64) / 64 = 0 to 3
                double lightness = 64 + ((i - 64) / 64);
                glm::dvec3 hsv = { hue, saturation, lightness };
                glm::dvec3 rgb = glm::rgbColor(hsv);

                palette[i] = { rgb.r, rgb.g, rgb.b };
            }

        }

        return palette;
    }
    const decltype(WadFile::Image::Palette) WadFile::Image::RainbowPalette = CreateRainbowPalette();
    WadFile::Image::Image(uint32_t width, uint32_t height, std::vector<glm::u8vec4> data)
      : Width(width), Height(height)
    {
        R_ASSERT(width > 0, "Invalid image width");
        R_ASSERT(height > 0, "Invalid image height");
        R_ASSERT(width * height == data.size(), "Number of provided pixels does not match dimensions of the image");
        Data.resize(data.size());

        bool transparent = false;
        for(std::size_t i = 0; i < width * height; i++)
        {
            glm::u8vec3 rgb = data[i].rgb();
            if(data[i].a == 0xFFu)
            {
                transparent = true;
                Data[i] = 255;
                continue;
            }

            auto paletteIterator = std::find(Palette.begin(), Palette.end(), rgb);
            if(paletteIterator == Palette.end())
            { // Not found
                if(Palette.size() == (transparent ? 255 : 256))
                    throw std::runtime_error("Exceeded palette size");

                Data[i] = Palette.size();

                Palette.emplace_back(rgb);
            }
            else
                Data[i] = paletteIterator - Palette.begin();
        }

        if(transparent)
        {
            if(Palette.size() < 256)
                Palette.resize(256, {0x00u, 0x00u, 0x00u});

            Palette[255] = {0x00u, 0x00u, 0xFFu};
        }
    }
    WadFile::Item WadFile::Image::AsItem(std::string name) const
    {
        Item item(std::move(name), ItemType::Image);

        item.Data.resize(
            sizeof(Width) + sizeof(Height) + // Dimensions
            sizeof(uint8_t) * Data.size() + // Image data
            sizeof(uint16_t) + sizeof(glm::u8vec3) * Palette.size() // Palette
        );

        MemoryBuffer itemDataBuffer(
            reinterpret_cast<char*>(item.Data.data()),
            item.Data.size()
        );
        std::ostream out(&itemDataBuffer);

        // Dimensions
        out.write(reinterpret_cast<const char*>(&Width), sizeof(Width));
        out.write(reinterpret_cast<const char*>(&Height), sizeof(Height));

        // Image data
        R_ASSERT(Width * Height == Data.size(), "Pixel data does not match image's pixel count");
        out.write(reinterpret_cast<const char*>(Data.data()), Data.size());

        // Palette
        uint16_t paletteSize = Palette.size();
        if(paletteSize == 0)
            throw std::runtime_error("Empty Palette");
        if(paletteSize > 256)
            throw std::runtime_error("Palette size too big");
        out.write(reinterpret_cast<const char*>(&paletteSize), sizeof(paletteSize));
        out.write(reinterpret_cast<const char*>(Palette.data()), sizeof(glm::u8vec3) * Palette.size());

        out.flush();
        return item;
    }
#pragma endregion

#pragma region Texture
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

    WadFile::Texture::Texture(std::string name, uint32_t width, uint32_t height, std::vector<glm::u8vec4> data)
        : Name(std::move(name)), Width(width), Height(height)
    {
        R_ASSERT(width > 0, "Invalid image width");
        R_ASSERT(height > 0, "Invalid image height");
        R_ASSERT(width * height == data.size(), "Number of provided pixels does not match dimensions of the image");
        MipMapData[0].resize(data.size());

        bool transparent = false;
        for(std::size_t i = 0; i < width * height; i++)
        {
            if(data[i].a == 0x00u)
            {
                transparent = true;
                MipMapData[0][i] = 255;
                continue;
            }

            glm::u8vec3 rgb = data[i].rgb();

            auto paletteIterator = std::find(Palette.begin(), Palette.end(), rgb);
            if(paletteIterator == Palette.end())
            { // Not found
                if(Palette.size() == (transparent ? 255 : 256))
                    throw std::runtime_error("Exceeded palette size");

                MipMapData[0][i] = Palette.size();

                Palette.emplace_back(rgb);
            }
            else
                MipMapData[0][i] = paletteIterator - Palette.begin();
        }

        // Generate Mip-Maps
        {
            MipMapDimensions[0] = glm::u32vec2(Width, Height);
            MipMapDimensions[1] = glm::u32vec2(Width / 2u, Height / 2u);
            MipMapDimensions[2] = glm::u32vec2(Width / 4u, Height / 4u);
            MipMapDimensions[3] = glm::u32vec2(Width / 8u, Height / 8u);

            MipMapData[1] = GenerateMipMap(MipMapDimensions[0], MipMapData[0]);
            MipMapData[2] = GenerateMipMap(MipMapDimensions[1], MipMapData[1]);
            MipMapData[3] = GenerateMipMap(MipMapDimensions[2], MipMapData[2]);
        }

        if(transparent)
        {
            Palette.resize(256, {0x00u, 0x00u, 0x00u});

            Palette[255] = {0x00u, 0x00u, 0xFFu};
        }
    }
    WadFile::Texture::Texture(std::istream& in)
    {
        // Name
        {
            char name[Texture::MaxNameLength];
            in.read(name, Texture::MaxNameLength);
            Name = Cstr2Str(name, Texture::MaxNameLength);
            R_ASSERT(!Name.empty(), "Texture name cannot be empty");
        }

        // Dimensions
        in.read(reinterpret_cast<char*>(&Width), sizeof(Texture::Width));
        in.read(reinterpret_cast<char*>(&Height), sizeof(Texture::Height));
        R_ASSERT(Width >= (1u << Texture::MipMapLevels), "Texture width is too small to fit " << Texture::MipMapLevels << " mip-map levels");
        R_ASSERT(Height >= (1u << Texture::MipMapLevels), "Texture height is too small to fit " << Texture::MipMapLevels << " mip-map levels");
        if(!IsMultipleOf2(Width))
            R_ASSERT(Width % 16 == 0, "Texture width must be divisible by 16 (to allow mip-maps to work correctly)");
        if(!IsMultipleOf2(Height))
            R_ASSERT(Height % 16 == 0, "Texture height must be divisible by 16 (to allow mip-maps to work correctly)");

        // Offsets
        uint32_t mipMapOffsets[Texture::MipMapLevels];
        in.read(reinterpret_cast<char*>(mipMapOffsets), sizeof(uint32_t) * Texture::MipMapLevels);

        for(std::size_t level = 0; level < Texture::MipMapLevels; level++)
        {
            in.seekg(mipMapOffsets[level]);

            MipMapDimensions[level] = {
                Width >> level,
                Height >> level
            };
            std::size_t dataLength = static_cast<std::size_t>(MipMapDimensions[level].x) * MipMapDimensions[level].y;

            std::vector<uint8_t>& data = MipMapData[level];
            data.resize(dataLength);
            in.read(reinterpret_cast<char*>(data.data()), dataLength);
        }
        R_ASSERT(Width == MipMapDimensions[0].x, "MipMap 0 does not match texture width");
        R_ASSERT(Height == MipMapDimensions[0].y, "MipMap 0 does not match texture height");

        // Palette size after last MipMap level
        uint16_t paletteSize;
        in.read(reinterpret_cast<char*>(&paletteSize), sizeof(paletteSize));
        if(paletteSize == 0)
            throw std::runtime_error("Empty Palette");
        if(paletteSize > 256)
            throw std::runtime_error("Palette size too big");

        // Palette
        Palette.resize(paletteSize);
        in.read(reinterpret_cast<char*>(Palette.data()), sizeof(glm::u8vec3) * paletteSize);
    }
    WadFile::Item WadFile::Texture::AsItem() const
    {
        if(!HasData())
            throw std::runtime_error("Texture without data cannot be converted into WadFile::Item");

        Item item = {};

        R_ASSERT(!Name.empty(), "Texture name is empty -> cannot create WAD Item from it");
        item.Name = Name;

        item.Type = ItemType::Texture;

        item.Data.resize(
            sizeof(char) * MaxNameLength + // Name
            sizeof(Width) + sizeof(Height) + // Dimensions
            sizeof(uint32_t) * Texture::MipMapLevels + // MipMap memory offsets
            sizeof(uint8_t) * (MipMapData[0].size() + MipMapData[1].size() + MipMapData[2].size() + MipMapData[3].size()) + // MipMap data
            sizeof(uint16_t) + sizeof(glm::u8vec3) * Palette.size() + 1 // Palette
        );

        MemoryBuffer itemDataBuffer(
            reinterpret_cast<char*>(item.Data.data()),
            item.Data.size()
        );
        {
            std::ostream out(&itemDataBuffer);

            // Name
            {
                R_ASSERT(!item.Name.empty(), "WAD item name cannot be empty");
                R_ASSERT(item.Name.size() < MaxNameLength, "WAD item name is too long");
                out.write(reinterpret_cast<const char*>(item.Name.c_str()), sizeof(char) * item.Name.size());

                uint8_t nullByte = '\0';
                for(int i = item.Name.size(); i < MaxNameLength; i++)
                    out.write(reinterpret_cast<const char*>(&nullByte), sizeof(char));
            }

            // Size
            R_ASSERT(Width > 0, "Invalid width value");
            out.write(reinterpret_cast<const char*>(&Width), sizeof(Width));
            R_ASSERT(Height > 0, "Invalid height value");
            out.write(reinterpret_cast<const char*>(&Height), sizeof(Height));

            // Data offsets
            {
                uint32_t offset = sizeof(char) * MaxNameLength +
                                  sizeof(Width) + sizeof(Height) +
                                  sizeof(uint32_t) * Texture::MipMapLevels;
                for(std::size_t i = 0; i < MipMapLevels; i++)
                {
                    auto& dim = MipMapDimensions[i];
                    out.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
                    offset += dim.x * dim.y;
                }
            }

            // Data
            {
                for(std::size_t i = 0; i < MipMapLevels; i++)
                    out.write(reinterpret_cast<const char*>(MipMapData[i].data()), sizeof(uint8_t) * MipMapData[i].size());
            }

            // Palette
            {
                uint16_t paletteSize = Palette.size();
                R_ASSERT(paletteSize > 0, "Invalid palette size - <= 0 is not valid");
                R_ASSERT(paletteSize <= 256, "Invalid palette size - > 256 is not valid");
                out.write(reinterpret_cast<const char*>(&paletteSize), sizeof(paletteSize));
                out.write(reinterpret_cast<const char*>(Palette.data()), sizeof(glm::u8vec3) * Palette.size());
            }

            // Dummy byte
            {
                uint8_t nullByte = '\0';
                for(int i = item.Name.size(); i < MaxNameLength; i++)
                    out.write(reinterpret_cast<const char*>(&nullByte), sizeof(char));
            }

            out.flush();
        }

#ifdef DEBUG
        // Name (1st char)
        D_ASSERT(((const uint8_t*)item.Data.data())[0] != '\0', "Item name cannot start with NULL character");

        // Width
        D_ASSERT(
            ((const uint8_t*)item.Data.data())[MaxNameLength + 0] != 0 ||
            ((const uint8_t*)item.Data.data())[MaxNameLength + 1] != 0 ||
            ((const uint8_t*)item.Data.data())[MaxNameLength + 2] != 0 ||
            ((const uint8_t*)item.Data.data())[MaxNameLength + 3] != 0,
            "Width cannot be zero"
        );

        // Height
        D_ASSERT(
            ((const uint8_t*)item.Data.data())[MaxNameLength + 4 + 0] != 0 ||
            ((const uint8_t*)item.Data.data())[MaxNameLength + 4 + 1] != 0 ||
            ((const uint8_t*)item.Data.data())[MaxNameLength + 4 + 2] != 0 ||
            ((const uint8_t*)item.Data.data())[MaxNameLength + 4 + 3] != 0,
            "Height cannot be zero"
        );

        // Offsets (1st mipmap offset)
        D_ASSERT(
            ((const uint8_t*) item.Data.data())[
                MaxNameLength +
                sizeof(uint32_t) * 2
            ] ==
            MaxNameLength +
            sizeof(uint32_t) * 2 +
            sizeof(uint32_t) * MipMapLevels,
            "Offset of 1st mip-map is incorrect"
        );

        // First data index into palette
        D_ASSERT(
            ((const uint8_t*)item.Data.data())[
                MaxNameLength +
                sizeof(uint32_t) * 2 +
                sizeof(uint32_t) * MipMapLevels
            ] == MipMapData[0][0],
            "Offset of Palette is incorrect"
        );

        // First pixel of palette
        D_ASSERT(
            ((const uint8_t*)item.Data.data())[
                MaxNameLength +
                sizeof(uint32_t) * 2 +
                sizeof(uint32_t) * MipMapLevels +
                MipMapData[0].size() + MipMapData[1].size() + MipMapData[2].size() + MipMapData[3].size() +
                sizeof(uint16_t)
            ] == Palette[0].x,
            "Checking Red component of 1st palette color failed"
        );
#endif

        return item;
    }
#pragma endregion

#pragma region Font
    WadFile::Font::Font(std::istream& in)
    {
        // Dimensions
        {
            glm::u32vec4 dimensions; // width, height, row count + height
            in.read(reinterpret_cast<char*>(&dimensions), sizeof(dimensions));
            Width = dimensions.x;
            Height = dimensions.y;
            RowCount = dimensions.z;
            RowHeight = dimensions.w;
        }
        Width = 256;// Fonts have Width=256 ( https://developer.valvesoftware.com/wiki/WAD ) but 2 tested fonts had different values there
        R_ASSERT(RowCount > 0, "Invalid number of font rows");
        R_ASSERT(RowHeight > 0, "Invalid font row height");
        R_ASSERT(Height >= RowCount * RowHeight, "Font image won't fit " << RowCount << " rows (of height " << RowHeight << ")");

        // Character offsets
        in.read(reinterpret_cast<char*>(&Characters), sizeof(FontChar) * Font::CharacterCount);

        // Calculate data length
        std::size_t dataLength = static_cast<std::size_t>(Width) * Height;
        R_ASSERT(dataLength > 0, "Invalid data length of font image");

        // Read data
        Data.resize(dataLength);
        in.read(reinterpret_cast<char*>(Data.data()), dataLength);

        // Read palette length
        uint16_t paletteSize;
        in.read(reinterpret_cast<char*>(&paletteSize), sizeof(paletteSize));
        if(paletteSize == 0)
            throw std::runtime_error("Empty Palette");
        if(paletteSize > 256)
        {
            std::cerr<< "Font palette size too big" << std::endl;
            paletteSize = 256;
        }

        // Read palette
        Palette.resize(paletteSize);
        in.read(reinterpret_cast<char*>(Palette.data()), sizeof(glm::u8vec3) * paletteSize);
    }
    WadFile::Item WadFile::Font::AsItem(std::string name) const
    {
        Item item = {};
        item.Name = std::move(name);
        item.Type = ItemType::Font;

        item.Data.resize(
            sizeof(Width) + sizeof(Height) +
            sizeof(FontChar) * CharacterCount +
            sizeof(uint8_t) * Data.size() +
            sizeof(uint16_t) + sizeof(glm::u8vec3) * Palette.size()
        );

        MemoryBuffer itemDataBuffer(
            reinterpret_cast<char*>(item.Data.data()),
            item.Data.size()
        );
        std::ostream out(&itemDataBuffer);

        out.write(reinterpret_cast<const char*>(&Width), sizeof(Width));
        out.write(reinterpret_cast<const char*>(&Height), sizeof(Height));

        out.write(reinterpret_cast<const char*>(Characters), sizeof(FontChar) * CharacterCount);

        out.write(reinterpret_cast<const char*>(Data.data()), Data.size());

        uint16_t paletteSize = Palette.size();
        out.write(reinterpret_cast<const char*>(&paletteSize), sizeof(paletteSize));
        out.write(reinterpret_cast<const char*>(Palette.data()), sizeof(glm::u8vec3) * Palette.size());

        out.flush();
        return item;
    }
    void WadFile::Font::WriteCharacterPngs(const std::filesystem::path& dir) const
    {
        std::filesystem::create_directories(dir);
        for(int charIndex = 0; charIndex < WadFile::Font::CharacterCount; charIndex++)
        {
            const auto& fc = Characters[charIndex];
            if(fc.Width == 0)
                continue;
            R_ASSERT(fc.Width > 0, "Invalid font character width");

            WadFile::Image fc_img = WadFile::Image();
            fc_img.Width = fc.Width;
            fc_img.Height = RowHeight;
            fc_img.Palette = Palette;
            fc_img.Data.reserve(fc_img.Width * fc_img.Height);

            int startX = fc.Offset % Width;
            int startY = fc.Offset / Width;
            for(int sy = 0; sy < fc_img.Height; sy++)
            {
                int i = (startY + sy) * Width + startX;

                for(int sx = 0; sx < fc_img.Width; sx++)
                {
                    fc_img.Data.emplace_back(Data[i + sx]);
                }
            }
            R_ASSERT(fc_img.Data.size() == fc_img.Width * fc_img.Height, "Number of saved pixels does not match number of image pixels");

            fc_img.WriteRgbaPng(dir / (std::to_string(charIndex) + ".png"));
        }
    }
#pragma endregion

    void WadFile::ExportTextures(const std::filesystem::path& directory, const std::string& extension) const
    {
        if(!std::filesystem::exists(directory))
            std::filesystem::create_directory(directory);

        R_ASSERT(extension.size() > 1, "Invalid texture extension");
        R_ASSERT(extension[0] == '.', "Invaldi texture extension - must start with '.' character");

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
    std::ostream& operator<<(std::ostream& out, const WadFile& wad)
    {
        std::vector<WadFile::EntryHeader> entries{};
        std::vector<const uint8_t*> entryData{};

        entries.reserve(wad.m_Items.size());
        entryData.reserve(wad.m_Items.size());

        // Add items to entries
        {
            for(const WadFile::Item& item : wad.m_Items)
            {
                WadFile::EntryHeader entry = {};
                entry.Compression = false;
                entry.Type = item.Type;

                entry.DiskSize = item.Data.size();
                entry.Size = entry.DiskSize;

                // Copy name
                R_ASSERT(item.Name.length() <= 15, "WAD Entry name is too long");
                std::copy(item.Name.c_str(), item.Name.c_str() + item.Name.size(), entry.Name);
                std::fill(entry.Name + item.Name.size(), entry.Name + 16, '\0');

                entries.emplace_back(entry);
                entryData.emplace_back(item.Data.data());
            }
        }

        int64_t startOffset = out.tellp();

        // Write original magic number
        out.write(WadFile::Magic, sizeof(WadFile::Magic));

        // Write item count
        {
            uint32_t itemCount = entries.size();
            out.write(reinterpret_cast<const char*>(&itemCount), sizeof(itemCount));
        }

        // Write entry offset
        uint32_t entriesOffset = sizeof(WadFile::Magic) + sizeof(uint32_t) + sizeof(uint32_t); // magic + itemCount + entriesOffset
        out.write(reinterpret_cast<const char*>(&entriesOffset), sizeof(entriesOffset));

        // Write entries
        out.write(reinterpret_cast<const char*>(entries.data()), sizeof(WadFile::EntryHeader) * entries.size());

        // Write entry data
        std::vector<typeof(WadFile::EntryHeader::Offset)> offsets(entries.size());
        for(int i = 0; i < entries.size(); i++)
        {
            offsets[i] = out.tellp() - startOffset;

            if(entries[i].DiskSize != 0)
            {
                R_ASSERT(entryData[i] != nullptr, "WAD Entry has size (of data) but no data (NULL pointer)");
                out.write(reinterpret_cast<const char*>(entryData[i]), entries[i].DiskSize);
            }
        }

        // Override Offset in Entries
        for(int i = 0; i < entries.size(); i++)
        {
            out.seekp(startOffset + entriesOffset + sizeof(WadFile::EntryHeader) * i + offsetof(WadFile::EntryHeader, Offset), std::ios_base::beg);

            out.write(reinterpret_cast<const char*>(&offsets[i]), sizeof(WadFile::EntryHeader::Offset));
        }

        return out;
    }
}
