#include "BspParser.hpp"

// Use to test images if you are getting weird results.
// Will replace palette for all images with HSL/HSV noise.
// Equivalent to WAD_PALETTE_DUMMY
//#define BSP_PALETTE_DUMMY 1

#include <fstream>
#include <iostream>

namespace Decay::Bsp
{
    std::array<std::size_t, BspParser::LumpType_Size> BspParser::s_DataMaxLength = {
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
    std::array<std::size_t, BspParser::LumpType_Size> BspParser::s_DataElementSize = {
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

    BspParser::BspParser(const std::filesystem::path& filename)
    {
        if(!std::filesystem::exists(filename))
            throw std::runtime_error("File not found");

        std::fstream file(filename, std::ios_base::binary | std::ios_base::in);

        // Magic Number
        {
            uint32_t magicNumber;
            file.read(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber));

            switch(magicNumber)
            {
                case 0x0000001Eu:
                    break; // OK
                case 0x1E000000u:
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
        file.read(reinterpret_cast<char*>(lumps), sizeof(lumps));

        for(std::size_t i = 0; i < LumpType_Size; i++)
        {
            file.seekg(lumps[i].Offset);

            void* d = std::malloc(lumps[i].Length);
            file.read(reinterpret_cast<char*>(d), lumps[i].Length);

            m_Data[i] = d;
            m_DataLength[i] = lumps[i].Length;
        }

        file.close();

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
            }
        }
    }

    BspParser::~BspParser()
    {
        for(std::size_t i = 0; i < LumpType_Size; i++)
            std::free(m_Data[i]);
    }

    std::vector<Wad::WadParser::Texture> BspParser::GetTextures() const
    {
        MemoryBuffer itemDataBuffer(
                reinterpret_cast<char*>(m_Data[static_cast<uint8_t>(LumpType::Textures)]),
                m_DataLength[static_cast<uint8_t>(LumpType::Textures)]
        );
        std::istream in(&itemDataBuffer);

        uint32_t count;
        in.read(reinterpret_cast<char*>(&count), sizeof(count));

        std::vector<uint32_t> offsets(count);
        in.read(reinterpret_cast<char*>(offsets.data()), sizeof(uint32_t) * count);

        std::vector<Wad::WadParser::Texture> textures(count);
        for(std::size_t i = 0; i < count; i++)
        {
            assert(offsets[i] > sizeof(uint32_t) + sizeof(uint32_t) * count);
            in.seekg(offsets[i]);

            Texture texture;
            in.read(reinterpret_cast<char*>(&texture), sizeof(Texture));

            Wad::WadParser::Texture wadTexture = {
                    texture.GetName(),
                    texture.Width,
                    texture.Height
            };

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
                assert(texture.Width == imageSize[0].x);
                assert(texture.Height == imageSize[0].y);

                // 2 Dummy bytes
                // after last MipMap level
                uint8_t dummy[2];
                in.read(reinterpret_cast<char*>(dummy), sizeof(uint8_t) * 2);
                if(dummy[0] != 0x00u)
                    std::cerr << "Texture dummy[0] byte not equal to 0x00 but " << static_cast<uint32_t>(dummy[0]) << " was read." << std::endl;
                if(dummy[1] != 0x01u)
                    std::cerr << "Texture dummy[1] byte not equal to 0x01 but " << static_cast<uint32_t>(dummy[1]) << " was read." << std::endl;

                // Palette
                std::array<uint8_t, Wad::WadParser::Texture::PaletteSize> imagePalette = {};
                in.read(reinterpret_cast<char*>(imagePalette.data()), sizeof(glm::u8vec3) * Wad::WadParser::Texture::PaletteSize);

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

            textures[i] = wadTexture;
        }

        return textures;
    }
}
