#pragma once

#include <filesystem>
#include <vector>
#include <glm/glm.hpp>

namespace Decay::Bsp
{
    class BspParser
    {
    public:
        explicit BspParser(const std::filesystem::path& filename);

        ~BspParser();

    public:
        enum class LumpType : uint8_t
        {
            Entities,
            Planes,
            Textures,
            Vertices,
            Visibility,
            Nodes,
            TextureInfo,
            Faces,
            Lighting,
            ClipNodes,
            Leaves,
            MarkSurface,
            Edges,
            SurfaceEdges,
            Models
        };
        static const std::size_t LumpType_Size = (uint8_t)LumpType::Models + 1;

    public:
        std::array<void*, LumpType_Size> m_Data = {};
        std::array<uint32_t , LumpType_Size> m_DataLength = {};

    public:
        struct Face
        {
            /// Plane the face is parallel to
            uint16_t Plane;
            /// Set if different normals orientation
            uint16_t PlaneSide;

            /// Index of the first Surface Edge
            uint32_t FirstEdge;
            /// Number of consecutive Surface Edges
            uint16_t EdgeCount;

            /// Index of the Texture Info structure
            uint16_t TextureMapping;

            uint8_t LightingStyles[4];
            /// Offsets into the raw LightMap data
            uint32_t LightmapOffset;
        };

        struct Edge
        {
            uint16_t First;
            uint16_t Second;
        };

#define LUMP_ENTRY(fun_vec, fun_count, fun_raw, type, lumpType) \
        [[nodiscard]] std::vector<type> fun_vec() const\
        {\
            std::size_t index = static_cast<std::size_t>(LumpType::lumpType);\
        std::vector<type> rtn(m_DataLength[index]);\
        std::copy(\
        static_cast<char*>(m_Data[index]),\
        static_cast<char*>(m_Data[index]) + m_DataLength[index],\
        static_cast<char*>(static_cast<void*>(rtn.data()))\
        );\
        return rtn;\
        }\
        [[nodiscard]] std::size_t fun_count() const\
        {\
            return m_DataLength[static_cast<uint8_t>(LumpType::lumpType)] / sizeof(type);\
        }\
        [[nodiscard]] type* fun_raw() const\
        {\
            return static_cast<type*>(m_Data[static_cast<uint8_t>(LumpType::lumpType)]);\
        }

        LUMP_ENTRY(GetVertices, GetVertexCount, GetRawVertices, glm::vec3, Vertices);
        LUMP_ENTRY(GetFaces, GetFaceCount, GetRawFaces, Face, Faces);
        LUMP_ENTRY(GetEdges, GetEdgeCount, GetRawEdges, Edge, Edges);

    };
}
