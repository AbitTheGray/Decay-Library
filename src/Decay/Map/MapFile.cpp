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
        R_ASSERT(in.good(), "Input stream is not in a good shape");
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
        R_ASSERT(!num0.empty(), "1st number of vector cannot be empty");

        IgnoreWhitespace(in);

        std::vector<char> num1 = ReadOnlyNumber(in, true);
        R_ASSERT(!num1.empty(), "2nd number of vector cannot be empty");

        IgnoreWhitespace(in);

        std::vector<char> num2 = ReadOnlyNumber(in, true);
        R_ASSERT(!num2.empty(), "3rd number of vector cannot be empty");

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
    /// Read vector+value inside square brackets.
    /// The float value is optional.
    void ReadTextureVector(std::istream& in, glm::vec3& vec, float& val)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");
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

        std::vector<char> num0 = ReadOnlyNumber(in, true, true);
        R_ASSERT(!num0.empty(), "1st number of vector cannot be empty");

        IgnoreWhitespace(in);

        std::vector<char> num1 = ReadOnlyNumber(in, true, true);
        R_ASSERT(!num1.empty(), "2nd number of vector cannot be empty");

        IgnoreWhitespace(in);

        std::vector<char> num2 = ReadOnlyNumber(in, true, true);
        R_ASSERT(!num2.empty(), "3rd number of vector cannot be empty");

        IgnoreWhitespace(in);

        std::vector<char> num3 = ReadOnlyNumber(in, true, true);
        IgnoreWhitespace(in);

        c = in.peek();
        if(c != ']')
            throw std::runtime_error("Raw_Texture vector must end by ']'");
        in.ignore(); // Skip ']'

        vec = {
            std::stof(str(num0)),
            std::stof(str(num1)),
            std::stof(str(num2))
        };
        if(!num3.empty())
            val = std::stof(str(num3));
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
    std::istream& operator>>(std::istream& in, MapFile::Face& plane)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        IgnoreWhitespace(in);

        if(in.peek() != '(')
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }
        for(int i = 0; i < MapFile::Face::PlaneVertexCount; i++)
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
            num = ReadOnlyNumber(in, true, true);
            plane.UOffset = std::stof(str(num));

            IgnoreWhitespace(in);

            plane.VAxis = { 0, 0, 0};
            num = ReadOnlyNumber(in, true, true);
            plane.VOffset = std::stof(str(num));
        }

        IgnoreWhitespace(in);

        plane.Rotation = std::stof(str(ReadOnlyNumber(in, true, true)));

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
    void MapFile::Face::Write(std::ostream& out, EngineVariant variant) const
    {
        out << std::noshowpoint;
#ifdef MAP_TAB_OFFSET
        out << "\t\t";
#endif
        // cross((p3 - p1), (p2 - p1)) != null
        R_ASSERT(glm::cross(glm::vec3(PlaneVertices[2] - PlaneVertices[0]), glm::vec3(PlaneVertices[1] - PlaneVertices[0])) != glm::vec3{}, "Vertices defining MAP Face's plane cannot be on same line");

        for(int i = 0; i < MapFile::Face::PlaneVertexCount; i++)
        {
            auto& pv = PlaneVertices[i];
            out << "( " << pv.x << ' ' << pv.y << ' ' << pv.z << " ) ";
        }

        R_ASSERT(Texture.find(' ') == std::string::npos, "Raw_Texture name cannot contain a space character");
        out << Texture << ' ';

        switch(variant)
        {
            case EngineVariant::GoldSrc:
                out << "[ " << UAxis.x << ' ' << UAxis.y << ' ' << UAxis.z << ' ' << UOffset << " ] ";
                out << "[ " << VAxis.x << ' ' << VAxis.y << ' ' << VAxis.z << ' ' << VOffset << " ] ";
                break;
            case EngineVariant::IdTech2:
                //TODO Verify offset order (UAxis and VAxis can potentially swap those)
                out << UOffset << ' ';
                out << VOffset << ' ';
                break;
            default:
                throw std::runtime_error("Invalid engine variant");
        }

        out << Rotation << ' ';

        out << Scale.x << ' ' << Scale.y;
    }

    std::istream& operator>>(std::istream& in, MapFile::Brush& brush)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

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
                R_ASSERT(in.good(), "Input stream is not in a good shape");
                MapFile::Face plane;
                in >> plane;
                if(in.fail())
                    throw std::runtime_error("Failed to read Brush Face");
                brush.Faces.emplace_back(plane);
            }
        }
        return in;
    }
    void MapFile::Brush::Write(std::ostream& out, EngineVariant variant) const
    {
#ifdef MAP_TAB_OFFSET
        out << '\t';
#endif
        out << "{\n";
        {
            R_ASSERT(Faces.size() >= 4, "MAP Brush must contain at least 4 faces to form 3D object");
            for(const MapFile::Face& face : Faces)
            {
                face.Write(out, variant);
                out << '\n';
            }
        }
#ifdef MAP_TAB_OFFSET
        out << '\t';
#endif
        out << '}';
    }

    std::istream& operator>>(std::istream& in, MapFile::Entity& entity)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

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
                R_ASSERT(in.good(), "Input stream is not in a good shape");
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
    void MapFile::Entity::Write(std::ostream& out, EngineVariant variant) const
    {
        out << "{\n";
        for(const auto& kv : Values)
        {
#ifdef MAP_TAB_OFFSET
            out << '\t';
#endif
            R_ASSERT(!kv.first.empty(), "Key cannot be empty");
            R_ASSERT(kv.first.find('\"') == std::string::npos, "Key must not contain '\"' character");
            R_ASSERT(kv.second.find('\"') == std::string::npos, "value must not contain '\"' character");
            out << '\"' << kv.first << "\" \"" << kv.second << "\"\n";
        }
        for(const auto& brush : Brushes)
        {
            brush.Write(out, variant);
            out << '\n';
        }
        out << "}";
    }

    std::istream& operator>>(std::istream& in, MapFile& mapFile)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

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
                R_ASSERT(in.good(), "Input stream is not in a good shape");
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
    void MapFile::Write(std::ostream& out, EngineVariant variant) const
    {
        for(const auto& entity : Entities)
        {
            entity.Write(out, variant);
            out << '\n';
        }
    }
}
