#include "Decay/Rmf/RmfFile.hpp"

#include <cstring>

using namespace Decay::Rmf;

void Test_VisGroup(const RmfFile::VisGroup& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::VisGroup result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.Name_str() == original.Name_str());
    assert(result.Color == original.Color);
    assert(result.Dummy == original.Dummy);
    assert(result.Index == original.Index);
    assert(result.Visible == original.Visible);

    for(int i = 0; i < RmfFile::VisGroup::Dummy2_Length; i++)
        assert(result.Dummy2[i] == original.Dummy2[i]);

    assert(result == original);
}
void Test_Face(const RmfFile::Face& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Face result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.TextureName_str() == original.TextureName_str());
    assert(result.Dummy == original.Dummy);

    assert(result.UAxis == original.UAxis);
    assert(result.XShift == original.XShift);
    assert(result.VAxis == original.VAxis);
    assert(result.YShift == original.YShift);

    assert(result.TextureRotation == original.TextureRotation);
    assert(result.TextureScale == original.TextureScale);

    for(int i = 0; i < RmfFile::Face::Dummy2_Length; i++)
        assert(result.Dummy2[i] == original.Dummy2[i]);

    assert(result.Vertices.size() == original.Vertices.size());
    for(int i = 0; i < result.Vertices.size(); i++)
        assert(result.Vertices[i] == original.Vertices[i]);

    for(int i = 0; i < RmfFile::Face::PlaneVertices_Length; i++)
        assert(result.PlaneVertices[i] == original.PlaneVertices[i]);
    assert(result == original);
}
void Test_Solid(const RmfFile::Solid& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Solid result = {};
    { // Type Name
        int typeNameLength = ss.get();
        assert(std::strlen(RmfFile::Solid::TypeName) + 1 == typeNameLength);
        ss.ignore(typeNameLength);
    }
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.VisGroup == original.VisGroup);
    assert(result.DisplayColor == original.DisplayColor);

    for(int i = 0; i < RmfFile::Solid::Dummy_Length; i++)
        assert(result.Dummy[i] == original.Dummy[i]);

    assert(result.Faces.size() == original.Faces.size());
    for(int i = 0; i < result.Faces.size(); i++)
        assert(result.Faces[i] == original.Faces[i]);

    assert(result == original);
}
void Test_Entity(const RmfFile::Entity& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Entity result = {};
    { // Type Name
        int typeNameLength = ss.get();
        assert(std::strlen(RmfFile::Entity::TypeName) + 1 == typeNameLength);
        ss.ignore(typeNameLength);
    }
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.VisGroup == original.VisGroup);
    assert(result.DisplayColor == original.DisplayColor);

    assert(result.Solids.size() == original.Solids.size());
    for(int i = 0; i < result.Solids.size(); i++)
        assert(result.Solids[i] == original.Solids[i]);

    assert(result.Classname == original.Classname);

    for(int i = 0; i < RmfFile::Entity::Dummy_Length; i++)
        assert(result.Dummy[i] == original.Dummy[i]);

    assert(result.EntityFlags == original.EntityFlags);

    //TODO order-independent test of KeyValue

    for(int i = 0; i < RmfFile::Entity::Dummy2_Length; i++)
        assert(result.Dummy2[i] == original.Dummy2[i]);

    assert(result.Position == original.Position);

    for(int i = 0; i < RmfFile::Entity::Dummy3_Length; i++)
        assert(result.Dummy3[i] == original.Dummy3[i]);

    assert(result == original);
}
void Test_Group(const RmfFile::Group& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Group result = {};
    { // Type Name
        int typeNameLength = ss.get();
        assert(std::strlen(RmfFile::Group::TypeName) + 1 == typeNameLength);
        ss.ignore(typeNameLength);
    }
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.VisGroup == original.VisGroup);
    assert(result.DisplayColor == original.DisplayColor);

    assert(result.Solids.size() == original.Solids.size());
    for(int i = 0; i < result.Solids.size(); i++)
        assert(result.Solids[i] == original.Solids[i]);

    assert(result.Entities.size() == original.Entities.size());
    for(int i = 0; i < result.Entities.size(); i++)
        assert(result.Entities[i] == original.Entities[i]);

    assert(result.Groups.size() == original.Groups.size());
    for(int i = 0; i < result.Groups.size(); i++)
        assert(result.Groups[i] == original.Groups[i]);

    assert(result == original);
}
void Test_Corner(const RmfFile::Corner& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Corner result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.Position == original.Position);
    assert(result.Index == original.Index);
    assert(result.NameOverride_str() == original.NameOverride_str());

    //TODO order-independent test of KeyValue

    assert(result == original);
}
void Test_PathType(const RmfFile::PathType& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::PathType result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result == original);
}
void Test_Path(const RmfFile::Path& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Path result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.Name_str() == original.Name_str());
    assert(result.Class_str() == original.Class_str());
    assert(result.Type == original.Type);

    assert(result.Corners.size() == original.Corners.size());
    for(int i = 0; i < result.Corners.size(); i++)
        assert(result.Corners[i] == original.Corners[i]);

    assert(result == original);
}
void Test_Camera(const RmfFile::Camera& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Camera result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.EyePosition == original.EyePosition);
    assert(result.LookPosition == original.LookPosition);

    assert(result == original);
}
void Test_World(const RmfFile::World& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::World result = {};
    { // Type Name
        int typeNameLength = ss.get();
        assert(std::strlen(RmfFile::World::TypeName) + 1 == typeNameLength);
        ss.ignore(typeNameLength);
    }
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.VisGroup == original.VisGroup);
    assert(result.DisplayColor == original.DisplayColor);

    assert(result.Solids.size() == original.Solids.size());
    for(int i = 0; i < result.Solids.size(); i++)
        assert(result.Solids[i] == original.Solids[i]);

    assert(result.Entities.size() == original.Entities.size());
    for(int i = 0; i < result.Entities.size(); i++)
        assert(result.Entities[i] == original.Entities[i]);

    assert(result.Groups.size() == original.Groups.size());
    for(int i = 0; i < result.Groups.size(); i++)
        assert(result.Groups[i] == original.Groups[i]);

    assert(result.Classname == original.Classname);

    for(int i = 0; i < RmfFile::Entity::Dummy_Length; i++)
        assert(result.Dummy[i] == original.Dummy[i]);

    assert(result.EntityFlags == original.EntityFlags);

    //TODO order-independent test of KeyValue

    for(int i = 0; i < RmfFile::Entity::Dummy2_Length; i++)
        assert(result.Dummy2[i] == original.Dummy2[i]);

    assert(result.Paths.size() == original.Paths.size());
    for(int i = 0; i < result.Paths.size(); i++)
        assert(result.Paths[i] == original.Paths[i]);

    assert(result == original);
}

