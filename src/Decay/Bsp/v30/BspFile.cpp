#include "BspFile.hpp"

// Use to test images if you are getting weird results.
// Will replace palette for all images with HSL/HSV noise.
// Equivalent to WAD_PALETTE_DUMMY
//#define BSP_PALETTE_DUMMY 1

#include <fstream>
#include <iostream>

#include <stb_image_write.h>

namespace Decay::Bsp::v30
{
    std::array<std::size_t, BspFile::LumpType_Size> BspFile::s_DataMaxLength = {
        MaxEntities,
        MaxPlanes,
        MaxTextures,
        MaxVertices,
        MaxVisibility,
        MaxNodes,
        MaxTextureMapping,
        MaxFaces,
        MaxLighting,
        MaxClipNodes,
        MaxLeaves,
        MaxMarkSurfaces,
        MaxEdges,
        MaxSurfaceEdges,
        MaxModels,
    };
    std::array<std::size_t, BspFile::LumpType_Size> BspFile::s_DataElementSize = {
        0, // sizeof(char), Not so simple
        sizeof(Plane),
        0, // sizeof(Texture), Not so simple
        sizeof(glm::vec3),
        0, // Visibility
        sizeof(Node),
        sizeof(TextureMapping),
        sizeof(Face),
        0, // Lighting
        sizeof(ClipNode),
        sizeof(Leaf),
        sizeof(MarkSurface),
        sizeof(Edge),
        sizeof(SurfaceEdges),
        sizeof(Model),
    };

    BspFile::BspFile(const std::filesystem::path& filename)
    {
        if(!std::filesystem::exists(filename))
            throw std::runtime_error("File not found");

        std::ifstream in(filename, std::ios_base::binary | std::ios_base::in);

        // Magic Number
        {
            uint32_t magicNumber;
            in.read(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber));

            switch(magicNumber)
            {
                case Magic:
                    break; // OK
                case Magic_WrongEndian:
                    throw std::runtime_error("Invalid endianness");
                default:
                    throw std::runtime_error("Unsupported magic number");
            }
        }

        struct LumpEntry
        {
            uint32_t Offset;
            uint32_t Length;
        };

        LumpEntry lumps[LumpType_Size];
        in.read(reinterpret_cast<char*>(lumps), sizeof(lumps));

        for(std::size_t i = 0; i < LumpType_Size; i++)
        {
            in.seekg(lumps[i].Offset);

            void* d = std::malloc(lumps[i].Length);
            in.read(reinterpret_cast<char*>(d), lumps[i].Length);

            m_Data[i] = d;
            m_DataLength[i] = lumps[i].Length;
        }

        in.close();

