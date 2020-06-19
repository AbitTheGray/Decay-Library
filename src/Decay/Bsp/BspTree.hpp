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

        class SmartModel : std::enable_shared_from_this<SmartModel>
        {
        public:
            SmartModel(glm::vec3 bbMin, glm::vec3 bbMax, glm::vec3 origin)
             : BB_Min(bbMin), BB_Max(bbMax), Origin(origin)
            {
            }

        public:
            glm::vec3 BB_Min;
            glm::vec3 BB_Max;

            glm::vec3 Origin;

            std::shared_ptr<SmartNode> Node;

            /// [ textureIndex ] = vertex indices
            std::map<uint16_t, std::vector<uint16_t>> Indices;
        };
        std::vector<std::shared_ptr<SmartModel>> Models;

    private:
        [[nodiscard]] inline std::vector<std::shared_ptr<SmartModel>> ProcessModels()
        {
            std::vector<std::shared_ptr<SmartModel>> models(Bsp->GetModelCount());
            for(std::size_t mi = 0; mi < models.size(); mi++)
            {
                BspFile::Model& model = Bsp->GetRawModels()[mi];
                models[mi] = ProcessModel(model);
            }
            return models;
        }
        [[nodiscard]] std::shared_ptr<SmartModel> ProcessModel(const BspFile::Model& model)
        {
            std::shared_ptr<SmartModel> smartModel = std::make_shared<SmartModel>(
                model.bbMin, model.bbMax,
                model.Origin
            );

            //smartModel->Node = ProcessNode(Bsp->GetRawNodes()[model.Headnodes[0]]);

            for(int32_t fi = model.FirstFaceIndex, fii = 0; fii < model.FaceCount; fi++, fii++)
            {
                BspFile::Face& face = Bsp->GetRawFaces()[fi];
                ProcessFace(face, smartModel->Indices);
            }

            return smartModel;
        }

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
        [[nodiscard]] inline std::map<uint16_t, std::vector<uint16_t>> FlattenIndices_Models() const
        {
            std::map<uint16_t, std::vector<uint16_t>> indices = {};

            for(auto& model : Models)
            {
                for(auto& kvp : model->Indices)
                {
                    std::vector<uint16_t>& ind = indices[kvp.first];
                    ind.reserve(kvp.second.size());

                    std::copy(kvp.second.begin(), kvp.second.end(), back_inserter(ind));
                }
            }

            return indices;
        }

    public:
        [[nodiscard]] inline std::map<uint16_t, std::vector<uint16_t>> FlattenIndices_Nodes() const
        {
            std::map<uint16_t, std::vector<uint16_t>> indices = {};

            for(auto& model : Models)
                FlattenIndices_Node(model->Node, indices);

            return indices;
        }
    private:
        static void FlattenIndices_Node(const std::shared_ptr<SmartNode>& smartNode, std::map<uint16_t, std::vector<uint16_t>>& indices)
        {
            const std::map<uint16_t, std::vector<uint16_t>>& smartIndices = smartNode->Indices;
            for(const auto& it : smartIndices)
            {
                std::vector<uint16_t>& ind = indices[it.first];
                ind.insert(ind.end(), it.second.begin(), it.second.end());
            }

            for(const std::shared_ptr<SmartLeaf>& leaf : smartNode->Leaves)
                FlattenIndices_Leaf(leaf, indices);

            for(const std::shared_ptr<SmartNode>& subNode : smartNode->ChildNodes)
                FlattenIndices_Node(subNode, indices);
        }
        inline static void FlattenIndices_Leaf(const std::shared_ptr<SmartLeaf>& smartLeaf, std::map<uint16_t, std::vector<uint16_t>>& indices)
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
