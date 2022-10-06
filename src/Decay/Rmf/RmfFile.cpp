#include "RmfFile.hpp"

// Parsing utils
namespace Decay::Rmf
{
    inline bool IsNullTerminated(const char* text, int maxLength)
    {
        for(int i = 0; i < maxLength; i++)
        {
            if(text[i] == '\0')
                return true;
        }
        return false;
    }

    /// Read length-prefixed string
    inline std::string ReadNString(std::istream& in, int maxLength = 0 /* no limit */)
    {
        R_ASSERT(maxLength >= 0, "Maximum length cannot be negative");

        uint8_t length = 0;
        in.read(reinterpret_cast<char*>(&length), sizeof(length));

        if(maxLength > 0 && length > maxLength)
            throw std::runtime_error("Failed to read NString - length is above allowed value");

        if(length > 1) // Contains data
        {
            std::string rtn = std::string(length - 1, ' ');
            in.read(rtn.data(), length - 1);
            R_ASSERT(in.get() == '\0', "length-prefixed string must end by NULL character");
            return rtn;
        }
        else if(length == 1) // 1 = only null-termination
        {
            R_ASSERT(in.get() == '\0', "length-prefixed string must end by NULL character");
            return std::string{};
        }
        else // length == 0
        {
            return std::string{};
        }
    }
    /// Write length-prefixed string
    inline void WriteNString(std::ostream& out, const std::string& text)
    {
        R_ASSERT(text.size() <= std::numeric_limits<uint8_t>::max(), "length-prefixed string is too long");
        uint8_t length = text.size() + 1;
        out.write(reinterpret_cast<const char*>(&length), sizeof(length));

        out.write(text.data(), text.size());

        uint8_t byte = 0;
        out.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
    }
    inline void ProcessGroupIntoEntityAndBrush(std::vector<Map::MapFile::Entity>& entities, Map::MapFile::Entity& worldspawn, const RmfFile::Group& group)
    {
        entities.reserve(group.Entities.size());
        for(const auto& entity : group.Entities)
            entities.emplace_back(entity);

        worldspawn.Brushes.reserve(group.Brushes.size());
        for(const auto& brush : group.Brushes)
            worldspawn.Brushes.emplace_back(brush);

        for(const auto& subGroup : group.Groups)
            ProcessGroupIntoEntityAndBrush(entities, worldspawn, group);
    }
}

namespace Decay::Rmf
{
    RmfFile::RmfFile(std::istream& in)
    {
        in >> *this;
        if(in.fail())
            throw std::runtime_error("Failed to read Rmf file");
    }
    RmfFile::RmfFile(const Map::MapFile& mapFile)
    {
        for(const auto& entity : mapFile.Entities)
        {
            const auto it_classname = entity.Values.find("classname");
            if(it_classname == entity.Values.end() || it_classname->second.empty())
                throw std::runtime_error("Entity in map must have a classname");
            const auto& classname = it_classname->second;

            if(classname == "worldspawn")
            {
                WorldInfo.Brushes.reserve(entity.Brushes.size());
                for(const auto& brush : entity.Brushes)
                    WorldInfo.Brushes.emplace_back(brush);

                for(const auto& kv : entity.Values)
                    WorldInfo.Values.emplace(kv);
            }
            else
            {
                WorldInfo.Entities.emplace_back(entity);
            }
        }
    }
    RmfFile::operator Map::MapFile() const
    {
        Map::MapFile mapFile{};

        mapFile.Entities.reserve(WorldInfo.Entities.size());

        Map::MapFile::Entity worldspawn = Map::MapFile::Entity{
            WorldInfo.Values,
            {}
        };
        worldspawn.Brushes.reserve(WorldInfo.Brushes.size());
        for(const auto& brush : WorldInfo.Brushes)
            worldspawn.Brushes.emplace_back(brush);

        for(const auto& entity : WorldInfo.Entities)
            mapFile.Entities.emplace_back(entity);

        for(const auto& group : WorldInfo.Groups)
            ProcessGroupIntoEntityAndBrush(mapFile.Entities, worldspawn, group);

        mapFile.Entities.insert(mapFile.Entities.begin(), std::move(worldspawn));
        return mapFile;
    }
}

