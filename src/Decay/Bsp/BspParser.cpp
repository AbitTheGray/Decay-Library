#include "BspParser.hpp"

#include <fstream>

namespace Decay::Bsp
{

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
            // Entities
            {
                //TODO
            }

            // Planes
            {
                //TODO
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
                //TODO
            }

            // Texture Info
            {
                //TODO
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
                //TODO
            }

            // Leaves
            {
                //TODO
            }

            // Mark Surface
            {
                //TODO
            }

            // Edges
            {
                assert(m_DataLength[static_cast<uint8_t>(LumpType::Edges)] % sizeof(Edge) == 0);
            }

            // Surface Edges
            {
                //TODO
            }

            // Models
            {
                //TODO
            }
        }
    }

    BspParser::~BspParser()
    {
        for(std::size_t i = 0; i < LumpType_Size; i++)
            free(m_Data[i]);
    }
}
