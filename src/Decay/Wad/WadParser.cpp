#include "WadParser.hpp"

#include <fstream>
#include <iostream>

#include <stb_image_write.h>

namespace Decay::Wad
{
    WadParser::WadParser(const std::filesystem::path& filename)
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

            [[nodiscard]] std::string GetName() const
            {
                for(std::size_t i = 0; i < EntryNameLength; i++)
                    if(Name[i] == '\0')
                        return std::string(Name, i);
                return std::string();
            }
        };

        std::vector<Entry> entries(entriesCount);
        file.read(reinterpret_cast<char*>(entries.data()), sizeof(Entry) * entriesCount);

        for(const Entry& entry : entries)
        {
            file.seekg(entry.Offset);
            std::size_t dataLength = entry.DiskSize;
            void* data = std::malloc(dataLength);

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

    WadParser::~WadParser()
    {
        for(Item& item : m_Items)
        {
            std::free(item.Data);
            item.Data = nullptr;
        }
    }

    WadParser::Image WadParser::ReadImage(const WadParser::Item& item)
    {
        struct memoryBuffer : std::streambuf
        {
            memoryBuffer(char* begin, char* end)
            {
                this->setg(begin, begin, end);
            }
        };
        memoryBuffer itemDataBuffer(
                reinterpret_cast<char*>(item.Data),
                reinterpret_cast<char*>(item.Data) + item.Size
        );
        std::istream in(&itemDataBuffer);

        Image image = {};
        in.read(reinterpret_cast<char*>(&image.Width), sizeof(image.Width));
        in.read(reinterpret_cast<char*>(&image.Height), sizeof(image.Height));

        //FIXME image.Width and image.Height are of invalid size
        throw std::runtime_error("Function must be fixed before use");

        // Calculate data length
        std::size_t dataLength = static_cast<std::size_t>(image.Width) * image.Height;

        // Read data
        image.Data.resize(dataLength);
        in.read(reinterpret_cast<char*>(image.Data.data()), dataLength);

        // Read palette length
        uint16_t paletteLength;
        in.read(reinterpret_cast<char*>(&paletteLength), sizeof(paletteLength));

        // Read palette
        image.Palette.resize(paletteLength);
        in.read(reinterpret_cast<char*>(image.Palette.data()), paletteLength);

        return image;
    }

    void WadParser::Image::WriteRgbPng(const std::filesystem::path& filename) const
    {
        auto pixels = AsRgb();
        assert(pixels.size() == Width * Height);
        stbi_write_png(filename.c_str(), Width, Height, 3, pixels.data(), sizeof(glm::i8vec3));
    }
    void WadParser::Image::WriteRgbaPng(const std::filesystem::path& filename) const
    {
        auto pixels = AsRgba();
        assert(pixels.size() == Width * Height);
        stbi_write_png(filename.c_str(), Width, Height, 3, pixels.data(), sizeof(glm::i8vec4));
    }
}
