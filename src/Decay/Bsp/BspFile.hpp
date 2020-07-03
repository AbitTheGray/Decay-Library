#pragma once

#include <filesystem>
#include <vector>
#include <map>
#include <glm/glm.hpp>

#include "../Common.hpp"
#include "../Wad/WadFile.hpp"

namespace Decay::Bsp
{
    class BspFile
    {
    public:
        explicit BspFile(const std::filesystem::path& filename);

        ~BspFile();

    public:
        enum class LumpType : uint8_t
        {
            Entities,
            Planes,
            Textures,
            Vertices,
            Visibility,
            Nodes,
            TextureMapping,
            Faces,
            Lighting,
            ClipNodes,
            Leaves,
            MarkSurface,
            Edges,
            SurfaceEdges,
            Models
        };
        static const std::size_t LumpType_Size = static_cast<uint8_t>(LumpType::Models) + 1;

    public:
        std::array<void*, LumpType_Size> m_Data = {};
        std::array<uint32_t, LumpType_Size> m_DataLength = {};
        static std::array<std::size_t, LumpType_Size> s_DataMaxLength;
        static std::array<std::size_t, LumpType_Size> s_DataElementSize;

    public:
        static const std::size_t MaxHulls = 4;

        static const std::size_t MaxModels = 400;
        static const std::size_t MaxBrushes = 4096;
        static const std::size_t MaxEntities = 1024;
        static const std::size_t MaxEntityString = 128 * 1024;

        static const std::size_t MaxPlanes = 32767;
        static const std::size_t MaxNodes = 32767;
        static const std::size_t MaxClipNodes = 32767;
        static const std::size_t MaxLeaves = 8192;
        static const std::size_t MaxVertices = 65535;
        static const std::size_t MaxFaces = 65535;
        static const std::size_t MaxMarkSurfaces = 65535;
        static const std::size_t MaxTextureMapping = 8192;
        static const std::size_t MaxEdges = 256000;
        static const std::size_t MaxSurfaceEdges = 512000;
        static const std::size_t MaxTextures = 512;
        static const std::size_t MaxMipTextures = 0x200000;
        static const std::size_t MaxLighting = 0x200000;
        static const std::size_t MaxVisibility = 0x200000;

        static const std::size_t MaxPortals = 65536;

    public:
        static const std::size_t Entities_MaxKey = 32;
        static const std::size_t Entities_MaxValue = 1024;

        enum class PlaneType : uint32_t
        {
            /// {1, 0, 0} or {-1, 0, 0}
            X = 0,
            /// {0, 1, 0} or {0, -1, 0}
            Y = 1,
            /// {0, 0, 1} or {0, 0, -1}
            Z = 2,
            /// Close to X axis than other axes
            AnyX = 3,
            /// Close to Y axis than other axes
            AnyY = 4,
            /// Close to Z axis than other axes
            AnyZ = 5,
        };
        struct Plane
        {
            /// Normalized vector.
            glm::vec3 Normal;
            float Distance;
            /// Used to speed-up rendering.
            PlaneType Type;
        };

        /// Including null terminator
        static const std::size_t MaxTextureName = 16;
        static const std::size_t MipTextureLevels = 4;
        struct Texture
        {
            /// Must be null-terminated.
            char Name[MaxTextureName];
            uint32_t Width, Height;
            /// MipMap offset relative to start of this struct.
            uint32_t MipMaps[MipTextureLevels];

            [[nodiscard]] inline std::string GetName() const { return Cstr2Str(Name, MaxTextureName); }

            [[nodiscard]] inline bool IsPacked() const
            {
                assert(MipMaps[0] == 0 || MipMaps[0] >= sizeof(Texture));
                assert(MipMaps[1] == 0 || MipMaps[1] > sizeof(Texture));
                assert(MipMaps[2] == 0 || MipMaps[2] > sizeof(Texture));
                assert(MipMaps[3] == 0 || MipMaps[3] > sizeof(Texture));

                return MipMaps[0] && MipMaps[1] && MipMaps[2] && MipMaps[3];
            }
        };

