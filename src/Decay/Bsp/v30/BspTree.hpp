#pragma once

#include <functional>

#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspEntities.hpp"

namespace Decay::Bsp::v30
{
    class BspTree
    {
    public:
        explicit BspTree(std::shared_ptr<BspFile> bsp);

    public:
        const std::shared_ptr<BspFile> Bsp;
        const std::vector<Wad::Wad3::WadFile::Texture> Textures;

        const BspEntities Entities;

    public:
        struct Vertex
        {
            /// World position
            glm::vec3 Position;

            /// Texture coordinates
#ifdef DECAY_BSP_ST_INSTEAD_OF_UV
            /// 0.0 to texture size
            glm::vec2 ST;
#else
            /// 0.0 to 1.0
            glm::vec2 UV;
#endif

            /// Lightmap coordinates
#ifdef DECAY_BSP_LIGHTMAP_ST_INSTEAD_OF_UV
            /// 0.0 to texture size
            glm::vec2 LightST;
#else
            /// 0.0 to 1.0
            glm::vec2 LightUV;
#endif

        public:
            inline bool operator==(const Vertex& other) const
            {
#ifdef DECAY_BSP_ST_INSTEAD_OF_UV
                return Position == other.Position && ST == other.ST;
#else
                return Position == other.Position && UV == other.UV;
#endif
            }
            inline bool operator!=(const Vertex& other) const
            {
#ifdef DECAY_BSP_ST_INSTEAD_OF_UV
                return Position != other.Position || ST != other.ST;
#else
                return Position != other.Position || UV != other.UV;
#endif
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
        class Lightmap
        {
        public:
            static const uint32_t InitSize = 512; // Used to be 256 but most official map had 3 or 4 of them
            static const uint32_t MaxSize = 1u << 12u; // 4096

#ifdef DEBUG
            static constexpr glm::u8vec3 InitColor = glm::u8vec3(0xFF, 0x00, 0x7F); // Pink
#else
            static constexpr glm::u8vec3 InitColor = glm::u8vec3(0x00, 0x00, 0x00); // Black
#endif

        public:
            explicit Lightmap(uint32_t width = InitSize, uint32_t height = InitSize)
             : Width(width), Height(height),
               Data(width * height, InitColor),
               Used(width * height), UsedRanges()
            {
                assert(Height / 2 - 16 >= 0);
                assert(Width / 2 - 16 >= 0);
            }

        public:
            uint32_t Width, Height;
            std::vector<glm::u8vec3> Data;

        public:
            std::vector<bool> Used;
            std::vector<glm::u32vec4> UsedRanges;

        public:
            bool CanInsert(uint32_t insert_x, uint32_t insert_y, glm::uvec2 size)
            {
                if(size.x + insert_x >= Width) [[unlikely]]
                    return false;
                if(size.y + insert_y >= Height) [[unlikely]]
                    return false;

                for(uint32_t y = 0; y < size.y; y++)
                {
                    std::size_t yi = (y + insert_y) * Width;
                    for(uint32_t x = 0; x < size.x; x++)
                    {
                        if(Used[yi + x + insert_x])
                            return false;
                    }
                }

                return true;
            }
            void Insert(uint32_t insert_x, uint32_t insert_y, glm::uvec2 size, const glm::u8vec3* data)
            {
                // Already checked by `CanInsert`, no need to check here (in Release)
                assert(size.x + insert_x < Width);
                assert(size.y + insert_y < Height);

                // Mark range as used
                UsedRanges.emplace_back(
                        insert_x, insert_y,
                        size.x, size.y
                );

                for(uint32_t y = 0; y < size.y; y++)
                {
                    std::size_t yi = (y + insert_y) * Width;
                    std::size_t insert_yi = y * size.x;

                    // Mark pixels as "Used"
                    std::fill(
                            Used.begin() + (yi + insert_x),
                            Used.begin() + ((yi + insert_x) + size.x),
                            true
                    );

                    // Copy pixel data
                    std::copy(
                            data + insert_yi,
                            data + (insert_yi + size.x),
                            Data.data() + (yi + insert_x)
                    );
                }
            }
        };
        Lightmap Light = Lightmap();
    public:
        void AddLight(glm::uvec2 size, const glm::u8vec3* data, glm::vec2& out_start, glm::vec2& out_end);

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
                const BspFile::Face& face = Bsp->GetRawFaces()[fi];
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
        void ExportFlatObj(const std::filesystem::path& filename, const std::filesystem::path& mtlFilename = {}) const;
        /// Wavefront OBJ - Materials
        /// Materials for OBJ.
        void ExportMtl(const std::filesystem::path& filename, const std::filesystem::path& texturePath = ".", const std::string& textureExtension = ".png") const;
        void ExportTextures(const std::filesystem::path& directory, const std::string& textureExtension = ".png", bool dummyForMissing = false) const;

    public:
        inline static float GetLightStyle_Char(char c)
        {
            if(c <= 'a')
                return 0;
            if(c >= 'z')
                return 1;
            return static_cast<float>(c - 'a') / static_cast<float>('z' - 'a' + 1);
        }
        static float GetLightStyle(const std::string& sequence, uint64_t time_ms)
        {
            if(sequence.length() == 0)
                throw std::runtime_error("Empty light sequence");
            if(sequence.length() == 1)
                return GetLightStyle_Char(sequence[0]);

            return GetLightStyle_Char(sequence[(time_ms / 10) % sequence.length()]);
        }
        static float GetLightStyle(uint8_t style, uint64_t time_ms) noexcept
        {
            switch(style)
            {
                // Normal
                default:
                case 0:
                    return GetLightStyle("m", time_ms);

                // Fluorescent flicker
                case 10:
                    return GetLightStyle("mmamammmmammamamaaamammma", time_ms);

                // Slow, strong pulse
                case 2:
                    return GetLightStyle("abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba", time_ms);

                // Slow pulse, noblack
                case 11:
                    return GetLightStyle("abcdefghijklmnopqrrqponmlkjihgfedcba", time_ms);

                // Gentle pulse
                case 5:
                    return GetLightStyle("jklmnopqrstuvwxyzyxwvutsrqponmlkj", time_ms);

                // Flicker A
                case 1:
                    return GetLightStyle("mmnmmommommnonmmonqnmmo", time_ms);
                // Flicker B
                case 6:
                    return GetLightStyle("nmonqnmomnmomomno", time_ms);

                // Candle A
                case 3:
                    return GetLightStyle("mmmmmaaaaammmmmaaaaaabcdefgabcdefg", time_ms);
                // Candle B
                case 7:
                    return GetLightStyle("mmmaaaabcdefgmmmmaaaammmaamm", time_ms);
                // Candle C
                case 8:
                    return GetLightStyle("mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa", time_ms);

                // Fast strobe
                case 4:
                    return GetLightStyle("mamamamamama", time_ms);
                // Slow strobe
                case 9:
                    return GetLightStyle("aaaaaaaazzzzzzzz", time_ms);
                // Underwater light mutation
                case 12:
                    return GetLightStyle("mmnnmmnnnmmnn", time_ms);
            }
        }
    };
}
