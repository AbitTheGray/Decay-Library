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

        // Magic1 Number
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
                    R_ASSERT(m_DataLength[i] < s_DataMaxLength[i] * s_DataElementSize[i], "Lump " << i << " is too short for its content");
                }
            }

            // Entities
            {
                //TODO
            }

            // Planes
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::Planes)] % sizeof(Plane) == 0, "Lump size for Planes does not match exactly expected size");
            }

            // Textures
            {
                //TODO
            }

            // Vertices
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::Vertices)] % sizeof(glm::vec3) == 0, "Lump size for Vertices does not match exactly expected size");
            }

            // Visibility
            {
                //TODO
            }

            // Nodes
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::Nodes)] % sizeof(Node) == 0, "Lump size for Nodes does not match exactly expected size");
            }

            // Texture Mapping
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::TextureMapping)] % sizeof(TextureMapping) == 0, "Lump size for Texture Mapping does not match exactly expected size");
            }

            // Faces
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::Faces)] % sizeof(Face) == 0, "Lump size for Faces does not match exactly expected size");
            }

            // Lighting
            {
                //TODO
            }

            // Clip Nodes
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::ClipNodes)] % sizeof(ClipNode) == 0, "Lump size for Clip Nodes does not match exactly expected size");
            }

            // Leaves
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::Leaves)] % sizeof(Leaf) == 0, "Lump size for Leaves does not match exactly expected size");
            }

            // Mark Surface
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::MarkSurface)] % sizeof(MarkSurface) == 0, "Lump size for Mark Surface does not match exactly expected size");
            }

            // Edges
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::Edges)] % sizeof(Edge) == 0, "Lump size for Edges does not match exactly expected size");
            }

            // Surface Edges
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::SurfaceEdges)] % sizeof(SurfaceEdges) == 0, "Lump size for Surface Edges does not match exactly expected size");
            }

            // Models
            {
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::Models)] % sizeof(Model) == 0, "Lump size for Models does not match exactly expected size");
                R_ASSERT(m_DataLength[static_cast<uint8_t>(LumpType::Models)] >= sizeof(Model), "There must be at least 1 model in the BSP file (for static world)");
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
        R_ASSERT(count < MaxTextures, "Too many textures");

        std::vector<uint32_t> offsets(count);
        in.read(reinterpret_cast<char*>(offsets.data()), sizeof(uint32_t) * count);

        std::vector<Wad::Wad3::WadFile::Texture> textures(count);
        for(std::size_t i = 0; i < count; i++)
        {
            if(static_cast<int32_t>(offsets[i]) == -1) // Missing texture
            {
#ifdef DEBUG
                std::cerr << "Texture " << i << " is missing" << std::endl;
#endif
                continue;
            }

            R_ASSERT(offsets[i] >= sizeof(uint32_t) + sizeof(uint32_t) * count, "Texture data offset points into offset list - too low value");
            R_ASSERT(offsets[i] < m_DataLength[static_cast<uint8_t>(LumpType::Textures)], "Texture data offset is outside of Textures Lump");
            in.seekg(offsets[i]);

            Texture texture = {};
            in.read(reinterpret_cast<char*>(&texture), sizeof(Texture));

            Wad::Wad3::WadFile::Texture wadTexture = {
                texture.Name_str(),
                texture.Width,
                texture.Height
            };
            R_ASSERT(!wadTexture.Name.empty(), "Texture name cannot be empty");

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
                R_ASSERT(texture.Width == wadTexture.MipMapDimensions[0].x, "MipMap level 0 must match texture size (width failed)");
                R_ASSERT(texture.Height == wadTexture.MipMapDimensions[0].y, "MipMap level 0 must match texture size (height failed)");

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
            reinterpret_cast<char*>(data),
            dataLength
        );
        std::ostream out(&dataBuffer);


        // Texture Insert Loop
        for(const auto& texture : textures)
        {
            Texture t = {};

            // Name
            {
                R_ASSERT(texture.Name.length() < MaxTextureName, "Texture name is too long");
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
                R_ASSERT(texture.Palette.size() <= 256, "Texture palette is too big");
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

        // Magic1 number
        typeof(Magic) magic = Magic;
        out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));

        // Lump headers
        out.write(reinterpret_cast<const char*>(lumps), sizeof(lumps));

        // Lump data
        for(std::size_t i = 0; i < LumpType_Size; i++)
            out.write(reinterpret_cast<const char*>(m_Data[i]), m_DataLength[i]);

        out.close();
    }
    void BspFile::SetEntities(const std::string& entities)
    {
        auto& data = m_Data[static_cast<int>(LumpType::Entities)];
        auto& dataLength = m_DataLength[static_cast<int>(LumpType::Entities)];

        std::free(data);

        if(entities.ends_with('\0'))
        {
            dataLength = entities.length();
            data = std::malloc(dataLength);

            std::copy(entities.begin(), entities.end(), reinterpret_cast<char*>(data));
        }
        else
        {
            dataLength = entities.length() + 1;
            data = std::malloc(dataLength);

            std::copy(entities.begin(), entities.end(), reinterpret_cast<char*>(data));
            reinterpret_cast<char*>(data)[dataLength - 1] = '\0';
        }
    }
    void BspFile::TextureParsed::WriteRgbPng(const std::filesystem::path& filename, std::size_t level) const
    {
        std::vector<glm::u8vec3> pixels = AsRgb();
        R_ASSERT(pixels.size() == Width * Height, "Too many pixels to write");
        R_ASSERT(Width <= std::numeric_limits<int32_t>::max() / 3, "Width is too big (numeric overflow)");
        stbi_write_png(filename.string().c_str(), Width, Height, 3, pixels.data(), static_cast<int32_t>(Width) * 3);
    }
    void BspFile::TextureParsed::WriteRgbaPng(const std::filesystem::path& filename, std::size_t level) const
    {
        std::vector<glm::u8vec4> pixels = AsRgba();
        R_ASSERT(pixels.size() == Width * Height, "Too many pixels to write");
        R_ASSERT(Width <= std::numeric_limits<int32_t>::max() / 4, "Width is too big (numeric overflow)");
        stbi_write_png(filename.string().c_str(), Width, Height, 4, pixels.data(), static_cast<int32_t>(Width) * 4);
    }
}
