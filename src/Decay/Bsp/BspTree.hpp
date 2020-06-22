#pragma once

#include <functional>

#include "BspFile.hpp"

// Enabling this adds vertices without duplicates.
// Takes more time to process but decreases Vertex size.
#define BSP_NO_DUPLICATES

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
            glm::vec2 ST;

        public:
            inline bool operator==(const Vertex& other) const
            {
                return Position == other.Position && UV == other.UV;
            }
            inline bool operator!=(const Vertex& other) const
            {
                return Position != other.Position || UV != other.UV;
            }
        };
        std::vector<Vertex> Vertices;

    public:
        class Model : std::enable_shared_from_this<Model>
        {
        public:
            Model(glm::vec3 bbMin, glm::vec3 bbMax, glm::vec3 origin)
             : BB_Min(bbMin), BB_Max(bbMax), Origin(origin)
            {
            }

        public:
            glm::vec3 BB_Min;
            glm::vec3 BB_Max;

            glm::vec3 Origin;

            /// [ textureIndex ] = vertex indices
            std::map<uint16_t, std::vector<uint16_t>> Indices;
        };
        std::vector<std::shared_ptr<Model>> Models;

    public:
        class Face
        {
        public:
            uint16_t TextureId;
            std::vector<uint16_t> Indices;

            /// [0] = Type of Light
            /// [1] = Base Light (0xFF = dark, 0x00 = light)
            /// [2],[3] = Additional light models
            uint8_t LightingStyles[4];
            /// Offsets into the raw LightMap data.
            /// Or -1 if none present.
            uint32_t LightmapOffset;
        };

    private:
        [[nodiscard]] std::shared_ptr<Model> ProcessModel(const BspFile::Model& model)
        {
            std::shared_ptr<Model> smartModel = std::make_shared<Model>(
                model.bbMin, model.bbMax,
                model.Origin
            );

            for(int32_t fi = model.FirstFaceIndex, fii = 0; fii < model.FaceCount; fi++, fii++)
            {
                BspFile::Face& face = Bsp->GetRawFaces()[fi];
                Face smartFace = ProcessFace(face);

                auto& indices = smartModel->Indices[smartFace.TextureId];
                assert(smartFace.Indices.size() % 3 == 0);
                indices.reserve(smartFace.Indices.size());
                std::copy(smartFace.Indices.begin(), smartFace.Indices.end(), back_inserter(indices));
            }

            return smartModel;
        }

        /// Call to add vertex while avoiding duplicates.
        /// Calling functions should still use `Vertices.reserve()` to make sure there is enough space when adding multiple vertices.
        [[nodiscard]] inline uint16_t AddVertex(const Vertex& vertex)
        {
#ifdef BSP_NO_DUPLICATES
            {
                auto it = std::find(Vertices.begin(), Vertices.end(), vertex);
                if(it != Vertices.end())
                    return it - Vertices.begin();
            }
#endif

            uint16_t index = Vertices.size();
            Vertices.emplace_back(vertex);
            return index;
        }

        Face ProcessFace(const BspFile::Face& face);

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
        /// Wavefront OBJ
        /// Text-based model format.
        void ExportFlatObj(const std::filesystem::path& filename) const;
        /// Wavefront OBJ - Materials
        /// Materials for OBJ.
        void ExportMtl(const std::filesystem::path& filename, const std::filesystem::path& texturePath = ".", const std::string& textureExtension = ".png") const;
        void ExportTextures(const std::filesystem::path& directory, const std::string& textureExtension = ".png", bool dummyForMissing = false) const;

    public:
        /// Extension always starts with period.
        /// Supported extensions:
        /// - .png
        /// - .bmp
        /// - .tga
        /// - .jpg / .jpeg (maximum quality
        /// - .raw (uint32_t width, uint32_t width, uint32_t... rgba_data)
        static std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec4* data)> GetImageWriteFunction(const std::string& extension);
    };
}
