#include "MapFile.hpp"

#define MAP_TAB_OFFSET

namespace Decay::Map
{
    std::istream& operator>>(std::istream& in, MapFile::Plane&)
    {
        throw std::runtime_error("Not Implemented"); //TODO
    }
    std::ostream& operator<<(std::ostream& out, const MapFile::Plane& plane)
    {
#ifdef MAP_TAB_OFFSET
        out << "\t\t";
#endif
        for(const auto& pv : plane.PlaneVertices)
            out << "( " << pv.x << ' ' << pv.y << ' ' << pv.z << " ) ";

        assert(plane.Texture.find(' ') == std::string::npos);
        out << plane.Texture << ' ';

        out << "[ " << plane.S.x << ' ' << plane.S.y << ' ' << plane.S.z << ' ' << plane.SOffset << " ] ";
        out << "[ " << plane.T.x << ' ' << plane.T.y << ' ' << plane.T.z << ' ' << plane.TOffset << " ] ";

        out << plane.Rotation << ' ';

        out << plane.Scale.x << ' ' << plane.Scale.y;

        return out;
    }
    std::istream& operator>>(std::istream& in, MapFile::Brush&)
    {
        throw std::runtime_error("Not Implemented"); //TODO
    }
    std::ostream& operator<<(std::ostream& out, const MapFile::Brush& brush)
    {
#ifdef MAP_TAB_OFFSET
        out << '\t';
#endif
        out << "{\n";
        {
            assert(brush.Planes.size() >= 4);
            for(const MapFile::Plane& plane : brush.Planes)
                out << plane << '\n';
        }
#ifdef MAP_TAB_OFFSET
        out << '\t';
#endif
        out << '}';
    }
    std::istream& operator>>(std::istream& in, MapFile::Entity&)
    {
        throw std::runtime_error("Not Implemented"); //TODO
    }
    std::ostream& operator<<(std::ostream& out, const MapFile::Entity& entity)
    {
        out << "{\n";
        for(const auto& kv : entity.Values)
        {
#ifdef MAP_TAB_OFFSET
            out << '\t';
#endif
            assert(!kv.first.empty());
            assert(kv.first.find('\"') == std::string::npos);
            assert(kv.second.find('\"') == std::string::npos);
            out << '\"' << kv.first << "\" \"" << kv.second << "\"\n";
        }
        for(const auto& brush : entity.Brushes)
            out << brush << '\n';
        out << "}";
        return out;
    }
    std::istream& operator>>(std::istream& in, MapFile&)
    {
        throw std::runtime_error("Not Implemented"); //TODO
    }
    std::ostream& operator<<(std::ostream& out, const MapFile& mapFile)
    {
        for(const auto& entity : mapFile.Entities)
            out << entity << '\n';
        return out;
    }
}
