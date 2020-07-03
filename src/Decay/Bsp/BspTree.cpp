#include "BspTree.hpp"

#include <fstream>

#include <stb_image_write.h>

namespace Decay::Bsp
{

    BspTree::BspTree(std::shared_ptr<BspFile> bsp)
     : Bsp(std::move(bsp)),
     Textures(Bsp->GetTextures()),
     Vertices(),
     Models(Bsp->GetModelCount()),
     Entities(ParseEntities(Bsp->GetRawEntityChars(), Bsp->GetEntityCharCount()))
    {
        for(std::size_t mi = 0; mi < Models.size(); mi++)
        {
            const BspFile::Model& model = Bsp->GetRawModels()[mi];
            Models[mi] = ProcessModel(model);
        }

        for(auto& ent : Entities)
        {
            auto classname = ent.find("classname");
            auto name = ent.find("Name");

            if(name == ent.end())
                std::cout << (classname == ent.end() ? "???" : classname->second) << std::endl;
            else
                std::cout << (classname == ent.end() ? "???" : classname->second) << ": " << name->second << std::endl;
        }
    }

    BspTree::Face BspTree::ProcessFace(const BspFile::Face& face)
    {
        if(face.SurfaceEdgeCount == 0)
            return {};

        // Texture info
        assert(face.TextureMapping < Bsp->GetTextureMappingCount());
        const BspFile::TextureMapping& textureMapping = Bsp->GetRawTextureMapping()[face.TextureMapping];
        auto textureIndex = textureMapping.Texture;
        assert(textureIndex < Textures.size());

        const Wad::WadFile::Texture& texture = Textures[textureIndex];


        Face smartFace = Face();

        smartFace.LightingStyles[0] = face.LightingStyles[0];
        smartFace.LightingStyles[1] = face.LightingStyles[1];
        smartFace.LightingStyles[2] = face.LightingStyles[2];
        smartFace.LightingStyles[3] = face.LightingStyles[3];

        smartFace.LightmapOffset = face.LightmapOffset;

        smartFace.TextureId = textureIndex;

        // Reserve space in `Vertices` and `Indices`
        {
            std::size_t indicesCount = (face.SurfaceEdgeCount - 2) * 3;
            Vertices.reserve(indicesCount);
            smartFace.Indices.reserve(indicesCount);
        }


        // Get vertex indices from: Face -> Surface Edge -> Edge (-> Vertex)
        assert(face.SurfaceEdgeCount >= 3); // To at least for a triangle
        std::vector<uint16_t> faceIndices(face.SurfaceEdgeCount);
        for(
            std::size_t sei = face.FirstSurfaceEdge, seii = 0;
            seii < face.SurfaceEdgeCount;
            sei++, seii++
        )
        {
            assert(sei < Bsp->GetSurfaceEdgeCount());
            const BspFile::SurfaceEdges& surfaceEdge = Bsp->GetRawSurfaceEdges()[sei];

            if(surfaceEdge >= 0)
            {
                assert(surfaceEdge < Bsp->GetEdgeCount());
                faceIndices[seii] = Bsp->GetRawEdges()[surfaceEdge].First;
            }
            else
            {
                assert(-surfaceEdge < Bsp->GetEdgeCount());
                faceIndices[seii] = Bsp->GetRawEdges()[-surfaceEdge].Second; // Quake used ~ (swap bits) instead of - (swap sign)
            }
        }


        // Triangulate the face
        {
            // Main index
            auto mainVertex = Bsp->GetRawVertices()[faceIndices[0]];
            uint16_t mainIndex = AddVertex(
                    Vertex {
                            mainVertex,
                            glm::vec2 {
                                    textureMapping.GetTexelU(mainVertex, texture.Size),
                                    textureMapping.GetTexelV(mainVertex, texture.Size)
                            },
                            glm::vec2 {
                                    textureMapping.GetTexelS(mainVertex),
                                    textureMapping.GetTexelT(mainVertex)
                            }
                    }
            );

            // Second index
            auto secondVertex = Bsp->GetRawVertices()[faceIndices[1]];
            uint16_t secondIndex = AddVertex(
                    Vertex {
                            secondVertex,
                            glm::vec2 {
                                    textureMapping.GetTexelU(secondVertex, texture.Size),
                                    textureMapping.GetTexelV(secondVertex, texture.Size)
                            },
                            glm::vec2 {
                                    textureMapping.GetTexelS(secondVertex),
                                    textureMapping.GetTexelT(secondVertex)
                            }
                    }
            );

            // Other indices
            assert(face.SurfaceEdgeCount >= 3);
            for(std::size_t ii = 2; ii < face.SurfaceEdgeCount; ii++)
            {
                auto thirdVertex = Bsp->GetRawVertices()[faceIndices[ii]];
                uint16_t thirdIndex = AddVertex(
                        Vertex {
                                thirdVertex,
                                glm::vec2 {
                                        textureMapping.GetTexelU(thirdVertex, texture.Size),
                                        textureMapping.GetTexelV(thirdVertex, texture.Size)
                                },
                                glm::vec2 {
                                        textureMapping.GetTexelS(thirdVertex),
                                        textureMapping.GetTexelT(thirdVertex)
                                }
                        }
                );

                // Add triangle to indices
                smartFace.Indices.emplace_back(mainIndex);
                smartFace.Indices.emplace_back(secondIndex);
                smartFace.Indices.emplace_back(thirdIndex);

                // Current 3rd index is 2nd index for next
                secondIndex = thirdIndex;
            }
        }

        assert(smartFace.Indices.size() % 3 == 0);

        return smartFace;
    }

