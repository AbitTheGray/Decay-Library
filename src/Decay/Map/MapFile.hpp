#pragma once

#include "Decay/Common.hpp"
#include "unordered_map"

namespace Decay::Map
{
    /// https://quakewiki.org/wiki/Quake_Map_Format
    /// https://www.gamers.org/dEngine/quake2/Q2DP/Q2DP_Map/Q2DP_Map-2.html
    class MapFile
    {
    public:
        enum class EngineVariant
        {
            IdTech2 = 0,
            GoldSrc = 1
        };
    public:
        struct Face
        {
            static const int PlaneVertexCount = 3;
            glm::i32vec3 PlaneVertices[PlaneVertexCount]{};
            std::string Texture{};

            glm::vec3 UAxis;
            float UOffset = 0;

            glm::vec3 VAxis;
            float VOffset = 0;

            float Rotation = 0;

            glm::vec2 Scale = { 1, 1 };

            /// Calculates normal vector for the plane.
            /// Can be {0, 0, 0} for invalid planes.
            /// cross((p2 - p0), (p1 - p0))
            [[nodiscard]] inline glm::dvec3 Normal() const noexcept { return glm::normalize(glm::cross(glm::dvec3(PlaneVertices[2] - PlaneVertices[0]), glm::dvec3(PlaneVertices[1] - PlaneVertices[0]))); }
            /// Calculates distance of the plane from origin
            [[nodiscard]] inline double DistanceFromOrigin() const noexcept
            {
                auto normal = Normal();
                double d0 = (normal.x * PlaneVertices[0].x) + (normal.y * PlaneVertices[0].y) + (normal.z * PlaneVertices[0].z);
#ifdef DEBUG
                double d1 = (normal.x * PlaneVertices[1].x) + (normal.y * PlaneVertices[1].y) + (normal.z * PlaneVertices[1].z);
                double d2 = (normal.x * PlaneVertices[2].x) + (normal.y * PlaneVertices[2].y) + (normal.z * PlaneVertices[2].z);
                D_ASSERT(abs(d0 - d1) < 0.001);
                D_ASSERT(abs(d0 - d2) < 0.001);
                D_ASSERT(abs(d1 - d2) < 0.001);
#endif
                return d0;
            }

            [[nodiscard]] inline bool operator==(const Face& other) const
            {
                for(int i = 0; i < Face::PlaneVertexCount; i++)
                    if(PlaneVertices[i] != other.PlaneVertices[i])
                        return false;

                if(Texture != other.Texture)
                    return false;

                if(UAxis != other.UAxis)
                    return false;
                if(UOffset != other.UOffset)
                    return false;

                if(VAxis != other.VAxis)
                    return false;
                if(VOffset != other.VOffset)
                    return false;

                if(Rotation != other.Rotation)
                    return false;
                if(Scale != other.Scale)
                    return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Face& other) const { return !operator==(other); }

            void Write(std::ostream&, EngineVariant) const;
        };
        template<std::size_t N, typename T>
        struct Polygon
        {
            std::vector<glm::vec<N, T, glm::defaultp>> Vertices;
        };
        struct Brush
        {
            /// At least 4 planes creating convex object.
            std::vector<Face> Faces{};

            //TODO More utility functions
            [[nodiscard]] std::vector<Polygon<3, double>> Polygons() const;

            [[nodiscard]] inline bool operator==(const Brush& other) const
            {
                if(Faces.size() != other.Faces.size())
                    return false;
                for(int i = 0; i < Faces.size(); i++)
                    if(Faces[i] != other.Faces[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Brush& other) const { return !operator==(other); }

            void Write(std::ostream&, EngineVariant) const;
        };
        struct Entity
        {
            std::unordered_map<std::string, std::string> Values{};
            std::vector<Brush> Brushes{};

            void Write(std::ostream&, EngineVariant) const;
        };

    public:
        std::vector<Entity> Entities{};

    public:
        MapFile() = default;
        explicit MapFile(std::istream& in);
        ~MapFile() = default;

    public:
        void Write(std::ostream&, EngineVariant) const;
    };

    std::istream& operator>>(std::istream& in, MapFile::Face&);

    std::istream& operator>>(std::istream& in, MapFile::Brush&);

    std::istream& operator>>(std::istream& in, MapFile::Entity&);

    std::istream& operator>>(std::istream& in, MapFile&);
}