// Stream Operators
namespace Decay::Rmf
{
    std::istream& operator>>(std::istream& in, RmfFile::VisGroup& visGroup)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        in.read(reinterpret_cast<char*>(&visGroup), sizeof(RmfFile::VisGroup));

        R_ASSERT(IsNullTerminated(visGroup.Name, RmfFile::VisGroup::Name_Length), "VisGroup name must end with NULL character");

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::VisGroup& visGroup)
    {
        out.write(reinterpret_cast<const char*>(&visGroup), sizeof(RmfFile::VisGroup));

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Face& face)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        in.read(reinterpret_cast<char*>(&face), offsetof(RmfFile::Face, Vertices));

        // Vertices
        int verticesCount = 0;
        in.read(reinterpret_cast<char*>(&verticesCount), sizeof(verticesCount));
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        face.Vertices.resize(verticesCount);
        in.read(reinterpret_cast<char*>(face.Vertices.data()), sizeof(RmfFile::Vector_t) * verticesCount);

        in.read(reinterpret_cast<char*>(face.PlaneVertices), sizeof(RmfFile::Vector_t) * RmfFile::Face::PlaneVertices_Length);

        R_ASSERT(IsNullTerminated(face.TextureName, RmfFile::Face::TextureName_Length), "Face texture name must end with NULL character");

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Face& face)
    {
        out.write(reinterpret_cast<const char*>(&face), offsetof(RmfFile::Face, Vertices));

        // Vertices
        R_ASSERT(face.Vertices.size() <= std::numeric_limits<int>::max(), "Too many vertices to fit int32");
        int verticesCount = face.Vertices.size();
        out.write(reinterpret_cast<const char*>(&verticesCount), sizeof(verticesCount));
        out.write(reinterpret_cast<const char*>(face.Vertices.data()), sizeof(RmfFile::Vector_t) * verticesCount);

        out.write(reinterpret_cast<const char*>(face.PlaneVertices), sizeof(RmfFile::Vector_t) * RmfFile::Face::PlaneVertices_Length);

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Brush& brush)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        in.read(reinterpret_cast<char*>(&brush.VisGroup), sizeof(brush.VisGroup));
        in.read(reinterpret_cast<char*>(&brush.DisplayColor), sizeof(brush.DisplayColor));
        in.read(reinterpret_cast<char*>(brush.Dummy), brush.Dummy_Length);

        // Faces
        int faceCount = 0;
        in.read(reinterpret_cast<char*>(&faceCount), sizeof(faceCount));
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        R_ASSERT(faceCount >= 4, "RMF Brush must have at least 4 faces to form a 3D object");
        R_ASSERT(brush.Faces.size() <= std::numeric_limits<int>::max(), "RMF Brush contains too many faces");
        brush.Faces.resize(faceCount);
        for(int i = 0; i < faceCount; i++)
            in >> brush.Faces[i];

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Brush& brush)
    {
        WriteNString(out, brush.TypeName);

        out.write(reinterpret_cast<const char*>(&brush.VisGroup), sizeof(brush.VisGroup));
        out.write(reinterpret_cast<const char*>(&brush.DisplayColor), sizeof(brush.DisplayColor));
        out.write(reinterpret_cast<const char*>(brush.Dummy), sizeof(brush.Dummy_Length));

        // Faces
        R_ASSERT(brush.Faces.size() <= std::numeric_limits<int>::max(), "RMF Brush contains too many faces");
        int faceCount = brush.Faces.size();
        out.write(reinterpret_cast<const char*>(&faceCount), sizeof(faceCount));
        for(int i = 0; i < faceCount; i++)
            out << brush.Faces[i];

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Entity& entity)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        in.read(reinterpret_cast<char*>(&entity.VisGroup), sizeof(entity.VisGroup));
        in.read(reinterpret_cast<char*>(&entity.DisplayColor), sizeof(entity.DisplayColor));

        // Brushes
        {
            int brushCount = 0;
            in.read(reinterpret_cast<char*>(&brushCount), sizeof(brushCount));
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            R_ASSERT(entity.Brushes.size() <= std::numeric_limits<int>::max(), "RMF entity contains too many brushes");

            entity.Brushes.reserve(brushCount);
            for(int i = 0; i < brushCount; i++)
            {
                std::string type = ReadNString(in, 20);
                R_ASSERT(type == RmfFile::Brush::TypeName, "RMF entity can only contain Brush sub-objects but '" << type << "' found");

                RmfFile::Brush brush{};
                in >> brush;
                R_ASSERT(in.good(), "Input stream is not in a good shape");

                entity.Brushes.emplace_back(brush);
            }
        }

        entity.Classname = ReadNString(in, entity.Classname_MaxLength);

        in.read(reinterpret_cast<char*>(entity.Dummy), entity.Dummy_Length);
        in.read(reinterpret_cast<char*>(&entity.EntityFlags), sizeof(entity.EntityFlags));

        // Read Values
        {
            int keyValueCount = 0;
            in.read(reinterpret_cast<char*>(&keyValueCount), sizeof(keyValueCount));

            for(int i = 0; i < keyValueCount; i++)
            {
#ifdef RMF_NO_LIMITS
                const auto key = ReadNString(in);
                entity.Values[key] = ReadNString(in);
#else
                const auto key = ReadNString(in, RmfFile::Entity::KeyValue_Key_MaxLength);
                entity.Values[key] = ReadNString(in, RmfFile::Entity::KeyValue_Value_MaxLength);
#endif
            }
        }

        in.read(reinterpret_cast<char*>(entity.Dummy2), entity.Dummy2_Length);
        in.read(reinterpret_cast<char*>(&entity.Position), sizeof(entity.Position));
        in.read(reinterpret_cast<char*>(entity.Dummy3), entity.Dummy3_Length);

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Entity& entity)
    {
        WriteNString(out, entity.TypeName);

        out.write(reinterpret_cast<const char*>(&entity.VisGroup), sizeof(RmfFile::Entity::VisGroup));
        out.write(reinterpret_cast<const char*>(&entity.DisplayColor), sizeof(RmfFile::Entity::DisplayColor));

        // Brushes
        R_ASSERT(entity.Brushes.size() <= std::numeric_limits<int>::max(), "RMF entity contains too many brushes");
        int brushCount = entity.Brushes.size();
        out.write(reinterpret_cast<const char*>(&brushCount), sizeof(brushCount));
        for(int i = 0; i < brushCount; i++)
            out << entity.Brushes[i];

#ifndef RMF_NO_LIMITS
        R_ASSERT(entity.Classname.size() < RmfFile::Entity::Classname_MaxLength, "RMF Entity '" << entity.Classname << "' is too long (maximum " << (int)RmfFile::Entity::Classname_MaxLength << " characters)"); // < and not <= because there needs to be space for '\0'
#endif
        WriteNString(out, entity.Classname);

        out.write(reinterpret_cast<const char*>(entity.Dummy), RmfFile::Entity::Dummy_Length);
        out.write(reinterpret_cast<const char*>(&entity.EntityFlags), sizeof(RmfFile::Entity::EntityFlags));

        // Key-Values
        R_ASSERT(entity.Values.size() <= std::numeric_limits<int>::max(), "RMF Entity has too many values");
        int pairs = entity.Values.size();
        out.write(reinterpret_cast<const char*>(&pairs), sizeof(pairs));
        for(const auto& kv : entity.Values)
        {
#ifndef RMF_NO_LIMITS
            R_ASSERT(kv.first.size() < RmfFile::Entity::KeyValue_Key_MaxLength, "Key of RMF Entity value is too long");
            R_ASSERT(kv.second.size() < RmfFile::Entity::KeyValue_Value_MaxLength, "Key of RMF Entity value is too long");
#endif
            WriteNString(out, kv.first);
            WriteNString(out, kv.second);
        }

        out.write(reinterpret_cast<const char*>(entity.Dummy2), RmfFile::Entity::Dummy2_Length);
        out.write(reinterpret_cast<const char*>(&entity.Position), sizeof(RmfFile::Entity::Position));
        out.write(reinterpret_cast<const char*>(entity.Dummy3), RmfFile::Entity::Dummy3_Length);

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Group& group)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        in.read(reinterpret_cast<char*>(&group.VisGroup), sizeof(group.VisGroup));
        in.read(reinterpret_cast<char*>(&group.DisplayColor), sizeof(group.DisplayColor));

        // Objects
        {
            int objectCount = 0;
            in.read(reinterpret_cast<char*>(&objectCount), sizeof(objectCount));
            R_ASSERT(in.good(), "Input stream is not in a good shape");

            group.Brushes.reserve(objectCount);
            group.Entities.reserve(objectCount);
            group.Groups.reserve(objectCount);
            for(int i = 0; i < objectCount; i++)
            {
                std::string type = ReadNString(in, 20);
                if(type == RmfFile::Brush::TypeName)
                {
                    RmfFile::Brush brush{};
                    in >> brush;
                    R_ASSERT(in.good(), "Input stream is not in a good shape");

                    group.Brushes.emplace_back(brush);
                }
                else if(type == RmfFile::Entity::TypeName)
                {
                    RmfFile::Entity entity{};
                    in >> entity;
                    R_ASSERT(in.good(), "Input stream is not in a good shape");

                    group.Entities.emplace_back(entity);
                }
                else if(type == RmfFile::Group::TypeName)
                {
                    RmfFile::Group subGroup{};
                    in >> subGroup;
                    R_ASSERT(in.good(), "Input stream is not in a good shape");

                    group.Groups.emplace_back(subGroup);
                }
                else
                    throw std::runtime_error("Invalid object inside a group");
            }
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Group& group)
    {
        WriteNString(out, group.TypeName);

        out.write(reinterpret_cast<const char*>(&group.VisGroup), sizeof(RmfFile::Entity::VisGroup));
        out.write(reinterpret_cast<const char*>(&group.DisplayColor), sizeof(RmfFile::Entity::DisplayColor));

        int objectCount = group.Brushes.size() + group.Entities.size() + group.Groups.size();
        out.write(reinterpret_cast<const char*>(&objectCount), sizeof(objectCount));
        for(const auto& brush : group.Brushes)
            out << brush;
        for(const auto& entity : group.Entities)
            out << entity;
        for(const auto& subGroup : group.Groups)
            out << subGroup;

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Corner& corner)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        in.read(reinterpret_cast<char*>(&corner.Position), sizeof(corner.Position));
        in.read(reinterpret_cast<char*>(&corner.Index), sizeof(corner.Index));

        in.read(reinterpret_cast<char*>(corner.NameOverride), corner.NameOverride_Length);
        R_ASSERT(IsNullTerminated(corner.NameOverride, corner.NameOverride_Length), "RMF Corner name is too long");

        // Read Values
        {
            int keyValueCount = 0;
            in.read(reinterpret_cast<char*>(&keyValueCount), sizeof(keyValueCount));

            for(int i = 0; i < keyValueCount; i++)
            {
#ifdef RMF_NO_LIMITS
                const auto key = ReadNString(in);
                corner.Values[key] = ReadNString(in);
#else
                const auto key = ReadNString(in, RmfFile::Entity::KeyValue_Key_MaxLength);
                corner.Values[key] = ReadNString(in, RmfFile::Entity::KeyValue_Value_MaxLength);
#endif
            }
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Corner& corner)
    {
        out.write(reinterpret_cast<const char*>(&corner.Position), sizeof(RmfFile::Corner::Position));
        out.write(reinterpret_cast<const char*>(&corner.Index), sizeof(RmfFile::Corner::Index));
        out.write(reinterpret_cast<const char*>(corner.NameOverride), RmfFile::Corner::NameOverride_Length);

        // Key-Values
        R_ASSERT(corner.Values.size() <= std::numeric_limits<int>::max(), "RMF Corner has too many values");
        int keyValueCount = corner.Values.size();
        out.write(reinterpret_cast<const char*>(&keyValueCount), sizeof(keyValueCount));
        for(const auto& kv : corner.Values)
        {
#ifndef RMF_NO_LIMITS
            R_ASSERT(kv.first.size() < RmfFile::Entity::KeyValue_Key_MaxLength, "Key of RMF Corner value is too long");
            R_ASSERT(kv.second.size() < RmfFile::Entity::KeyValue_Value_MaxLength, "Key of RMF Corner value is too long");
#endif
            WriteNString(out, kv.first);
            WriteNString(out, kv.second);
        }

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::PathType& pathType)
    {
        in.read(reinterpret_cast<char*>(&pathType), sizeof(pathType));
        //TODO Validate

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::PathType& pathType)
    {
        //TODO Validate
        out.write(reinterpret_cast<const char*>(&pathType), sizeof(pathType));

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Path& path)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        in.read(reinterpret_cast<char*>(path.Name), path.Name_Length);
        R_ASSERT(IsNullTerminated(path.Name, path.Name_Length), "RMF Path's name must end by NULL character");

        in.read(reinterpret_cast<char*>(path.Classname), path.Class_Length);
        R_ASSERT(IsNullTerminated(path.Classname, path.Class_Length), "RMF Path's name must end by NULL character");
