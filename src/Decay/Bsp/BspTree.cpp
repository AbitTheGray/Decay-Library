#include "BspTree.hpp"

#include <fstream>

#include <stb_image_write.h>

namespace Decay::Bsp
{

    BspTree::BspTree(std::shared_ptr<BspFile> bsp)
     : Bsp(std::move(bsp)),
     Textures(Bsp->GetTextures()),
     Vertices(),
     Models(Bsp->GetModelCount())
    {
        for(std::size_t mi = 0; mi < Models.size(); mi++)
        {
            const BspFile::Model& model = Bsp->GetRawModels()[mi];
            Models[mi] = ProcessModel(model);
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

    void BspTree::ExportFlatObj(const std::filesystem::path& filename) const
    {
        std::fstream out(filename.string(), std::ios_base::out | std::ios_base::trunc);

        // Header
        {
            out << "# .obj file generated by Decay Library" << std::endl;

            auto now = std::chrono::system_clock::now();
            std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
            out << "# Exported: " << std::ctime(&nowTime) << std::endl;
        }

        out.flush();

        // Vertices
        for(const auto& vec : Vertices)
        {
            out << "v " << -vec.Position.x << ' ' << vec.Position.z << ' ' << vec.Position.y << std::endl;
            out << "vt " << 1-vec.UV.x << ' ' << vec.UV.y << std::endl;
        }

        out.flush();

        // Indices
        //for(auto& model : Models)
        for(std::size_t mi = 0; mi < Models.size(); mi++)
        {
            auto& model = Models[mi];

            out << "o Model_" << mi << std::endl;

            for(auto& kvp : model->Indices)
            {
                out << "usemtl texture_" << Textures[kvp.first].Name << std::endl;

                auto& indices = kvp.second;

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

                    //THINK Convert indices back to plygon face
                    out << "f " << i0 << '/' << i0 << ' ' << i1 << '/' << i1 << ' ' << i2 << '/' << i2 << std::endl;
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
}
