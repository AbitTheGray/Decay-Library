#include "MapFile.hpp"

#include "Decay/CommonReadUtils.hpp"

// Uncomment to output more human-friendly `*.map` files
#define MAP_TAB_OFFSET

// Parsing utils
namespace Decay::Map
{
    /// Read vector inside round brackets.
    void ReadPlaneVector(std::istream& in, glm::i32vec3& vec)
    {
        assert(in.good());
        int c;
        IgnoreWhitespace(in);

        c = in.peek();
        if(c != '(')
        {
            in.setstate(std::ios_base::failbit);
            return;
        }
        in.ignore(); // Skip '('

        IgnoreWhitespace(in);
        std::vector<char> num0 = ReadOnlyNumber(in, true);
        assert(!num0.empty());
        IgnoreWhitespace(in);
        std::vector<char> num1 = ReadOnlyNumber(in, true);
        assert(!num1.empty());
        IgnoreWhitespace(in);
        std::vector<char> num2 = ReadOnlyNumber(in, true);
        assert(!num2.empty());
        IgnoreWhitespace(in);

        c = in.peek();
        if(c != ')')
            throw std::runtime_error("Plane vertex must end by ')'");
        in.ignore(); // Skip ')'

        vec = {
            std::stoi(str(num0)),
            std::stoi(str(num1)),
            std::stoi(str(num2))
        };
    }
    /// Read vector+value inside square brackets
    void ReadTextureVector(std::istream& in, glm::i32vec3& vec, int32_t& val)
    {
        assert(in.good());
        int c;
        IgnoreWhitespace(in);

        c = in.peek();
        if(c != '[')
        {
            in.setstate(std::ios_base::failbit);
            return;
        }
        in.ignore(); // Skip '['

        IgnoreWhitespace(in);
        std::vector<char> num0 = ReadOnlyNumber(in, true);
        assert(!num0.empty());
        IgnoreWhitespace(in);
        std::vector<char> num1 = ReadOnlyNumber(in, true);
        assert(!num1.empty());
        IgnoreWhitespace(in);
        std::vector<char> num2 = ReadOnlyNumber(in, true);
        assert(!num2.empty());
        IgnoreWhitespace(in);
        std::vector<char> num3 = ReadOnlyNumber(in, true);
        assert(!num3.empty());
        IgnoreWhitespace(in);

        c = in.peek();
        if(c != ']')
            throw std::runtime_error("Texture vector must end by ']'");
        in.ignore(); // Skip ']'

        vec = {
            std::stoi(str(num0)),
            std::stoi(str(num1)),
            std::stoi(str(num2))
        };
        val = std::stoi(str(num3));
    }
}

namespace Decay::Map
{
    MapFile::MapFile(std::istream& in)
    {
        in >> *this;
        if(in.fail())
            throw std::runtime_error("Failed to read Map file");
    }
    /*
    std::vector<MapFile::Polygon<3, double>> MapFile::Brush::Polygons() const
    {
        typedef MapFile::Polygon<3, double> Polygon3d;
        std::vector<Polygon3d> polygons = {};

        throw std::runtime_error("Not Implemented"); //TODO
    }
    */
}

// Stream Operators
namespace Decay::Map
{
    std::istream& operator>>(std::istream& in, MapFile::Plane& plane)
    {
        assert(in.good());

        IgnoreWhitespace(in);

        if(in.peek() != '(')
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }
        for(int i = 0; i < MapFile::Plane::PlaneVertexCount; i++)
        {
            auto& pv = plane.PlaneVertices[i];
            ReadPlaneVector(in, pv);
            IgnoreWhitespace(in);
        }

        plane.Texture = str(ReadUntilWhitespaceOr(in, '[' /* Start of `S` */));

        IgnoreWhitespace(in);

        int c = in.peek();
        if(c == '[') // GoldSrc
        {
            ReadTextureVector(in, plane.UAxis, plane.UOffset);
            IgnoreWhitespace(in);
            ReadTextureVector(in, plane.VAxis, plane.VOffset);
        }
        else // IdTech2
        {
            std::vector<char> num;

            plane.UAxis = { 0, 0, 0};
            num = ReadOnlyNumber(in, true);
            plane.UOffset = std::stoi(str(num));

            IgnoreWhitespace(in);

            plane.VAxis = { 0, 0, 0};
            num = ReadOnlyNumber(in, true);
            plane.VOffset = std::stoi(str(num));
        }

        IgnoreWhitespace(in);

        plane.Rotation = std::stoi(str(ReadOnlyNumber(in, true)));

