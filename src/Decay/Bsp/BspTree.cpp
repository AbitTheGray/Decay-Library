#include "BspTree.hpp"

#include <fstream>

namespace Decay::Bsp
{

    BspTree::BspTree(std::shared_ptr<BspFile> bsp)
     : Bsp(std::move(bsp)), Textures(Bsp->GetTextures()), MainNode(ProcessNode(Bsp->GetRawNodes()[0]))
    {

    }

    void BspTree::UpdateNode_ChildIndex(const std::shared_ptr<SmartNode>& smartNode, int16_t childIndex)
    {
        if(childIndex > 0)
        {
            const BspFile::Node& node = Bsp->GetRawNodes()[childIndex];
            smartNode->ChildNodes.emplace_back(ProcessNode(node));
        }
        else
        {
            childIndex = ~static_cast<uint16_t>(childIndex);

            const BspFile::Leaf& leaf = Bsp->GetRawLeaves()[childIndex];
            smartNode->Leaves.emplace_back(ProcessLeaf(leaf));
        }
    }

    void BspTree::Insert_Visual(const std::shared_ptr<SmartNode>& smartNode, const BspFile::Node& node)
    {
        for(std::size_t fi = node.FirstFaceIndex, fic = 0; fic < node.FaceCount; fi++, fic++)
        {
            assert(fi < Bsp->GetFaceCount());
            const BspFile::Face& face = Bsp->GetRawFaces()[fi];

            ProcessFace(face, smartNode->Indices);
        }
    }

    std::shared_ptr<BspTree::SmartLeaf> BspTree::ProcessLeaf(const BspFile::Leaf& leaf)
    {
        std::shared_ptr<BspTree::SmartLeaf> smartLeaf = std::make_shared<BspTree::SmartLeaf>();
        {
            for(std::size_t msi = leaf.FirstMarkSurface, msic = 0; msic < leaf.MarkSurfaceCount; msi++, msic++)
            {
                assert(msi < Bsp->GetMarkSurfaceCount());
                const BspFile::MarkSurface& markSurface = Bsp->GetRawMarkSurfaces()[msi];

                assert(markSurface < Bsp->GetFaceCount());
                const BspFile::Face& face = Bsp->GetRawFaces()[markSurface];

                ProcessFace(face, smartLeaf->Indices);
            }
        }
        return std::move(smartLeaf);
    }

    void BspTree::ProcessFace(const BspFile::Face& face, std::map<uint16_t, std::vector<uint16_t>>& indicesMap)
    {
        if(face.SurfaceEdgeCount == 0)
            return;

        // Texture info
        assert(face.TextureMapping < Bsp->GetTextureMappingCount());
        const BspFile::TextureMapping& textureMapping = Bsp->GetRawTextureMapping()[face.TextureMapping];
        auto textureIndex = textureMapping.Texture;
        assert(textureIndex < Textures.size());

        const Wad::WadFile::Texture& texture = Textures[textureIndex];
        auto& indices = indicesMap[textureIndex];


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
            assert(sei < Bsp->GetSurfaceEdgeCount());
            const BspFile::SurfaceEdges& surfaceEdge = Bsp->GetRawSurfaceEdges()[sei];

            uint16_t index;
            if(surfaceEdge >= 0)
            {
                assert(surfaceEdge < Bsp->GetEdgeCount());
                index = Bsp->GetRawEdges()[surfaceEdge].First;
            }
            else
            {
                assert(-surfaceEdge < Bsp->GetEdgeCount());
                index = Bsp->GetRawEdges()[-surfaceEdge].Second;
            }

            faceIndices[seii] = index;
        }

        // Triangulate the face
        {
            // Main index
            uint16_t mainIndex = Vertices.size();

            auto mainVertex = Bsp->GetRawVertices()[faceIndices[0]];
            Vertices.emplace_back(
                    Vertex {
                            mainVertex,
                            glm::vec2 {
                                    textureMapping.GetTexelU(mainVertex, texture.Size),
                                    textureMapping.GetTexelV(mainVertex, texture.Size)
                            }
                    }
            );

            // Second index
            uint16_t secondIndex = Vertices.size();

            auto secondVertex = Bsp->GetRawVertices()[faceIndices[1]];
            Vertices.emplace_back(
                    Vertex {
                            secondVertex,
                            glm::vec2 {
                                    textureMapping.GetTexelU(secondVertex, texture.Size),
                                    textureMapping.GetTexelV(secondVertex, texture.Size)
                            }
                    }
            );

            // Reserve space in `Vertices`
            Vertices.reserve((face.SurfaceEdgeCount - 2) * 3);

            // Other indices
            for(std::size_t ii = 2; ii < face.SurfaceEdgeCount; ii++)
            {
                uint16_t thirdIndex = Vertices.size();

                auto thirdVertex = Bsp->GetRawVertices()[faceIndices[ii]];
                Vertices.emplace_back(
                        Vertex {
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
            //TODO Fix up axis
            out << "v " << vec.Position.x << ' ' << vec.Position.y << ' ' << vec.Position.z << std::endl;
            out << "vt " << vec.UV.x << ' ' << vec.UV.y << std::endl;
        }

        out.flush();

        // Indices
        auto textureIndices = FlattenIndices();
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
                out << "f " << i0 << '/' << i0 << ' ' << i1 << '/' << i1 << ' ' << i2 << '/' << i2 << ' ' << std::endl;
            }
        }

        out.flush();
    }
}