#ifdef RMF_PATH_TYPE_CHECK
        R_ASSERT(path.Class_str() == "path_corner" || path.Class_str() == "path_track", "RMF path can only be `path_corner` or `path_track`");
#endif

        in >> path.Type;

        // Read Corners
        {
            int cornerCount = 0;
            in.read(reinterpret_cast<char*>(&cornerCount), sizeof(cornerCount));
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            path.Corners.reserve(cornerCount);

            for(int i = 0; i < cornerCount; i++)
            {
                RmfFile::Corner corner = {};
                in >> corner;

                path.Corners.emplace_back(corner);
            }
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Path& path)
    {
        out.write(path.Name, RmfFile::Path::Name_Length);
#ifdef RMF_PATH_TYPE_CHECK
        R_ASSERT(path.Class_str() == "path_corner" || path.Class_str() == "path_track", "RMF path can only be `path_corner` or `path_track`");
#endif
        out.write(path.Classname, RmfFile::Path::Class_Length);
        out << path.Type;

        int cornerCount = path.Corners.size();
        out.write(reinterpret_cast<const char*>(&cornerCount), sizeof(cornerCount));
        for(const auto& corner : path.Corners)
            out << corner;

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Camera& camera)
    {
        in.read(reinterpret_cast<char*>(&camera.EyePosition), sizeof(camera.EyePosition));
        in.read(reinterpret_cast<char*>(&camera.LookPosition), sizeof(camera.LookPosition));

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Camera& camera)
    {
        out.write(reinterpret_cast<const char*>(&camera.EyePosition), sizeof(camera.EyePosition));
        out.write(reinterpret_cast<const char*>(&camera.LookPosition), sizeof(camera.LookPosition));

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::World& world)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        in.read(reinterpret_cast<char*>(&world.VisGroup), sizeof(world.VisGroup));
        in.read(reinterpret_cast<char*>(&world.DisplayColor), sizeof(world.DisplayColor));

        // Objects
        {
            int objectCount = 0;
            in.read(reinterpret_cast<char*>(&objectCount), sizeof(objectCount));
            R_ASSERT(in.good(), "Input stream is not in a good shape");

            world.Brushes.reserve(objectCount);
            world.Entities.reserve(objectCount);
            world.Groups.reserve(objectCount);
            for(int i = 0; i < objectCount; i++)
            {
                std::string type = ReadNString(in, 20);
                if(type == RmfFile::Brush::TypeName)
                {
                    RmfFile::Brush brush{};
                    in >> brush;
                    R_ASSERT(in.good(), "Input stream is not in a good shape");

                    world.Brushes.emplace_back(brush);
                }
                else if(type == RmfFile::Entity::TypeName)
                {
                    RmfFile::Entity entity{};
                    in >> entity;
                    R_ASSERT(in.good(), "Input stream is not in a good shape");

                    world.Entities.emplace_back(entity);
                }
                else if(type == RmfFile::Group::TypeName)
                {
                    RmfFile::Group subGroup{};
                    in >> subGroup;
                    R_ASSERT(in.good(), "Input stream is not in a good shape");

                    world.Groups.emplace_back(subGroup);
                }
                else
                    throw std::runtime_error("Invalid object inside the world");

            }
        }

        world.Classname = ReadNString(in, RmfFile::Entity::Classname_MaxLength);
        R_ASSERT(world.Classname == "worldspawn", "RMF World must have classname `worldspawn`");

        in.read(reinterpret_cast<char*>(world.Dummy), world.Dummy_Length);
        in.read(reinterpret_cast<char*>(&world.EntityFlags), sizeof(world.EntityFlags));

        // Read Values
        {
            int keyValueCount = 0;
            in.read(reinterpret_cast<char*>(&keyValueCount), sizeof(keyValueCount));

            for(int i = 0; i < keyValueCount; i++)
            {
#ifdef RMF_NO_LIMITS
                const auto key = ReadNString(in);
                world.Values[key] = ReadNString(in);
#else
                const auto key = ReadNString(in, RmfFile::Entity::KeyValue_Key_MaxLength);
                world.Values[key] = ReadNString(in, RmfFile::Entity::KeyValue_Value_MaxLength);
#endif
            }
        }

        in.read(reinterpret_cast<char*>(world.Dummy2), world.Dummy2_Length);

        // Paths
        {
            int pathCount = 0;
            in.read(reinterpret_cast<char*>(&pathCount), sizeof(pathCount));
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            world.Paths.reserve(pathCount);

            for(int i = 0; i < pathCount; i++)
            {
                RmfFile::Path path = {};
                in >> path;

                world.Paths.emplace_back(path);
            }
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::World& world)
    {
        WriteNString(out, world.TypeName);

        out.write(reinterpret_cast<const char*>(&world.VisGroup), sizeof(RmfFile::World::VisGroup));
        out.write(reinterpret_cast<const char*>(&world.DisplayColor), sizeof(RmfFile::World::DisplayColor));

        int objectCount = world.Brushes.size() + world.Entities.size() + world.Groups.size();
        out.write(reinterpret_cast<const char*>(&objectCount), sizeof(objectCount));
        for(const auto& brush : world.Brushes)
            out << brush;
        for(const auto& entity : world.Entities)
            out << entity;
        for(const auto& subGroup : world.Groups)
            out << subGroup;

        R_ASSERT(world.Classname == "worldspawn", "RMF World must have classname `worldspawn`");
        WriteNString(out, world.Classname);
        out.write(reinterpret_cast<const char*>(world.Dummy), RmfFile::World::Dummy_Length);
        out.write(reinterpret_cast<const char*>(&world.EntityFlags), sizeof(RmfFile::World::EntityFlags));

        // Key-Values
        R_ASSERT(world.Values.size() <= std::numeric_limits<int>::max(), "RMF World has too many values");
#ifdef RMF_INCLUDE_WAD
        int pairs = world.Values.size();
#else
        int pairs = world.Values.size() - (world.Values.contains("wad") ? 1 : 0);
#endif
        out.write(reinterpret_cast<const char*>(&pairs), sizeof(pairs));
        for(const auto& kv : world.Values)
        {
#ifndef RMF_INCLUDE_WAD
            if(kv.first == "wad")
            {
                std::cerr << "Skipping \"wad\" value (not saved into RMF): " << kv.second << std::endl;
            }
            else
#endif
            {
#ifndef RMF_NO_LIMITS
                R_ASSERT(kv.first.size() < RmfFile::Entity::KeyValue_Key_MaxLength, "Key of RMF World value is too long");
                R_ASSERT(kv.second.size() < RmfFile::Entity::KeyValue_Value_MaxLength, "Key of RMF World value is too long");
#endif
                WriteNString(out, kv.first);
                WriteNString(out, kv.second);
            }
        }

        out.write(reinterpret_cast<const char*>(world.Dummy2), RmfFile::World::Dummy2_Length);

        R_ASSERT(world.Paths.size() <= std::numeric_limits<int>::max(), "RMF World has too many paths");
        int pathCount = world.Paths.size();
        out.write(reinterpret_cast<const char*>(&pathCount), sizeof(pathCount));
        for(const auto& path : world.Paths)
            out << path;

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile& rmf)
    {
        // Magic header
        {
            uint8_t magic1[rmf.Magic1.size()];
            in.read(reinterpret_cast<char*>(magic1), rmf.Magic1.size());
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            for(int i = 0; i < rmf.Magic1.size(); i++)
                if(magic1[i] != rmf.Magic1[i])
                    throw std::runtime_error("Failed to read first magic number of RMF file");

            uint8_t magic2[rmf.Magic2.size()];
            in.read(reinterpret_cast<char*>(magic2), rmf.Magic2.size());
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            for(int i = 0; i < rmf.Magic2.size(); i++)
                if(magic2[i] != rmf.Magic2[i])
                    throw std::runtime_error("Failed to read second magic number of RMF file");
        }

        // Vis Groups
        {
            int visGroupCount = 0;
            in.read(reinterpret_cast<char*>(&visGroupCount), sizeof(visGroupCount));
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            for(int i = 0; i < visGroupCount; i++)
            {
                RmfFile::VisGroup visGroup{};
                in >> visGroup;
                R_ASSERT(in.good(), "Input stream is not in a good shape");
                rmf.VisGroups.emplace_back(visGroup);
            }
        }

        // World Info
        {
            std::string worldInfoType = ReadNString(in, 20);
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            R_ASSERT(worldInfoType == RmfFile::World::TypeName, "Invalid type of RMF Object - expected RMF World (\"" << RmfFile::World::TypeName << "\")");
            in >> rmf.WorldInfo;
            R_ASSERT(in.good(), "Input stream is not in a good shape");
        }

        // Doc Info
        {
            uint8_t docInfo[rmf.DocInfo.size()];
            in.read(reinterpret_cast<char*>(docInfo), rmf.DocInfo.size());
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            for(int i = 0; i < rmf.DocInfo.size(); i++)
                if(docInfo[i] != rmf.DocInfo[i])
                    throw std::runtime_error("Failed to read DOCINFO of RMF file");

            float cameraVersion = 0;
            in.read(reinterpret_cast<char*>(&cameraVersion), sizeof(cameraVersion));
#ifdef DEBUG
            std::cout << "Camera version: " << cameraVersion << std::endl;
#endif

            in.read(reinterpret_cast<char*>(&rmf.ActiveCamera), sizeof(rmf.ActiveCamera));

            int cameraCount = 0;
            in.read(reinterpret_cast<char*>(&cameraCount), sizeof(cameraCount));
            R_ASSERT(in.good(), "Input stream is not in a good shape");
            for(int i = 0; i < cameraCount; i++)
            {
                RmfFile::Camera camera = {};
                in >> camera;
                R_ASSERT(in.good(), "Input stream is not in a good shape");
                rmf.Cameras.emplace_back(camera);
            }
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile& rmf)
    {
        out.write(reinterpret_cast<const char*>(rmf.Magic1.data()), rmf.Magic1.size());
        out.write(reinterpret_cast<const char*>(rmf.Magic2.data()), rmf.Magic2.size());

        R_ASSERT(rmf.VisGroups.size() <= std::numeric_limits<int>::max(), "RMF file has too many VisGroups");
        int visGroupCount = rmf.VisGroups.size();
        out.write(reinterpret_cast<const char*>(&visGroupCount), sizeof(visGroupCount));
        for(const auto& path : rmf.VisGroups)
            out << path;

        out << rmf.WorldInfo;

        out.write(reinterpret_cast<const char*>(rmf.DocInfo.data()), rmf.DocInfo.size());

        float cameraDataVersion = 0; //TODO Verify
        out.write(reinterpret_cast<const char*>(&cameraDataVersion), sizeof(cameraDataVersion));
        out.write(reinterpret_cast<const char*>(&rmf.ActiveCamera), sizeof(rmf.ActiveCamera));

        R_ASSERT(rmf.Cameras.size() <= std::numeric_limits<int>::max(), "RMF file has too many cameras");
        int cameraCount = rmf.Cameras.size();
        R_ASSERT(rmf.ActiveCamera == -1 || rmf.ActiveCamera < rmf.Cameras.size(), "Invalid active camera - out of bounds (use -1 for no camera)");
        out.write(reinterpret_cast<const char*>(&cameraCount), sizeof(cameraCount));
        for(const auto& camera : rmf.Cameras)
            out << camera;

        return out;
    }
}
