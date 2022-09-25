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
        assert(maxLength >= 0);

        uint8_t length = 0;
        in.read(reinterpret_cast<char*>(&length), sizeof(length));

        if(maxLength > 0 && length > maxLength)
            throw std::runtime_error("Failed to read NString - length is above allowed value");

        if(length > 1) // Contains data
        {
            std::string rtn = std::string(length - 1, ' ');
            in.read(rtn.data(), length - 1);
            assert(in.get() == '\0');
            return rtn;
        }
        else if(length == 1) // 1 = only null-termination
        {
            assert(in.get() == '\0');
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
        assert(text.size() <= std::numeric_limits<uint8_t>::max());
        uint8_t length = text.size() + 1;
        out.write(reinterpret_cast<const char*>(&length), sizeof(length));

        out.write(text.data(), text.size());

        uint8_t byte = 0;
        out.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
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
}

// Stream Operators
namespace Decay::Rmf
{
    std::istream& operator>>(std::istream& in, RmfFile::VisGroup& visGroup)
    {
        assert(in.good());

        in.read(reinterpret_cast<char*>(&visGroup), sizeof(RmfFile::VisGroup));

        assert(IsNullTerminated(visGroup.Name, RmfFile::VisGroup::Name_Length));

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::VisGroup& visGroup)
    {
        out.write(reinterpret_cast<const char*>(&visGroup), sizeof(RmfFile::VisGroup));

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Face& face)
    {
        assert(in.good());

        in.read(reinterpret_cast<char*>(&face), offsetof(RmfFile::Face, Vertices));

        // Vertices
        int verticesCount = 0;
        in.read(reinterpret_cast<char*>(&verticesCount), sizeof(verticesCount));
        assert(in.good());
        face.Vertices.resize(verticesCount);
        in.read(reinterpret_cast<char*>(face.Vertices.data()), sizeof(RmfFile::Vector_t) * verticesCount);

        in.read(reinterpret_cast<char*>(face.PlaneVertices), sizeof(RmfFile::Vector_t) * RmfFile::Face::PlaneVertices_Length);

        assert(IsNullTerminated(face.TextureName, RmfFile::Face::TextureName_Length));

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Face& face)
    {
        out.write(reinterpret_cast<const char*>(&face), offsetof(RmfFile::Face, Vertices));

        // Vertices
        assert(face.Vertices.size() <= std::numeric_limits<int>::max());
        int verticesCount = face.Vertices.size();
        out.write(reinterpret_cast<const char*>(&verticesCount), sizeof(verticesCount));
        out.write(reinterpret_cast<const char*>(face.Vertices.data()), sizeof(RmfFile::Vector_t) * verticesCount);

        out.write(reinterpret_cast<const char*>(face.PlaneVertices), sizeof(RmfFile::Vector_t) * RmfFile::Face::PlaneVertices_Length);

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Solid& solid)
    {
        assert(in.good());
        in.read(reinterpret_cast<char*>(&solid.VisGroup), sizeof(solid.VisGroup));
        in.read(reinterpret_cast<char*>(&solid.DisplayColor), sizeof(solid.DisplayColor));
        in.read(reinterpret_cast<char*>(&solid.Dummy), solid.Dummy_Length);

        // Faces
        int faceCount = 0;
        in.read(reinterpret_cast<char*>(&faceCount), sizeof(faceCount));
        assert(in.good());
        assert(faceCount >= 4);
        solid.Faces.resize(faceCount);
        for(int i = 0; i < faceCount; i++)
            in >> solid.Faces[i];

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Solid& solid)
    {
        WriteNString(out, solid.TypeName);

        out.write(reinterpret_cast<const char*>(&solid.VisGroup), sizeof(solid.VisGroup));
        out.write(reinterpret_cast<const char*>(&solid.DisplayColor), sizeof(solid.DisplayColor));
        out.write(reinterpret_cast<const char*>(&solid.Dummy), sizeof(solid.Dummy_Length));

        // Faces
        assert(solid.Faces.size() <= std::numeric_limits<int>::max());
        int faceCount = solid.Faces.size();
        out.write(reinterpret_cast<const char*>(&faceCount), sizeof(faceCount));
        for(int i = 0; i < faceCount; i++)
            out << solid.Faces[i];

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Entity& entity)
    {
        assert(in.good());
        in.read(reinterpret_cast<char*>(&entity.VisGroup), sizeof(entity.VisGroup));
        in.read(reinterpret_cast<char*>(&entity.DisplayColor), sizeof(entity.DisplayColor));

        // Solids
        {
            int solidCount = 0;
            in.read(reinterpret_cast<char*>(&solidCount), sizeof(solidCount));
            assert(in.good());

            entity.Solids.reserve(solidCount);
            for(int i = 0; i < solidCount; i++)
            {
                std::string type = ReadNString(in, 20);
                assert(type == RmfFile::Solid::TypeName);

                RmfFile::Solid solid{};
                in >> solid;
                assert(in.good());

                entity.Solids.emplace_back(solid);
            }
        }

        entity.Classname = ReadNString(in, entity.Classname_MaxLength);

        in.read(reinterpret_cast<char*>(&entity.Dummy), entity.Dummy_Length);
        in.read(reinterpret_cast<char*>(&entity.EntityFlags), sizeof(entity.EntityFlags));

        // Read KeyValue
        {
            int keyValueCount = 0;
            in.read(reinterpret_cast<char*>(&keyValueCount), sizeof(keyValueCount));

            for(int i = 0; i < keyValueCount; i++)
                entity.KeyValue[ReadNString(in, RmfFile::Entity::KeyValue_Key_MaxLength)] = ReadNString(in, RmfFile::Entity::KeyValue_Value_MaxLength);
        }

        in.read(reinterpret_cast<char*>(&entity.Dummy2), entity.Dummy2_Length);
        in.read(reinterpret_cast<char*>(&entity.Position), sizeof(entity.Position));
        in.read(reinterpret_cast<char*>(&entity.Dummy3), entity.Dummy3_Length);

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Entity& entity)
    {
        WriteNString(out, entity.TypeName);

        out.write(reinterpret_cast<const char*>(&entity.VisGroup), sizeof(RmfFile::Entity::VisGroup));
        out.write(reinterpret_cast<const char*>(&entity.DisplayColor), sizeof(RmfFile::Entity::DisplayColor));

        // Solids
        assert(entity.Solids.size() <= std::numeric_limits<int>::max());
        int solidCount = entity.Solids.size();
        out.write(reinterpret_cast<const char*>(&solidCount), sizeof(solidCount));
        for(int i = 0; i < solidCount; i++)
            out << entity.Solids[i];

        assert(entity.Classname.size() < RmfFile::Entity::Classname_MaxLength); // < and not <= because there needs to be space for '\0'
        WriteNString(out, entity.Classname);

        out.write(reinterpret_cast<const char*>(&entity.Dummy), RmfFile::Entity::Dummy_Length);
        out.write(reinterpret_cast<const char*>(&entity.EntityFlags), sizeof(RmfFile::Entity::EntityFlags));

        // Key-Values
        assert(entity.KeyValue.size() <= std::numeric_limits<int>::max());
        int pairs = entity.KeyValue.size();
        out.write(reinterpret_cast<const char*>(&pairs), sizeof(pairs));
        for(const auto& kv : entity.KeyValue)
        {
            assert(kv.first.size() < RmfFile::Entity::KeyValue_Key_MaxLength);
            assert(kv.second.size() < RmfFile::Entity::KeyValue_Value_MaxLength);
            WriteNString(out, kv.first);
            WriteNString(out, kv.second);
        }

        out.write(reinterpret_cast<const char*>(&entity.Dummy2), RmfFile::Entity::Dummy2_Length);
        out.write(reinterpret_cast<const char*>(&entity.Position), sizeof(RmfFile::Entity::Position));
        out.write(reinterpret_cast<const char*>(&entity.Dummy3), RmfFile::Entity::Dummy3_Length);

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Group& group)
    {
        assert(in.good());
        in.read(reinterpret_cast<char*>(&group.VisGroup), sizeof(group.VisGroup));
        in.read(reinterpret_cast<char*>(&group.DisplayColor), sizeof(group.DisplayColor));

        // Objects
        {
            int objectCount = 0;
            in.read(reinterpret_cast<char*>(&objectCount), sizeof(objectCount));
            assert(in.good());

            group.Solids.reserve(objectCount);
            group.Entities.reserve(objectCount);
            group.Groups.reserve(objectCount);
            for(int i = 0; i < objectCount; i++)
            {
                std::string type = ReadNString(in, 20);
                if(type == RmfFile::Solid::TypeName)
                {
                    RmfFile::Solid solid{};
                    in >> solid;
                    assert(in.good());

                    group.Solids.emplace_back(solid);
                }
                else if(type == RmfFile::Entity::TypeName)
                {
                    RmfFile::Entity entity{};
                    in >> entity;
                    assert(in.good());

                    group.Entities.emplace_back(entity);
                }
                else if(type == RmfFile::Group::TypeName)
                {
                    RmfFile::Group subGroup{};
                    in >> subGroup;
                    assert(in.good());

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

        int objectCount = group.Solids.size() + group.Entities.size() + group.Groups.size();
        out.write(reinterpret_cast<const char*>(&objectCount), sizeof(objectCount));
        for(const auto& solid : group.Solids)
            out << solid;
        for(const auto& entity : group.Entities)
            out << entity;
        for(const auto& subGroup : group.Groups)
            out << subGroup;

        return out;
    }

    std::istream& operator>>(std::istream& in, RmfFile::Corner& corner)
    {
        assert(in.good());
        in.read(reinterpret_cast<char*>(&corner.Position), sizeof(corner.Position));
        in.read(reinterpret_cast<char*>(&corner.Index), sizeof(corner.Index));

        in.read(reinterpret_cast<char*>(&corner.NameOverride), corner.NameOverride_Length);
        assert(IsNullTerminated(corner.NameOverride, corner.NameOverride_Length));

        // Read KeyValue
        {
            int keyValueCount = 0;
            in.read(reinterpret_cast<char*>(&keyValueCount), sizeof(keyValueCount));

            for(int i = 0; i < keyValueCount; i++)
                corner.KeyValue[ReadNString(in, RmfFile::Entity::KeyValue_Key_MaxLength)] = ReadNString(in, RmfFile::Entity::KeyValue_Value_MaxLength);
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile::Corner& corner)
    {
        out.write(reinterpret_cast<const char*>(&corner.Position), sizeof(RmfFile::Corner::Position));
        out.write(reinterpret_cast<const char*>(&corner.Index), sizeof(RmfFile::Corner::Index));
        out.write(reinterpret_cast<const char*>(&corner.NameOverride), RmfFile::Corner::NameOverride_Length);

        // Key-Values
        assert(corner.KeyValue.size() <= std::numeric_limits<int>::max());
        int keyValueCount = corner.KeyValue.size();
        out.write(reinterpret_cast<const char*>(&keyValueCount), sizeof(keyValueCount));
        for(const auto& kv : corner.KeyValue)
        {
            assert(kv.first.size() < RmfFile::Entity::KeyValue_Key_MaxLength);
            assert(kv.second.size() < RmfFile::Entity::KeyValue_Value_MaxLength);
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
        assert(in.good());

        in.read(reinterpret_cast<char*>(&path.Name), path.Name_Length);
        assert(IsNullTerminated(path.Name, path.Name_Length));

        in.read(reinterpret_cast<char*>(&path.Class), path.Class_Length);
        assert(IsNullTerminated(path.Class, path.Class_Length));
        assert(path.Class_str() == "path_corner" || path.Class_str() == "path_track");

        in >> path.Type;

        // Read Corners
        {
            int cornerCount = 0;
            in.read(reinterpret_cast<char*>(&cornerCount), sizeof(cornerCount));
            assert(in.good());
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
        assert(path.Class_str() == "path_corner" || path.Class_str() == "path_track");
        out.write(path.Class, RmfFile::Path::Class_Length);
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
        assert(in.good());
        in.read(reinterpret_cast<char*>(&world.VisGroup), sizeof(world.VisGroup));
        in.read(reinterpret_cast<char*>(&world.DisplayColor), sizeof(world.DisplayColor));

        // Objects
        {
            int objectCount = 0;
            in.read(reinterpret_cast<char*>(&objectCount), sizeof(objectCount));
            assert(in.good());

            world.Solids.reserve(objectCount);
            world.Entities.reserve(objectCount);
            world.Groups.reserve(objectCount);
            for(int i = 0; i < objectCount; i++)
            {
                std::string type = ReadNString(in, 20);
                if(type == RmfFile::Solid::TypeName)
                {
                    RmfFile::Solid solid{};
                    in >> solid;
                    assert(in.good());

                    world.Solids.emplace_back(solid);
                }
                else if(type == RmfFile::Entity::TypeName)
                {
                    RmfFile::Entity entity{};
                    in >> entity;
                    assert(in.good());

                    world.Entities.emplace_back(entity);
                }
                else if(type == RmfFile::Group::TypeName)
                {
                    RmfFile::Group subGroup{};
                    in >> subGroup;
                    assert(in.good());

                    world.Groups.emplace_back(subGroup);
                }
                else
                    throw std::runtime_error("Invalid object inside the world");

            }
        }

        world.Classname = ReadNString(in, RmfFile::Entity::Classname_MaxLength);
        assert(world.Classname == "worldspawn");

        in.read(reinterpret_cast<char*>(&world.Dummy), world.Dummy_Length);
        in.read(reinterpret_cast<char*>(&world.EntityFlags), sizeof(world.EntityFlags));

        // Read KeyValue
        {
            int keyValueCount = 0;
            in.read(reinterpret_cast<char*>(&keyValueCount), sizeof(keyValueCount));

            for(int i = 0; i < keyValueCount; i++)
                world.KeyValue[ReadNString(in, RmfFile::Entity::KeyValue_Key_MaxLength)] = ReadNString(in, RmfFile::Entity::KeyValue_Value_MaxLength);
        }

        in.read(reinterpret_cast<char*>(&world.Dummy2), world.Dummy2_Length);

        // Paths
        {
            int pathCount = 0;
            in.read(reinterpret_cast<char*>(&pathCount), sizeof(pathCount));
            assert(in.good());
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

        int objectCount = world.Solids.size() + world.Entities.size() + world.Groups.size();
        out.write(reinterpret_cast<const char*>(&objectCount), sizeof(objectCount));
        for(const auto& solid : world.Solids)
            out << solid;
        for(const auto& entity : world.Entities)
            out << entity;
        for(const auto& subGroup : world.Groups)
            out << subGroup;

        assert(world.Classname == "worldspawn");
        WriteNString(out, world.Classname);
        out.write(reinterpret_cast<const char*>(&world.Dummy), RmfFile::World::Dummy_Length);
        out.write(reinterpret_cast<const char*>(&world.EntityFlags), sizeof(RmfFile::World::EntityFlags));

        // Key-Values
        assert(world.KeyValue.size() <= std::numeric_limits<int>::max());
        int pairs = world.KeyValue.size();
        out.write(reinterpret_cast<const char*>(&pairs), sizeof(pairs));
        for(const auto& kv : world.KeyValue)
        {
            assert(kv.first.size() < RmfFile::Entity::KeyValue_Key_MaxLength);
            assert(kv.second.size() < RmfFile::Entity::KeyValue_Value_MaxLength);
            WriteNString(out, kv.first);
            WriteNString(out, kv.second);
        }

        out.write(reinterpret_cast<const char*>(&world.Dummy2), RmfFile::World::Dummy2_Length);

        assert(world.Paths.size() <= std::numeric_limits<int>::max());
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
            in.read(reinterpret_cast<char*>(&magic1), rmf.Magic1.size());
            assert(in.good());
            for(int i = 0; i < rmf.Magic1.size(); i++)
                if(magic1[i] != rmf.Magic1[i])
                    throw std::runtime_error("Failed to read first magic number of RMF file");

            uint8_t magic2[rmf.Magic2.size()];
            in.read(reinterpret_cast<char*>(&magic2), rmf.Magic2.size());
            assert(in.good());
            for(int i = 0; i < rmf.Magic2.size(); i++)
                if(magic2[i] != rmf.Magic2[i])
                    throw std::runtime_error("Failed to read second magic number of RMF file");
        }

        // Vis Groups
        {
            int visGroupCount = 0;
            in.read(reinterpret_cast<char*>(&visGroupCount), sizeof(visGroupCount));
            assert(in.good());
            for(int i = 0; i < visGroupCount; i++)
            {
                RmfFile::VisGroup visGroup = {};
                in >> visGroup;
                assert(in.good());
                rmf.VisGroups.emplace_back(visGroup);
            }
        }

        // World Info
        {
            std::string worldInfoType = ReadNString(in, 20);
            assert(in.good());
            assert(worldInfoType == RmfFile::World::TypeName);
            in >> rmf.WorldInfo;
            assert(in.good());
        }

        // Doc Info
        {
            uint8_t docInfo[rmf.DocInfo.size()];
            in.read(reinterpret_cast<char*>(&docInfo), rmf.DocInfo.size());
            assert(in.good());
            for(int i = 0; i < rmf.DocInfo.size(); i++)
                if(docInfo[i] != rmf.DocInfo[i])
                    throw std::runtime_error("Failed to read DOCINFO of RMF file");

            float cameraVersion = 0;
            in.read(reinterpret_cast<char*>(&cameraVersion), sizeof(cameraVersion));

            in.read(reinterpret_cast<char*>(&rmf.ActiveCamera), sizeof(rmf.ActiveCamera));

            int cameraCount = 0;
            in.read(reinterpret_cast<char*>(&cameraCount), sizeof(cameraCount));
            assert(in.good());
            for(int i = 0; i < cameraCount; i++)
            {
                RmfFile::Camera camera = {};
                in >> camera;
                assert(in.good());
                rmf.Cameras.emplace_back(camera);
            }
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const RmfFile& rmf)
    {
        out.write(reinterpret_cast<const char*>(rmf.Magic1.data()), rmf.Magic1.size());
        out.write(reinterpret_cast<const char*>(rmf.Magic2.data()), rmf.Magic2.size());

        assert(rmf.VisGroups.size() <= std::numeric_limits<int>::max());
        int visGroupCount = rmf.VisGroups.size();
        out.write(reinterpret_cast<const char*>(&visGroupCount), sizeof(visGroupCount));
        for(const auto& path : rmf.VisGroups)
            out << path;

        out << rmf.WorldInfo;

        out.write(reinterpret_cast<const char*>(rmf.DocInfo.data()), rmf.DocInfo.size());

        float cameraDataVersion = 0; //TODO Verify
        out.write(reinterpret_cast<const char*>(&cameraDataVersion), sizeof(cameraDataVersion));
        out.write(reinterpret_cast<const char*>(&rmf.ActiveCamera), sizeof(rmf.ActiveCamera));

        assert(rmf.Cameras.size() <= std::numeric_limits<int>::max());
        int cameraCount = rmf.Cameras.size();
        assert(rmf.ActiveCamera == -1 || rmf.ActiveCamera < rmf.Cameras.size());
        out.write(reinterpret_cast<const char*>(&cameraCount), sizeof(cameraCount));
        for(const auto& camera : rmf.Cameras)
            out << camera;

        return out;
    }
}