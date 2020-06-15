#pragma once

#include "BspFile.hpp"

namespace Decay::Bsp
{
    class BspTree
    {
    public:
        explicit BspTree(std::shared_ptr<BspFile> bsp);

    public:
        const std::shared_ptr<BspFile> Bsp;
        const std::vector<Wad::WadFile::Texture> Textures;

    public:
        struct Vertex
        {
            glm::vec3 Position;
            glm::vec2 UV;
        };
        std::vector<Vertex> Vertices;

    public:
        class SmartLeaf : std::enable_shared_from_this<SmartLeaf>
        {
        public:
            /// [ textureIndex ] = vertex indices
            std::map<uint16_t, std::vector<uint16_t>> Indices;
        };
        class SmartNode : std::enable_shared_from_this<SmartNode>
        {
        public:
            /// [ textureIndex ] = vertex indices
            std::map<uint16_t, std::vector<uint16_t>> Indices;

        public:
            std::vector<std::shared_ptr<SmartNode>> ChildNodes;
            std::vector<std::shared_ptr<SmartLeaf>> Leaves;
        };
        std::shared_ptr<SmartNode> MainNode;

    private:
        [[nodiscard]] inline std::shared_ptr<SmartNode> ProcessNode(const BspFile::Node& node)
        {
            std::shared_ptr<SmartNode> smartNode = std::make_shared<SmartNode>();
            {
                Insert_Visual(smartNode, node);

                UpdateNode_ChildIndex(smartNode, node.ChildrenIndex[0]);
                UpdateNode_ChildIndex(smartNode, node.ChildrenIndex[1]);
            }
            return std::move(smartNode);
        }

        void ProcessFace(const BspFile::Face& face, std::map<uint16_t, std::vector<uint16_t>>& indicesMap);

        [[nodiscard]] std::shared_ptr<SmartLeaf> ProcessLeaf(const BspFile::Leaf& leaf);

        void UpdateNode_ChildIndex(const std::shared_ptr<SmartNode>& smartNode, int16_t childIndex);

        void Insert_Visual(const std::shared_ptr<SmartNode>& smartNode, const BspFile::Node& node);

    public:
        [[nodiscard]] inline std::map<uint16_t, std::vector<uint16_t>> FlattenIndices() const
        {
            std::map<uint16_t, std::vector<uint16_t>> indices = {};
            {
                FlattenIndices(MainNode, indices);
            }
            return indices;
        }
    private:
        static void FlattenIndices(const std::shared_ptr<SmartNode>& smartNode, std::map<uint16_t, std::vector<uint16_t>>& indices)
        {
            const std::map<uint16_t, std::vector<uint16_t>>& smartIndices = smartNode->Indices;
            for(const auto& it : smartIndices)
            {
                std::vector<uint16_t>& ind = indices[it.first];
                ind.insert(ind.end(), it.second.begin(), it.second.end());
            }

            for(const std::shared_ptr<SmartLeaf>& leaf : smartNode->Leaves)
                FlattenIndices(leaf, indices);

            for(const std::shared_ptr<SmartNode>& subNode : smartNode->ChildNodes)
                FlattenIndices(subNode, indices);
        }
        inline static void FlattenIndices(const std::shared_ptr<SmartLeaf>& smartLeaf, std::map<uint16_t, std::vector<uint16_t>>& indices)
        {
            const auto& smartIndices = smartLeaf->Indices;
            for(const auto& it : smartIndices)
            {
                std::vector<uint16_t>& ind = indices[it.first];
                ind.insert(ind.end(), it.second.begin(), it.second.end());
            }
        }

    public:
        void ExportFlatObj(const std::filesystem::path& filename);
    };
}