    void BspTree::ExportFlatObj(const std::filesystem::path& filename, const std::filesystem::path& mtlFilename) const
    {
        std::fstream out(filename.string(), std::ios_base::out | std::ios_base::trunc);

        // Header
        {
            out << "# .obj file generated by Decay Library" << std::endl;

            auto now = std::chrono::system_clock::now();
            std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
            out << "# Exported: " << std::ctime(&nowTime) << std::endl;
        }

        // MTL file
        if(!mtlFilename.empty())
            out << "mtllib " << mtlFilename << std::endl;

        out.flush();

        // Vertices
        for(const auto& vec : Vertices)
        {
            out << "v " << -vec.Position.x << ' ' << vec.Position.z << ' ' << vec.Position.y << std::endl;
            out << "vt " << vec.UV.x << ' ' << 1-vec.UV.y << std::endl;
        }

        out.flush();

        // Indices
        //for(auto& model : Models)
        for(std::size_t mi = 0; mi < Models.size(); mi++)
        {
            out << std::endl;

            out << "o Model_" << mi << std::endl;

            for(auto& kvp : Models[mi]->Indices)
            {
                out << std::endl;

                out << "g texture_" << Textures[kvp.first].Name << std::endl;
                out << "usemtl texture_" << Textures[kvp.first].Name << std::endl;

                auto& indices = kvp.second;

#ifdef BSP_OBJ_POLYGONS
                bool prevPolygon = false;
#endif

                assert(indices.size() % 3 == 0);
                for(std::size_t ii = 0; ii < indices.size(); ii += 3)
                {
                    // +1 because OBJ starts at 1 instead of 0
                    uint16_t i0 = indices[ii + 0] + 1;
                    uint16_t i1 = indices[ii + 1] + 1;
                    uint16_t i2 = indices[ii + 2] + 1;

                    assert(i0 <= Vertices.size());
                    assert(i1 <= Vertices.size());
                    assert(i2 <= Vertices.size());

#ifdef BSP_OBJ_POLYGONS
                    if(prevPolygon)
                        out  << ' ' << i2 << '/' << i2;
                    else
                        out << "f " << i0 << '/' << i0 << ' ' << i1 << '/' << i1 << ' ' << i2 << '/' << i2;
#else
                    out << "f " << i0 << '/' << i0 << ' ' << i1 << '/' << i1 << ' ' << i2 << '/' << i2 << std::endl;
#endif

#ifdef BSP_OBJ_POLYGONS
                    // Is there another triangle?
                    if(ii + 3 < indices.size())
                    {
                        uint16_t i3 = indices[ii + 0 + 3] + 1;
                        uint16_t i4 = indices[ii + 1 + 3] + 1;
                        uint16_t i5 = indices[ii + 2 + 3] + 1;

                        // Format of output from polygon->triangles function
                        // [ii + 0] is same
                        // old [ii + 2] -> new [ii + 1]
                        // Only new index is new [ii + 2]
                        prevPolygon = (i0 == i3 && i2 == i4);

                        // Next triangle is not from same polygon
                        if(!prevPolygon)
                            out << std::endl;
                    }
                    else
                    {
                        //prevPolygon = false; // Not needed as there are no more indices
                        out << std::endl;
                    }
#else
                    out << std::endl;
#endif
                }
            }

            out.flush();
        }
    }

