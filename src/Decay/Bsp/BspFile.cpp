#include "BspFile.hpp"

// Use to test images if you are getting weird results.
// Will replace palette for all images with HSL/HSV noise.
// Equivalent to WAD_PALETTE_DUMMY
//#define BSP_PALETTE_DUMMY 1

#include <fstream>
#include <iostream>

#include <stb_image_write.h>

namespace Decay::Bsp
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

    std::vector<Wad::WadFile::Texture> BspFile::GetTextures() const
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

        std::vector<Wad::WadFile::Texture> textures(count);
        for(std::size_t i = 0; i < count; i++)
        {
            assert(offsets[i] >= sizeof(uint32_t) + sizeof(uint32_t) * count);
            assert(offsets[i] < m_DataLength[static_cast<uint8_t>(LumpType::Textures)]);
            in.seekg(offsets[i]);

            Texture texture = {};
            in.read(reinterpret_cast<char*>(&texture), sizeof(Texture));

            Wad::WadFile::Texture wadTexture = {
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

                // 2 Dummy bytes
                // after last MipMap level
                uint8_t dummy[2];
                in.read(reinterpret_cast<char*>(dummy), sizeof(uint8_t) * 2);
                if(dummy[0] != 0x00u)
                    std::cerr << "Texture dummy[0] byte not equal to 0x00 but " << static_cast<uint32_t>(dummy[0]) << " was read." << std::endl;
                if(dummy[1] != 0x01u)
                    std::cerr << "Texture dummy[1] byte not equal to 0x01 but " << static_cast<uint32_t>(dummy[1]) << " was read." << std::endl;

                // Palette
                in.read(reinterpret_cast<char*>(wadTexture.Palette.data()), sizeof(glm::u8vec3) * Wad::WadFile::Texture::PaletteSize);

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

    void BspFile::ProcessNode_Children(const std::shared_ptr<SmartNode>& smartNode, int16_t childIndex, const std::shared_ptr<NodeTree>& tree) const
    {
        if(childIndex > 0)
        {
            const Node& node = GetRawNodes()[childIndex];
            smartNode->ChildNodes.emplace_back(ProcessNode(node, tree));
        }
        else
        {
            childIndex = ~static_cast<uint16_t>(childIndex);

            const Leaf& leaf = GetRawLeaves()[childIndex];
            //TODO Process leaf
        }
    }

    void BspFile::ProcessNode_Visual(const std::shared_ptr<SmartNode>& smartNode, const BspFile::Node& node, const std::shared_ptr<NodeTree>& tree) const
    {
        for(std::size_t fi = node.FirstFaceIndex, fic = 0; fic < node.FaceCount; fi++, fic++)
        {
            assert(fi < GetFaceCount());
            const Face& face = GetRawFaces()[fi];
            if(face.SurfaceEdgeCount == 0)
                continue;

            // Texture info
            assert(face.TextureMapping < GetTextureMappingCount());
            const TextureMapping& textureMapping = GetRawTextureMapping()[face.TextureMapping];
            auto textureIndex = textureMapping.Texture;
            assert(textureIndex < tree->Textures.size());

            Wad::WadFile::Texture& texture = tree->Textures[textureIndex];
            auto& indices = smartNode->Indices[textureIndex];


            //TODO Lighting (style + offset)


            // Get vertex indices from: Face -> Surface Edge -> Edge (-> Vertex)
            assert(face.SurfaceEdgeCount >= 3); // To at least for a triangle
            std::vector<uint16_t> faceIndices(face.SurfaceEdgeCount);
            for(
                    std::size_t sei = face.FirstSurfaceEdge, seii = 0;
                    seii < face.SurfaceEdgeCount;
                    sei++, seii++
                    )
            {
                assert(sei < GetSurfaceEdgeCount());
                const SurfaceEdges& surfaceEdge = GetRawSurfaceEdges()[sei];

                uint16_t index;
                if(surfaceEdge >= 0)
                {
                    assert(surfaceEdge < GetEdgeCount());
                    index = GetRawEdges()[surfaceEdge].First;
                }
                else
                {
                    assert(-surfaceEdge < GetEdgeCount());
                    index = GetRawEdges()[-surfaceEdge].Second;
                }

                faceIndices[seii] = index;
            }

            // Triangulate the face
            {
                // Main index
                uint16_t mainIndex = tree->Vertices.size();

                auto mainVertex = GetRawVertices()[faceIndices[0]];
                tree->Vertices.emplace_back(
                        TreeVertex {
                                mainVertex,
                                glm::vec2 {
                                        textureMapping.GetTexelU(mainVertex, texture.Size),
                                        textureMapping.GetTexelV(mainVertex, texture.Size)
                                }
                        }
                );

                // Second index
                uint16_t secondIndex = tree->Vertices.size();

                auto secondVertex = GetRawVertices()[faceIndices[1]];
                tree->Vertices.emplace_back(
                        TreeVertex {
                                secondVertex,
                                glm::vec2 {
                                        textureMapping.GetTexelU(secondVertex, texture.Size),
                                        textureMapping.GetTexelV(secondVertex, texture.Size)
                                }
                        }
                );

                // Reserve space in `tree->Vertices`
                tree->Vertices.reserve((face.SurfaceEdgeCount - 2) * 3);

                // Other indices
                for(std::size_t ii = 2; ii < face.SurfaceEdgeCount; ii++)
                {
                    uint16_t thirdIndex = tree->Vertices.size();

                    auto thirdVertex = GetRawVertices()[faceIndices[ii]];
                    tree->Vertices.emplace_back(
                            TreeVertex {
                                    thirdVertex,
                                    glm::vec2 {
                                            textureMapping.GetTexelU(thirdVertex, texture.Size),
                                            textureMapping.GetTexelV(thirdVertex, texture.Size)
                                    }
                            }
                    );

                    // Add triangle to indices
                    indices.emplace_back(mainIndex);
                    indices.emplace_back(secondIndex);
                    indices.emplace_back(thirdIndex);

                    // Current 3rd index is 2nd index for next
                    secondIndex = thirdIndex;
                }
            }
        }
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
