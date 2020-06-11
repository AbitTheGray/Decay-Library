#pragma once

#include "BspFile.hpp"

namespace Decay::Bsp
{
    class BspTree
    {
    public:
        BspTree(std::shared_ptr<BspFile> bsp);

    public:
        std::shared_ptr<BspFile> Bsp;
        std::vector<Wad::WadFile::Texture> Textures;

    public:
        struct Vertex
        {
            glm::vec3 Position;
            glm::vec2 UV;
        };
        std::vector<Vertex> Vertices;

    public:
        class SmartNode : std::enable_shared_from_this<SmartNode>
        {
        public:
            /// [ textureIndex ] = vertex indices
            std::map<uint16_t, std::vector<uint16_t>> Indices;

        public:
            std::vector<std::shared_ptr<SmartNode>> ChildNodes;
            //std::vector<std::shared_ptr<Leaf>> Leaves;
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

        void UpdateNode_ChildIndex(const std::shared_ptr<SmartNode>& smartNode, int16_t childIndex);

        void Insert_Visual(const std::shared_ptr<SmartNode>& smartNode, const BspFile::Node& node);
    };
}