int main()
{
    // Vis Group
    {
        std::cout << "Vis Group" << std::endl;

        Test_VisGroup({
            { 'a', 'b', 'c', 'd' },
            { 1, 2, 3 },
            0, // Dummy
            0,
            0,
            {} // Dummy 2
        });

        Test_VisGroup({
            { 'a', 'B', 'c', 'D' },
            { 66, 6, 0 },
            0, // Dummy
            1,
            1,
            {} // Dummy 2
        });

        std::cout << std::endl;
    }

    // Face
    RmfFile::Face face0;
    RmfFile::Face face1;
    {
        std::cout << "Face" << std::endl;

        face0 = {
            { 'a', 'b', 'c', 'd' },
            0.0f, // Dummy
            { 1, 0, 0 }, 0,
            { 0, -1, 0 }, 0,
            0,
            { 1.0, 1.0 },
            {}, // Dummy 2
            { // Vertices
                { 0, 0, 0 },
                { 1, 1, 0 },
                { 1, 0, 0 }
            },
            { // Plane Vertices
                { 0, 0, 0 },
                { 1, 1, 0 },
                { 1, 0, 0 }
            }
        };

        face1 = {
            { 'a', 'B', 'c', 'D' },
            0.0f, // Dummy
            { 1, 0, 0 }, 1,
            { 0, -1, 0 }, -1,
            45,
            { -1.0, 3.6 },
            {}, // Dummy 2
            { // Vertices
                { 0, 0, 123 },
                { 1, 1, 123 },
                { 1, 0, 123 }
            },
            { // Plane Vertices
                { 0, 0, 123 },
                { 1, 1, 123 },
                { 1, 0, 123 }
            }
        };

        Test_Face(face0);
        Test_Face(face1);

        std::cout << std::endl;
    }

    // Solid
    RmfFile::Solid solid0;
    RmfFile::Solid solid1;
    {
        std::cout << "Solid" << std::endl;

        solid0 = {
            0,
            { 1, 2, 3 },
            {}, // Dummy
            {
                face0,
                face0,
                face0,
                face0
            }
        };

        solid1 = {
            1,
            { 0, 66, 3 },
            {}, // Dummy
            {
                face0,
                face0,
                face0,
                face0,
                face1
            }
        };

        Test_Solid(solid0);
        Test_Solid(solid1);

        std::cout << std::endl;
    }

    // Entity
    RmfFile::Entity entity0;
    RmfFile::Entity entity1;
    RmfFile::Entity entity2;
    {
        std::cout << "Entity" << std::endl;

        entity0 = {
            0,
            { 1, 2, 3 },
            {
                solid0
            },
            "worldspawn",
            {}, // Dummy
            0,
            {
                { "wad", "\\half-life\\valve\\half-life.wad" },
                { "skybox", "city_01" }
            },
            {}, // Dummy 2
            { 11, 22, 33 },
            {} // Dummy 3
        };

        entity1 = {
            1,
            { 11, 22, 33 },
            {
                solid0,
                solid1
            },
            "worldspawn",
            {}, // Dummy
            0,
            {
                { "wad", "\\half-life\\valve\\half-life.wad" },
                { "skybox", "city_01" }
            },
            {}, // Dummy 2
            { 11, 22, 33 },
            {} // Dummy 3
        };

        entity2 = {
            0,
            { 16, 26, 36 },
            {},
            "info",
            {}, // Dummy
            0,
            {},
            {}, // Dummy 2
            { 0, 0, 0 },
            {} // Dummy 3
        };

        Test_Entity(entity0);
        Test_Entity(entity1);
        Test_Entity(entity2);

        std::cout << std::endl;
    }

    // Group
    RmfFile::Group group0;
    RmfFile::Group group1;
    {
        std::cout << "Group" << std::endl;

        group0 = {
            0,
            { 1, 2, 3 },
            {}, {}, {}
        };

        group1 = {
            1,
            { 1, 2, 3 },
            {
                solid0,
                solid1
            },
            {
                entity0,
                entity1
            },
            {
                group0
            }
        };

        Test_Group(group0);
        Test_Group(group1);

        std::cout << std::endl;
    }

    // Corner
    RmfFile::Corner corner0;
    RmfFile::Corner corner1;
    {
        std::cout << "Corner" << std::endl;

        corner0 = {
            { 1, 2, 3 },
            0,
            { 'a', 'b', 'c', 'd' },
            {}
        };

        corner1 = {
            { 1, 2, 3 },
            1,
            {},
            {
                { "abcd", "The Value" }
            }
        };

        Test_Corner(corner0);
        Test_Corner(corner1);

        std::cout << std::endl;
    }

    // PathType
    {
        std::cout << "PathType" << std::endl;

        Test_PathType(RmfFile::PathType::OneWay);
        Test_PathType(RmfFile::PathType::Circular);
        Test_PathType(RmfFile::PathType::PingPong);

        std::cout << std::endl;
    }

    // Path
    RmfFile::Path path0;
    RmfFile::Path path1;
    {
        std::cout << "Path" << std::endl;

        path0 = {
            { 'a', 'b', 'c', 'd' },
            { 'p', 'a', 't', 'h', '_', 'c', 'o', 'r', 'n', 'e', 'r' },
            RmfFile::PathType::Circular,
            {}
        };

        path1 = {
            { 'a', 'B', 'c', 'D' },
            { 'p', 'a', 't', 'h', '_', 't', 'r', 'a', 'c', 'k' },
            RmfFile::PathType::OneWay,
            {
                corner0,
                corner1
            }
        };

        Test_Path(path0);
        Test_Path(path1);

        std::cout << std::endl;
    }

    // Camera
    {
        std::cout << "Camera" << std::endl;

        Test_Camera({
            { 1, 2, 3 },
            { 16, 26, 36 },
        });

        Test_Camera({
            { 1, 1, 1 },
            { 0, 0, 0 },
        });

        std::cout << std::endl;
    }

    // World
    {
        std::cout << "Entity" << std::endl;

        RmfFile::World world0 = {
            0,
            { 1, 2, 3 },
            {
                solid0
            },
            {
                entity0,
                entity1,
                entity2
            },
            {
                group0,
                group1
            },
            "worldspawn",
            {}, // Dummy
            0,
            {
                { "wad", "\\half-life\\valve\\half-life.wad" },
                { "skybox", "city_01" }
            },
            {}, // Dummy 2
            {
                path0,
                path1
            }
        };

        Test_World(world0);

        std::cout << std::endl;
    }
}