        IgnoreWhitespace(in);

        auto scaleX = ReadOnlyNumber(in, true, true);
        IgnoreWhitespace(in);
        auto scaleY = ReadOnlyNumber(in, true, true);

        plane.Scale = {
            std::stof(str(scaleX)),
            std::stof(str(scaleY))
        };

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const MapFile::Plane& plane)
    {
#ifdef MAP_TAB_OFFSET
        out << "\t\t";
#endif
        // cross((p3 - p1), (p2 - p1)) != null
        assert(glm::cross(glm::vec3(plane.PlaneVertices[2] - plane.PlaneVertices[0]), glm::vec3(plane.PlaneVertices[1] - plane.PlaneVertices[0])) != glm::vec3{});

        for(int i = 0; i < MapFile::Plane::PlaneVertexCount; i++)
        {
            auto& pv = plane.PlaneVertices[i];
            out << "( " << pv.x << ' ' << pv.y << ' ' << pv.z << " ) ";
        }

        assert(plane.Texture.find(' ') == std::string::npos);
        out << plane.Texture << ' ';

        out << "[ " << plane.UAxis.x << ' ' << plane.UAxis.y << ' ' << plane.UAxis.z << ' ' << plane.UOffset << " ] ";
        out << "[ " << plane.VAxis.x << ' ' << plane.VAxis.y << ' ' << plane.VAxis.z << ' ' << plane.VOffset << " ] ";

        out << plane.Rotation << ' ';

        out << plane.Scale.x << ' ' << plane.Scale.y;

        return out;
    }

    std::istream& operator>>(std::istream& in, MapFile::Brush& brush)
    {
        assert(in.good());

        IgnoreWhitespace(in);

        int c = in.peek();
        if(c != '{')
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }
        in.ignore(); // Skip '{'

        while(true)
        {
            IgnoreWhitespace(in);

            if(in.eof())
                throw std::runtime_error("End-of-File inside Brush");
            c = in.peek();
            if(c == EOF)
                throw std::runtime_error("End-of-File reached inside Brush");
            else if(c == '}') // End of brush
            {
                in.ignore(); // Skip '}'
                break;
            }
            else
            {
                assert(in.good());
                MapFile::Plane plane;
                in >> plane;
                if(in.fail())
                    throw std::runtime_error("Failed to read Brush Plane");
                brush.Planes.emplace_back(plane);
            }
        }
        return in;
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
        return out;
    }

    std::istream& operator>>(std::istream& in, MapFile::Entity& entity)
    {
        assert(in.good());

        IgnoreWhitespace(in);

        int c = in.peek();
        if(c != '{')
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }
        in.ignore(); // Skip '{'

        while(true)
        {
            IgnoreWhitespace(in);

            if(in.eof())
                throw std::runtime_error("End-of-File inside Brush");
            c = in.peek();
            if(c == EOF)
                throw std::runtime_error("End-of-File reached inside Brush");
            else if(c == '}') // End of entity
            {
                in.ignore(); // Skip '}'
                break;
            }
            else if(c == '{') // Start of brush
            {
                assert(in.good());
                MapFile::Brush brush;
                in >> brush;
                if(in.fail())
                    throw std::runtime_error("Failed to read Brush");
                entity.Brushes.emplace_back(brush);
            }
            else if(c == '\"') // Start of key-value
            {
                try
                {
                    std::vector<char> key = ReadQuotedString(in);
                    IgnoreWhitespace(in);
                    std::vector<char> value = ReadQuotedString(in);
                    entity.Values.emplace(str(key), str(value));
                }
                catch(std::exception& ex)
                {
                    throw std::runtime_error(std::string("Failed to read Map Entity key-value: ") + ex.what());
                }
            }
            else
            {
                throw std::runtime_error("Unexpected character inside Map Entity");
            }
        }
        return in;
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

    std::istream& operator>>(std::istream& in, MapFile& mapFile)
    {
        assert(in.good());

        while(true)
        {
            IgnoreWhitespace(in);
            if(in.eof())
                return in;

            int c = in.peek();
            if(c == EOF)
                return in;
            else if(c == '{')
            {
                assert(in.good());
                MapFile::Entity entity;
                in >> entity;
                if(in.fail())
                    throw std::runtime_error("Failed to read Map Entity");
                mapFile.Entities.emplace_back(entity);
                continue;
            }
            else
                return in;
        }
    }
    std::ostream& operator<<(std::ostream& out, const MapFile& mapFile)
    {
        for(const auto& entity : mapFile.Entities)
            out << entity << '\n';
        return out;
    }
}
