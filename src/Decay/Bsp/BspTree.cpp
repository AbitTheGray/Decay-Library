#include "BspTree.hpp"

#include <fstream>

namespace Decay::Bsp
{

    BspTree::BspTree(std::shared_ptr<BspFile> bsp)
     : Bsp(std::move(bsp)),
     Textures(Bsp->GetTextures()),
     Vertices(Bsp->GetVertexCount()),
     Models(Bsp->GetModelCount())
    {
        for(std::size_t mi = 0; mi < Models.size(); mi++)
        {
            BspFile::Model& model = Bsp->GetRawModels()[mi];
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
            if(mainVertex.x == -1952 && mainVertex.z == 32 && mainVertex.y == 1152)
                std::cout << faceIndices[0] << std::endl;

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
            if(secondVertex.x == -1952 && secondVertex.z == 32 && secondVertex.y == 1152)
                std::cout << faceIndices[0] << std::endl;

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
                if(thirdVertex.x == -1952 && thirdVertex.z == 32 && thirdVertex.y == 1152)
                    std::cout << faceIndices[0] << std::endl;

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

    void BspTree::ExportFlatObj(const std::filesystem::path& filename)
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
                out << "usemtl texture_" << kvp.first << std::endl;

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
        }

        /*
        auto textureIndices = FlattenIndices_Models();
        for(const auto& it : textureIndices)
        {
            const auto& textureId = it.first;
            //TODO Define texture + mtl

            const auto& indices = it.second;

            assert(indices.size() % 3 == 0);
            for(std::size_t i = 0; i < indices.size(); i += 3)
            {
                // +1 because OBJ starts at 1 instead of 0
                uint16_t i0 = indices[i + 0] + 1;
                uint16_t i1 = indices[i + 1] + 1;
                uint16_t i2 = indices[i + 2] + 1;

                assert(i0 <= Vertices.size());
                assert(i1 <= Vertices.size());
                assert(i2 <= Vertices.size());

                //THINK Convert indices back to plygon face
                out << "f " << i0 << '/' << i0 << ' ' << i1 << '/' << i1 << ' ' << i2 << '/' << i2 << std::endl;
            }
        }
         */

        out.flush();
    }
}