    void BspTree::ExportMtl(const std::filesystem::path& filename, const std::filesystem::path& texturePath, const std::string& textureExtension) const
    {
        std::fstream out(filename.string(), std::ios_base::out | std::ios_base::trunc);

        // Header
        {
            out << "# .mtl file generated by Decay Library" << std::endl;

            auto now = std::chrono::system_clock::now();
            std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
            out << "# Exported: " << std::ctime(&nowTime) << std::endl;
        }

        out.flush();

        for(auto& texture : Textures)
        {
            auto imgPath = texturePath == "." ?
                    std::filesystem::path(texture.Name + textureExtension) :
                    texturePath / (texture.Name + textureExtension);

            out << "newmtl texture_" << texture.Name << std::endl;
            out << "Ka 1.0 1.0 1.0" << std::endl;
            out << "Kd 1.0 1.0 1.0" << std::endl;
            out << "Ks 0.0 0.0 0.0" << std::endl;
            out << "illum 1" << std::endl;
            out << "map_Ka " << imgPath << std::endl;
            out << "map_Kd " << imgPath << std::endl;

            out << std::endl;

            out.flush();
        }
    }

    std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data)> BspTree::GetImageWriteFunction(const std::string& extension)
    {
        if(extension == ".png" || extension == ".PNG")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data) -> void
            {
                // Write to file
                stbi_write_png(
                        path,
                        width,
                        height,
                        4,
                        data,
                        static_cast<int32_t>(width) * 4
                );
            };
        }
        else if(extension == ".bmp" || extension == ".BMP")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data) -> void
            {
                stbi_write_bmp(
                        path,
                        width,
                        height,
                        4,
                        data
                );
            };
        }
        else if(extension == ".tga" || extension == ".TGA")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data) -> void
            {
                stbi_write_tga(
                        path,
                        width,
                        height,
                        4,
                        data
                );
            };
        }
        else if(extension == ".jpg" || extension == ".JPG" || extension == ".jpeg" || extension == ".JPEG")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data) -> void
            {
                stbi_write_jpg(
                        path,
                        width,
                        height,
                        4,
                        data,
                        100 // 0 = minimum, 100 = maximum
                );
            };
        }
        else if(extension == ".raw" || extension == ".RAW")
        {
            return [](const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data) -> void
            {
                std::fstream out(path, std::ios_base::out | std::ios_base::trunc);

                out.write(reinterpret_cast<const char*>(&width), sizeof(width));
                out.write(reinterpret_cast<const char*>(&height), sizeof(height));

                out.write(reinterpret_cast<const char*>(data), static_cast<std::size_t>(width) * height);

                out.flush();
            };
        }
        else
            throw std::runtime_error("Unsupported texture extension for export");
    }

    void BspTree::ExportTextures(const std::filesystem::path& directory, const std::string& textureExtension, bool dummyForMissing) const
    {
        if(std::filesystem::exists(directory))
        {
            if(!std::filesystem::is_directory(directory))
                throw std::runtime_error("`directory` argument does not point to directory");
        }
        else
            std::filesystem::create_directory(directory);

        assert(textureExtension.size() > 1);
        assert(textureExtension[0] == '.');

        std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data)> writeFunc = GetImageWriteFunction(textureExtension);

        for(auto& texture : Textures)
        {
            std::vector<glm::u8vec4> rgba;
            if(texture.MipMapData[0].empty()) // No data
            {
                if(!dummyForMissing)
                    continue;

                // Crete dummy data
                rgba.resize(texture.Width * texture.Height);
                std::fill(
                        rgba.begin(),
                        rgba.end(),
                        glm::u8vec4(0xBFu, 0xBFu, 0xBFu, 0xFFu)
                );
            }
            else
                rgba = texture.AsRgba();

            writeFunc(
                    (directory / (texture.Name + textureExtension)).string().c_str(),
                    texture.Width,
                    texture.Height,
                    rgba.data()
            );
        }
    }

    std::vector<std::map<std::string, std::string>> BspTree::ParseEntities(const char* raw, std::size_t len)
    {
        std::vector<std::map<std::string, std::string>> entities = {};
        std::map<std::string, std::string> entity = {};

        const char* keyStart = nullptr;
        const char* keyEnd = nullptr;

        const char* valueStart = nullptr;

        for(std::size_t i = 0; i < len; i++)
        {
            char c = raw[i];
            switch(c)
            {
                case '\0':
                [[unlikely]]
                {
                    break;
                }

                // Start of entity
                case '{':
                {
                    if(!entity.empty())
                    {
                        if(keyStart == nullptr || (keyEnd != nullptr && valueStart == nullptr))
                            std::cerr << "Non-empty entity at start of new entity" << std::endl;
                    }
                    break;
                }

                // End of entity
                case '}':
                {
                    if(entity.empty())
                    {
                        std::cerr << "Empty entity after its end" << std::endl;
                        break;
                    }

                    entities.emplace_back(entity);
                    entity.clear();
                    break;
                }

                // Key/Value encasement characters
                case '"':
                {
                    if(keyStart == nullptr)
                        keyStart = raw + i + 1;
                    else if(keyEnd == nullptr)
                        keyEnd = raw + i;
                    else if(valueStart == nullptr)
                        valueStart = raw + i + 1;
                    else // valueEnd
                    {
                        entity.emplace(
                            std::string(keyStart, keyEnd),
                            std::string(valueStart, raw + i)
                        );

                        keyStart = nullptr;
                        keyEnd = nullptr;
                        valueStart = nullptr;
                    }
                }

                // White character, valid anywhere
                case ' ':
                    break;

                // New line
                case '\n':
                {
                    if(keyStart != nullptr)
                    {
                        std::cerr << "Unexpected new-line character" << std::endl;
                        break;
                    }
                    break;
                }

                // Special characters
                case '\t':
                case '\b':
                [[unlikely]]
                {
                    std::cerr << "Unsupported " << (int)c << " character" << std::endl;
                    break;
                }

                // Content of entity
                default:
                [[likely]]
                {
                    if(keyStart == nullptr)
                    [[unlikely]]
                    {
                        std::cerr << "Unexpected character '" << c << "' (" << (int)c << ") in key" << std::endl;
                        break;
                    }
                    if(keyEnd != nullptr && valueStart == nullptr)
                    [[unlikely]]
                    {
                        std::cerr << "Unexpected character '" << c << "' (" << (int)c << ") in value" << std::endl;
                        break;
                    }
                    break;
                }
            }
        }

        if(keyStart != nullptr)
            std::cerr << "Incomplete key-value pair after entity processing" << std::endl;
        if(!entity.empty())
            std::cerr << "Non-empty entity after entity processing" << std::endl;

        return entities;
    }
}
