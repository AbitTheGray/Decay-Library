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
            glm::i32vec3 PlaneVertices[3];
            std::string Texture;

            glm::i32vec3 S;
            int32_t SOffset;

            glm::i32vec3 T;
            int32_t TOffset;

            int Rotation;

            glm::vec2 Scale;
        };
        struct Brush
        {
            /// At least 4
            std::vector<Plane> Planes;
            //TODO More utility functions
        };
        struct Entity
        {
            std::vector<Brush> Brushes;
            std::unordered_map<std::string, std::string> Values;
        };

        std::vector<Entity> Entities;
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
