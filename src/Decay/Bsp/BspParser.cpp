#include "BspParser.hpp"

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
            free(m_Data[i]);
    }
}