        struct Node
        {
            uint32_t PlaneIndex;
            /// If > 0, then indices into Nodes
            /// otherwise bitwise inverse indices into Leafs
            int16_t ChildrenIndex[2];
            /// Bounding Box
            glm::i16vec3 bbMin, bbMax;
            uint16_t FirstFaceIndex, FaceCount;
        };

        struct TextureMapping
        {
            glm::vec3 S;
            float SShift;

            glm::vec3 T;
            float TShift;

            /// Index into Textures array
            uint32_t Texture;
            /// Texture flags, seems to be always 0
            uint32_t TextureFlags;

        public:
            [[nodiscard]] inline float GetTexelS(const glm::vec3& position) const noexcept
            {
                return (S.x * position.x + S.y * position.y + S.z * position.z + SShift);
            }
            [[nodiscard]] inline float GetTexelT(const glm::vec3& position) const noexcept
            {
                return (T.x * position.x + T.y * position.y + T.z * position.z + TShift);
            }
            [[nodiscard]] inline float GetTexelU(const glm::vec3& position, const glm::u32vec2& textureSize) const noexcept
            {
                return GetTexelS(position) / textureSize.x;
            }
            [[nodiscard]] inline float GetTexelV(const glm::vec3& position, const glm::u32vec2& textureSize) const noexcept
            {
                return GetTexelT(position) / textureSize.y;
            }
        };

        struct Face
        {
            /// Plane the face is parallel to
            uint16_t Plane;
            /// Set if different normals orientation
            uint16_t PlaneSide;

            /// Index of the first Surface Edge
            uint32_t FirstSurfaceEdge;
            /// Number of consecutive Surface Edges
            uint16_t SurfaceEdgeCount;

            /// Index of the Texture Info structure
            uint16_t TextureMapping;

            uint8_t LightingStyles[4];
            /// Offsets into the raw LightMap data
            uint32_t LightmapOffset;
        };

        struct ClipNode
        {
            uint32_t PlaneIndex;
            /// If > 0, then indices into Nodes
            /// otherwise bitwise inverse indices into Leafs
            int16_t ChildrenIndex[2];
        };

        enum class LeafContent : int32_t
        {
            Empty = -1,
            Solid = -2,
            Water = -3,
            Slime = -4,
            Lava = -5,
            Sky = -6,
            Origin = -7,
            Clip = -8,
            Current_0 = -9,
            Current_90 = -10,
            Current_180 = -11,
            Current_270 = -12,
            Current_Up = -13,
            Current_Down = -14,
            Translucent = -15,
        };
        struct Leaf
        {
            LeafContent Content;
            /// Offset into the visibility lump
            /// or -1 if not exist
            int32_t VisOffset;
            /// Bounding Box
            glm::i16vec3 bbMin, bbMax;
            uint16_t FirstMarkSurface, MarkSurfaceCount;
            uint8_t AmbientSoundLevels[4];
        };

        typedef int16_t MarkSurface;

        struct Edge
        {
            uint16_t First;
            uint16_t Second;
        };

        typedef int32_t SurfaceEdges;

        struct Model
        {
            /// Bounding Box
            glm::vec3 bbMin, bbMax;
            glm::vec3 Origin;
            /// [0] = root node of Model's BSP tree (used for rendering)
            /// [1],[2] = BSP trees used for collision (at least the [1] one)
            /// [3] = 0
            int32_t Headnodes[4];
            /// Number of leaves
            int32_t VisLeafCount;
            /// Direct indexes into the faces array, not taking the redirecting by the marksurfaces.
            int32_t FirstFaceIndex, FaceCount;
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
        [[nodiscard]] std::size_t fun_count() const noexcept\
        {\
            return m_DataLength[static_cast<uint8_t>(LumpType::lumpType)] / sizeof(type);\
        }\
        [[nodiscard]] const type* fun_raw() const noexcept\
        {\
            return static_cast<type*>(m_Data[static_cast<uint8_t>(LumpType::lumpType)]);\
        }