        // Tests
        {
            for(std::size_t i = 0; i < LumpType_Size; i++)
            {
                if(s_DataElementSize[i] != 0)
                {
#ifdef BSP_DEBUG
                    std::cout << i << ": " << m_DataLength[i] << " < " << (s_DataMaxLength[i] * s_DataElementSize[i]) << " (" << s_DataMaxLength[i] << " * " << s_DataElementSize[i] << ")" << std::endl;
#endif
                    assert(m_DataLength[i] < s_DataMaxLength[i] * s_DataElementSize[i]);
                }
            }

            // Entities
            {
                //TODO
            }

            // Planes
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Planes)] % sizeof(Plane) == 0);
            }

            // Textures
            {
                //TODO
            }

            // Vertices
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Vertices)] % sizeof(glm::vec3) == 0);
            }

            // Visibility
            {
                //TODO
            }

            // Nodes
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Nodes)] % sizeof(Node) == 0);
            }

            // Texture Mapping
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::TextureMapping)] % sizeof(TextureMapping) == 0);
            }

            // Faces
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Faces)] % sizeof(Face) == 0);
            }

            // Lighting
            {
                //TODO
            }

            // Clip Nodes
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::ClipNodes)] % sizeof(ClipNode) == 0);
            }

            // Leaves
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Leaves)] % sizeof(Leaf) == 0);
            }

            // Mark Surface
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::MarkSurface)] % sizeof(MarkSurface) == 0);
            }

            // Edges
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Edges)] % sizeof(Edge) == 0);
            }

            // Surface Edges
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::SurfaceEdges)] % sizeof(SurfaceEdges) == 0);
            }

            // Models
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Models)] % sizeof(Model) == 0);
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Models)] >= sizeof(Model)); // There must be at least 1 model
            }
        }
    }

    BspFile::~BspFile()
    {
        for(std::size_t i = 0; i < LumpType_Size; i++)
            std::free(m_Data[i]);
    }

    uint32_t BspFile::GetTextureCount() const
    {
        MemoryBuffer itemDataBuffer(
            reinterpret_cast<char*>(m_Data[static_cast<uint8_t>(LumpType::Textures)]),
            m_DataLength[static_cast<uint8_t>(LumpType::Textures)]
        );
        std::istream in(&itemDataBuffer);

        uint32_t count;
        in.read(reinterpret_cast<char*>(&count), sizeof(count));

        return count;
    }

    std::vector<Wad::Wad3::WadFile::Texture> BspFile::GetTextures() const
    {
        MemoryBuffer itemDataBuffer(
            reinterpret_cast<char*>(m_Data[static_cast<uint8_t>(LumpType::Textures)]),
            m_DataLength[static_cast<uint8_t>(LumpType::Textures)]
        );
        std::istream in(&itemDataBuffer);

        uint32_t count;
        in.read(reinterpret_cast<char*>(&count), sizeof(count));
        assert(count < MaxTextures);

        std::vector<uint32_t> offsets(count);
        in.read(reinterpret_cast<char*>(offsets.data()), sizeof(uint32_t) * count);

        std::vector<Wad::Wad3::WadFile::Texture> textures(count);
        for(std::size_t i = 0; i < count; i++)
        {
            assert(offsets[i] >= sizeof(uint32_t) + sizeof(uint32_t) * count);
            assert(offsets[i] < m_DataLength[static_cast<uint8_t>(LumpType::Textures)]);
            in.seekg(offsets[i]);

            Texture texture = {};
            in.read(reinterpret_cast<char*>(&texture), sizeof(Texture));

            Wad::Wad3::WadFile::Texture wadTexture = {
                texture.GetName(),
                texture.Width,
                texture.Height
            };
            assert(!wadTexture.Name.empty());

            if(texture.IsPacked())
            {
                // MipMap data
                for(std::size_t level = 0; level < MipTextureLevels; level++)
                {
                    wadTexture.MipMapDimensions[level] = {
                            texture.Width >> level,
                            texture.Height >> level
                    };
                    std::size_t dataLength = static_cast<std::size_t>(wadTexture.MipMapDimensions[level].x) * wadTexture.MipMapDimensions[level].y;

                    std::vector<uint8_t>& data = wadTexture.MipMapData[level];
                    data.resize(dataLength);
                    in.read(reinterpret_cast<char*>(data.data()), dataLength);
                }
                assert(texture.Width == wadTexture.MipMapDimensions[0].x);
                assert(texture.Height == wadTexture.MipMapDimensions[0].y);

                // Palette size after last MipMap level
                uint16_t paletteSize;
                in.read(reinterpret_cast<char*>(&paletteSize), sizeof(paletteSize));
                if(paletteSize == 0)
                    throw std::runtime_error("Empty Palette");
                if(paletteSize > 256)
                    throw std::runtime_error("Palette size too big");

                // Palette
                wadTexture.Palette.resize(paletteSize);
                in.read(reinterpret_cast<char*>(wadTexture.Palette.data()), sizeof(glm::u8vec3) * paletteSize);

#ifdef BSP_PALETTE_DUMMY
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
            }

            textures[i] = std::move(wadTexture);
        }

        return textures;
    }

    void BspFile::SetTextures(const std::vector<Wad::Wad3::WadFile::Texture>& textures)
    {
        // Free old
        std::free(m_Data[static_cast<uint8_t>(LumpType::Textures)]);

        // No textures
        if(textures.empty())
        {
            uint32_t* data = reinterpret_cast<uint32_t*>(std::malloc(sizeof(uint32_t)));
            *data = 0; // textures.size()
            m_Data[static_cast<uint8_t>(LumpType::Textures)] = data;

            m_DataLength[static_cast<uint8_t>(LumpType::Textures)] = sizeof(uint32_t);
            return;
        }

        std::size_t dataLength = sizeof(Texture) * textures.size();
        for(const auto& texture : textures)
        {
            if(texture.HasData())
            {
                for(std::size_t i = 0; i < MipTextureLevels; i++)
                    dataLength += texture.MipMapDimensions[i].x * texture.MipMapDimensions[i].y;
                dataLength += sizeof(short);
                dataLength += texture.Palette.size() * 3;
            }
        }

        // Allocate memory
        void* data = std::malloc(dataLength);
        m_Data[static_cast<uint8_t>(LumpType::Textures)] = data;

        m_DataLength[static_cast<uint8_t>(LumpType::Textures)] = dataLength;

        MemoryBuffer dataBuffer = MemoryBuffer(
                data,
                dataLength
        );
        std::ostream out(&dataBuffer);


        // Texture Insert Loop
        for(const auto& texture : textures)
        {
            Texture t = {};

            // Name
            {
                assert(texture.Name.length() < MaxTextureName);
                std::fill(t.Name, t.Name + MaxTextureName, '\0');
                texture.Name.copy(t.Name, texture.Name.length());
            }

            t.Width = texture.Width;
            t.Height = texture.Height;

            if(texture.HasData())
            {
                t.MipMaps[0] = MaxTextureName + sizeof(t.Width) + sizeof(t.Height);

                for(std::size_t i = 0; i < MipTextureLevels - 1; i++)
                    t.MipMaps[i + 1] = t.MipMaps[i] + (t.Width >> i) * (t.Height >> i);

                out.write(reinterpret_cast<const char*>(&t), sizeof(t));

                // Data
                for(std::size_t i = 0; i < MipTextureLevels; i++)
                    out.write(reinterpret_cast<const char*>(texture.MipMapData[i].data()), texture.MipMapData[i].size());

                // Palette
                assert(texture.Palette.size() <= 256);
                short paletteSize = texture.Palette.size();
                out.write(reinterpret_cast<const char*>(&paletteSize), sizeof(paletteSize));

                out.write(reinterpret_cast<const char*>(texture.Palette.data()), sizeof(glm::u8vec3) * texture.Palette.size());
            }
            else
            {
                for(std::size_t i = 0; i < MipTextureLevels; i++)
                    t.MipMaps[i] = 0;
            }

            // Save from `std::ostream`
            out.flush();
        }

        // `m_Data` and `m_DataLength` are already stored.
        // `out.flush()` was called at end of Texture Insert Loop
        // `out.close()` should not be needed + destructor will call it after end of this function
    }

    void BspFile::Save(const std::filesystem::path& filename) const
    {
        std::size_t dataSize = 0;
        for(std::size_t i = 0; i < LumpType_Size; i++)
            dataSize += m_DataLength[i];

        struct LumpEntry
        {
            uint32_t Offset;
            uint32_t Length;
        };

        LumpEntry lumps[LumpType_Size];
        std::size_t prevOffset = sizeof(Magic) + sizeof(lumps);

        for(std::size_t i = 0; i < LumpType_Size; i++)
        {
            lumps[i].Length = m_DataLength[i];
            lumps[i].Offset = prevOffset;
            prevOffset += m_DataLength[i];
        }


        std::ofstream out(filename.string(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

        // Magic number
        typeof(Magic) magic = Magic;
        out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));

        // Lump headers
        out.write(reinterpret_cast<const char*>(lumps), sizeof(lumps));

        // Lump data
        for(std::size_t i = 0; i < LumpType_Size; i++)
            out.write(reinterpret_cast<const char*>(m_Data[i]), m_DataLength[i]);

        out.close();
    }

    void BspFile::TextureParsed::WriteRgbPng(const std::filesystem::path& filename, std::size_t level) const
    {
        std::vector<glm::u8vec3> pixels = AsRgb();
        assert(pixels.size() == Width * Height);
        assert(Width <= std::numeric_limits<int32_t>::max() / 3);
        stbi_write_png(filename.string().c_str(), Width, Height, 3, pixels.data(), static_cast<int32_t>(Width) * 3);
    }

    void BspFile::TextureParsed::WriteRgbaPng(const std::filesystem::path& filename, std::size_t level) const
    {
        std::vector<glm::u8vec4> pixels = AsRgba();
        assert(pixels.size() == Width * Height);
        assert(Width <= std::numeric_limits<int32_t>::max() / 4);
        stbi_write_png(filename.string().c_str(), Width, Height, 4, pixels.data(), static_cast<int32_t>(Width) * 4);
    }
}
