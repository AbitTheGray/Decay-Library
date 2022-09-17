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
        struct Plane
        {
            static const int PlaneVertexCount = 3;
            glm::i32vec3 PlaneVertices[PlaneVertexCount];
            std::string Texture;

            glm::i32vec3 S;
            int32_t SOffset;

            glm::i32vec3 T;
            int32_t TOffset;

            int Rotation;

            glm::vec2 Scale;

            [[nodiscard]] inline bool operator==(const Plane& other) const
            {
                for(int i = 0; i < Plane::PlaneVertexCount; i++)
                    if(PlaneVertices[i] != other.PlaneVertices[i])
                        return false;

                if(Texture != other.Texture)
                    return false;

                if(S != other.S)
                    return false;
                if(SOffset != other.SOffset)
                    return false;

                if(T != other.T)
                    return false;
                if(TOffset != other.TOffset)
                    return false;

                if(Rotation != other.Rotation)
                    return false;
                if(Scale != other.Scale)
                    return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Plane& other) const { return !operator==(other); }
        };
        struct Brush
        {
            /// At least 4
            std::vector<Plane> Planes;
            //TODO More utility functions

            [[nodiscard]] inline bool operator==(const Brush& other) const
            {
                if(Planes.size() != other.Planes.size())
                    return false;
                for(int i = 0; i < Planes.size(); i++)
                    if(Planes[i] != other.Planes[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Brush& other) const { return !operator==(other); }
        };
        struct Entity
        {
            std::vector<Brush> Brushes;
            std::unordered_map<std::string, std::string> Values;
        };

        std::vector<Entity> Entities;

    public:
        MapFile() = default;
        explicit MapFile(std::istream& in);
        ~MapFile() = default;
    };

    std::istream& operator>>(std::istream& in, MapFile::Plane&);
    std::ostream& operator<<(std::ostream& out, const MapFile::Plane&);

    std::istream& operator>>(std::istream& in, MapFile::Brush&);
    std::ostream& operator<<(std::ostream& out, const MapFile::Brush&);

    std::istream& operator>>(std::istream& in, MapFile::Entity&);
    std::ostream& operator<<(std::ostream& out, const MapFile::Entity&);

    std::istream& operator>>(std::istream& in, MapFile&);
    std::ostream& operator<<(std::ostream& out, const MapFile&);
}