        LUMP_ENTRY(GetEntityChars, GetEntityCharCount, GetRawEntityChars, char, Entities);
        LUMP_ENTRY(GetPlane, GetPlaneCount, GetRawPlanes, Plane, Planes);
        //TODO Textures
        LUMP_ENTRY(GetVertices, GetVertexCount, GetRawVertices, glm::vec3, Vertices);
        //TODO Visibility
        LUMP_ENTRY(GetNodes, GetNodeCount, GetRawNodes, Node, Nodes);
        LUMP_ENTRY(GetTextureMapping, GetTextureMappingCount, GetRawTextureMapping, TextureMapping, TextureMapping);
        LUMP_ENTRY(GetFaces, GetFaceCount, GetRawFaces, Face, Faces);
        //TODO Lighting
        LUMP_ENTRY(GetClipNodes, GetClipNodeCount, GetRawClipNodes, ClipNode, ClipNodes);
        LUMP_ENTRY(GetLeaves, GetLeafCount, GetRawLeaves, Leaf, Leaves);
        LUMP_ENTRY(GetMarkSurfaces, GetMarkSurfaceCount, GetRawMarkSurfaces, MarkSurface, MarkSurface);
        LUMP_ENTRY(GetEdges, GetEdgeCount, GetRawEdges, Edge, Edges);
        /// If the value of the surfedge is positive, the first vertex of the edge is used as vertex for rendering the face,
        /// otherwise, the value is multiplied by -1 and the second vertex of the indexed edge is used.
        LUMP_ENTRY(GetSurfaceEdges, GetSurfaceEdgeCount, GetRawSurfaceEdges, SurfaceEdges, SurfaceEdges);
        LUMP_ENTRY(GetModels, GetModelCount, GetRawModels, Model, Models);

        inline const Model& GetMainModel() const { return GetRawModels()[0]; }

    public:
        struct TextureParsed
        {
            static const std::size_t MaxNameLength = 16;
            std::string Name;
            uint32_t Width, Height;

            static const std::size_t MipMapLevels = 4;
            glm::u32vec2 MipMapDimensions[MipMapLevels] = {
                    {0, 0},
                    {0, 0},
                    {0, 0},
                    {0, 0}
            };
            std::vector<uint8_t> MipMapData[MipMapLevels];

            [[nodiscard]] bool HasData() const noexcept { return MipMapDimensions[0].x == 0; }

            static const std::size_t PaletteSize = 256;
            std::array<glm::u8vec3, PaletteSize> Palette;

        public:
            [[nodiscard]] inline std::vector<glm::u8vec3> AsRgb(std::size_t level = 0) const
            {
                assert(level < MipMapLevels);

                if(MipMapData[level].empty())
                    throw std::runtime_error("Texture does not contain data");

                std::vector<glm::u8vec3> pixels(MipMapData[level].size());
                for(std::size_t i = 0; i < MipMapData[level].size(); i++)
                    pixels[i] = Palette[MipMapData[level][i]];
                return pixels;
            }

            [[nodiscard]] inline std::vector<glm::u8vec4> AsRgba(std::size_t level = 0) const
            {
                assert(level < MipMapLevels);

                if(MipMapData[level].empty())
                    throw std::runtime_error("Texture does not contain data");

                bool paletteTransparent = Palette[255] == glm::u8vec3(0x00, 0x00, 0xFF);

                std::vector<glm::u8vec4> pixels(MipMapData[level].size());
                for(std::size_t i = 0; i < MipMapData[level].size(); i++)
                {
                    auto paletteIndex = MipMapData[level][i];

                    if(paletteIndex == 255 && paletteTransparent)
                        pixels[i] = glm::u8vec4(0x00u, 0x00u, 0xFFu, 0x00u); // Transparent
                    else
                        pixels[i] = glm::u8vec4(Palette[paletteIndex], 0xFFu); // Solid
                }
                return pixels;
            }

            void WriteRgbPng(const std::filesystem::path& filename, std::size_t level = 0) const;

            void WriteRgbaPng(const std::filesystem::path& filename, std::size_t level = 0) const;
        };

        [[nodiscard]] uint32_t GetTextureCount() const;

        [[nodiscard]] std::vector<Wad::WadFile::Texture> GetTextures() const;

    };
}
