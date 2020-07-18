#include "BspTree.hpp"

#include <map>
#include <vector>

namespace Decay::Bsp
{

    BspTree::BspTree(std::shared_ptr<BspFile> bsp)
            : Bsp(std::move(bsp)),
              Textures(Bsp->GetTextures()),
              Vertices(),
              Models(Bsp->GetModelCount()),
              Entities(ParseEntities(Bsp->GetRawEntityChars(), Bsp->GetEntityCharCount())),
              Entities_Model(),
              Entities_Name(),
              Entities_Type()
    {
        // Parse models
        for(std::size_t mi = 0; mi < Models.size(); mi++)
        {
            const BspFile::Model& model = Bsp->GetRawModels()[mi];
            Models[mi] = ProcessModel(model);
        }

        // Process entities into fast-access maps
        for(const Entity& ent : Entities)
        {
            auto classname = ent.find("classname");
            auto name = ent.find("targetname");
            auto model = ent.find("model");

            if(classname != ent.end())
                Entities_Type[classname->second].emplace_back(ent);

            if(name != ent.end())
                Entities_Name[name->second].emplace_back(ent);

            if(model != ent.end())
            {
                if(model->second.size() > 1 && model->second[0] == '*')
                {
                    try
                    {
                        int value = std::stoi(model->second.data() + 1);
                        Entities_Model.emplace(value, ent);
                    }
                    catch(std::invalid_argument& ex)
                    {
                        std::cerr << "Model '" << (model->second.data() + 1) << "' could not be parsed - No Conversion" << std::endl;
                    }
                    catch(std::out_of_range& ex)
                    {
                        std::cerr << "Model '" << (model->second.data() + 1) << "' could not be parsed - Out of Range" << std::endl;
                    }
                }
            }
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

        // This should be optimized by compiler
        smartFace.LightingStyles[0] = face.LightingStyles[0];
        smartFace.LightingStyles[1] = face.LightingStyles[1];
        smartFace.LightingStyles[2] = face.LightingStyles[2];
        smartFace.LightingStyles[3] = face.LightingStyles[3];

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

        // Lightmap calculation
        glm::vec2 uvStart = {0, 0}, uvEnd = {0, 0}, uvSize = {0, 0};
        auto& plane = Bsp->GetRawPlanes()[face.Plane];
        float minS = 0, maxS = 0;
        float minT = 0, maxT = 0;
        glm::ivec2 lightmapSize;

        assert(face.LightmapOffset >= -1);
        if(face.LightmapOffset != -1)
        {
            // Get UV bounds
            {
                auto& vertex = Bsp->GetRawVertices()[faceIndices[0]];

                float s = textureMapping.GetTexelS(vertex);
                minS = s;
                maxS = s;

                float t = textureMapping.GetTexelT(vertex);
                minT = t;
                maxT = t;
            }
            for(std::size_t ii = 1; ii < face.SurfaceEdgeCount; ii++)
            {
                auto& vertex = Bsp->GetRawVertices()[faceIndices[ii]];

                float s = textureMapping.GetTexelS(vertex);
                if(s < minS)
                    minS = s;
                if(s > maxS)
                    maxS = s;

                float t = textureMapping.GetTexelT(vertex);
                if(t < minT)
                    minT = t;
                if(t > maxT)
                    maxT = t;
            }

            lightmapSize = glm::ivec2(
                    ceilf(maxS / 16.0f) - floorf(minS / 16.0f) + 1,
                    ceilf(maxT / 16.0f) - floorf(minT / 16.0f) + 1
            );
            assert(lightmapSize.x > 0);
            assert(lightmapSize.y > 0);
            /*
            assert(lightmapSize.x <= 16);
            assert(lightmapSize.y <= 16);
             */
            int lightmapLength = lightmapSize.x * lightmapSize.y;
            if(face.LightmapOffset + lightmapLength * 3 > Bsp->GetLightingCount() * 3)
                throw std::runtime_error("Indexing outside of Lightmap");

            AddLight(
                    lightmapSize,
                    reinterpret_cast<const glm::u8vec3*>(reinterpret_cast<const uint8_t*>(Bsp->GetRawLighting()) + face.LightmapOffset),
                    uvStart,
                    uvEnd
            );
            uvSize = uvEnd - uvStart;
        }


        // Triangulate the face
        {
#ifdef DECAY_BSP_LIGHTMAP_ST_INSTEAD_OF_UV
            lightmapSize = {1, 1};
#endif

            // Main index
            auto mainVertex = Bsp->GetRawVertices()[faceIndices[0]];
#ifdef DECAY_BSP_ST_INSTEAD_OF_UV
            glm::vec2 mainUV = glm::vec2(
                    textureMapping.GetTexelS(mainVertex),
                    textureMapping.GetTexelT(mainVertex)
            );
#else
            glm::vec2 mainUV = glm::vec2(
                    textureMapping.GetTexelU(mainVertex, texture.Size),
                    textureMapping.GetTexelV(mainVertex, texture.Size)
            );
#endif
            glm::vec2 mainLightUV = uvStart + glm::vec2(
                    (textureMapping.GetTexelS(mainVertex) - minS) / lightmapSize.s * uvSize.s,
                    (textureMapping.GetTexelT(mainVertex) - minT) / lightmapSize.t * uvSize.t
            );
            uint16_t mainIndex = AddVertex(
                    Vertex {
                            mainVertex,
                            mainUV,
                            mainLightUV
                    }
            );

            // Second index
            auto secondVertex = Bsp->GetRawVertices()[faceIndices[1]];
#ifdef DECAY_BSP_ST_INSTEAD_OF_UV
            glm::vec2 secondUV = glm::vec2(
                    textureMapping.GetTexelS(secondVertex),
                    textureMapping.GetTexelT(secondVertex)
            );
#else
            glm::vec2 secondUV = glm::vec2(
                    textureMapping.GetTexelU(secondVertex, texture.Size),
                    textureMapping.GetTexelV(secondVertex, texture.Size)
            );
#endif
            glm::vec2 secondLightUV = glm::vec2(
                    (textureMapping.GetTexelS(secondVertex) - minS) / lightmapSize.s * uvSize.s,
                    (textureMapping.GetTexelT(secondVertex) - minT) / lightmapSize.t * uvSize.t
            );
            uint16_t secondIndex = AddVertex(
                    Vertex {
                            secondVertex,
                            secondUV,
                            secondLightUV
                    }
            );

            // Other indices
            assert(face.SurfaceEdgeCount >= 3);
            for(std::size_t ii = 2; ii < face.SurfaceEdgeCount; ii++)
            {
                auto thirdVertex = Bsp->GetRawVertices()[faceIndices[ii]];
#ifdef DECAY_BSP_ST_INSTEAD_OF_UV
                glm::vec2 thirdUV = glm::vec2(
                        textureMapping.GetTexelS(thirdVertex),
                        textureMapping.GetTexelT(thirdVertex)
                );
#else
                glm::vec2 thirdUV = glm::vec2(
                        textureMapping.GetTexelU(thirdVertex, texture.Size),
                        textureMapping.GetTexelV(thirdVertex, texture.Size)
                );
#endif
                glm::vec2 thirdLightUV = glm::vec2(
                        (textureMapping.GetTexelS(thirdVertex) - minS) / lightmapSize.s * uvSize.s,
                        (textureMapping.GetTexelT(thirdVertex) - minT) / lightmapSize.t * uvSize.t
                );
                uint16_t thirdIndex = AddVertex(
                        Vertex {
                                thirdVertex,
                                thirdUV,
                                thirdLightUV
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
            // Position
            out << "v " << -vec.Position.x << ' ' << vec.Position.z << ' ' << vec.Position.y << std::endl;

            // Texture coordinates
#ifdef DECAY_BSP_ST_INSTEAD_OF_UV
            #warning Exporting OBJ will use ST texture coordinates instead of UV
            out << "vt " << vec.ST.s << ' ' << 1-vec.ST.t << std::endl;
#else
            out << "vt " << vec.UV.x << ' ' << 1-vec.UV.y << std::endl;
#endif
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

        std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data)> writeFunc = ImageWriteFunction_RGBA(textureExtension);

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

    void BspTree::AddLight(glm::uvec2 size, const glm::u8vec3* data, glm::vec2& out_start, glm::vec2& out_end)
    {
        // Insert into texture
        float w1 = 1.0f / Light.Width;
        float h1 = 1.0f / Light.Height;
        {
            //TODO Optimize
            // Check few pixels (left, right, middle) before checking whole row?
            for(uint32_t y = 0; y < Light.Height - size.y; y++)
            {
                for(uint32_t x = 0; x < Light.Width - size.x; x++)
                {
                    if(Light.CanInsert(x, y, size))
                    {
                        Light.Insert(x, y, size, data);

                        out_start = {
                                x * w1,
                                y * h1
                        };
                        out_end = {
                                static_cast<float>(x + size.x) * w1,
                                static_cast<float>(y + size.y) * h1
                        };
                        return; // Successfully added
                    }
                }
            }
            // No space to insert into!
        }

        // Resize texture
        {
            const uint32_t w = Light.Width;
            const uint32_t h = Light.Height;

            if(w >= BspTree::Lightmap::MaxSize || h >= BspTree::Lightmap::MaxSize)
                throw std::runtime_error("Lightmap is too big");

            Light.Width *= 2;
            Light.Height *= 2;

            const std::size_t dataSize = w * h * 4;
            Light.Data.resize(dataSize);
            Light.Used.resize(dataSize);

            auto data_ptr = Light.Data.begin();
            auto used_ptr = Light.Used.begin();

            // Move content of Light.Data & Light.
            // Use to top-left corner instead of top quarter.
            for(int64_t oldRow = h, newRow = Light.Height; oldRow >= 0; oldRow--, newRow -= 2)
            {
                std::copy(
                        data_ptr + (w * oldRow),
                        data_ptr + ((w+1) * oldRow),
                        data_ptr + (w * newRow)
                );
                std::copy(
                        used_ptr + (w * oldRow),
                        used_ptr + ((w+1) * oldRow),
                        used_ptr + (w * newRow)
                );
            }
        }

#ifndef DECAY_BSP_LIGHTMAP_ST_INSTEAD_OF_UV
        // Recalculate UV
        for(auto& vertex : Vertices)
            vertex.LightUV *= 0.5f;
#endif

        // Insert into texture
        // Same as above but skips previous pixels
        {
            //TODO Optimize
            // Check few pixels (left, right, middle) before checking whole row?
            assert(Light.Height / 2 - 16 >= 0);
            assert(Light.Width / 2 - 16 >= 0);
            for(uint32_t y = Light.Height / 2 - 16; y < Light.Height - size.y; y++)
            {
                for(uint32_t x = Light.Width / 2 - 16; x < Light.Width - size.x; x++)
                {
                    if(Light.CanInsert(x, y, size))
                    {
                        Light.Insert(x, y, size, data);

                        out_start = {
                                x * w1,
                                y * h1
                        };
                        out_end = {
                                static_cast<float>(x + size.x) * w1,
                                static_cast<float>(y + size.y) * h1
                        };
                        return; // Successfully added
                    }
                }
            }
            // No space to insert into!
        }

        throw std::runtime_error("Could not insert into resized lightmap");
    }
}
