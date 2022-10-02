#include "Decay/Rmf/RmfFile.hpp"

#include <cstring>

using namespace Decay::Rmf;

#define R_ASSERT_TEST(a_column) R_ASSERT(result.a_column == original.a_column, #a_column " does not match")

void Test_VisGroup(const RmfFile::VisGroup& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::VisGroup result{};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT(result.Name_str() == original.Name_str(), "Name does not match");
    R_ASSERT_TEST(Color);
    R_ASSERT_TEST(Dummy);
    R_ASSERT_TEST(Index);
    R_ASSERT_TEST(Visible);

    for(int i = 0; i < RmfFile::VisGroup::Dummy2_Length; i++)
        R_ASSERT(result.Dummy2[i] == original.Dummy2[i], "Dummy2[" << i << "] does not match");

    R_ASSERT(result == original, "Result does not match the original");
}
void Test_Face(const RmfFile::Face& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Face result{};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT(result.TextureName_str() == original.TextureName_str(), "Texture Name does not match");
    R_ASSERT_TEST(Dummy);

    R_ASSERT_TEST(UAxis);
    R_ASSERT_TEST(XShift);
    R_ASSERT_TEST(VAxis);
    R_ASSERT_TEST(YShift);

    R_ASSERT_TEST(TextureRotation);
    R_ASSERT_TEST(TextureScale);

    for(int i = 0; i < RmfFile::Face::Dummy2_Length; i++)
    R_ASSERT(result.Dummy2[i] == original.Dummy2[i], "Dummy2[" << i << "] does not match");

    R_ASSERT(result.Vertices.size() == original.Vertices.size(), "Vertex count does not match");
    for(int i = 0; i < result.Vertices.size(); i++)
        R_ASSERT(result.Vertices[i] == original.Vertices[i], "Vertices[" << i << "] does not match");

    for(int i = 0; i < RmfFile::Face::PlaneVertices_Length; i++)
        R_ASSERT(result.PlaneVertices[i] == original.PlaneVertices[i], "PlaneVertices[" << i << "] does not match");
    R_ASSERT(result == original, "Result does not match the original");
}
void Test_Solid(const RmfFile::Brush& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Brush result{};
    { // Type Name
        int typeNameLength = ss.get();
        R_ASSERT(std::strlen(RmfFile::Brush::TypeName) + 1 == typeNameLength, "Brush saved incorrect type (based on length)");
        ss.ignore(typeNameLength);
    }
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT_TEST(VisGroup);
    R_ASSERT_TEST(DisplayColor);

    for(int i = 0; i < RmfFile::Brush::Dummy_Length; i++)
        R_ASSERT(result.Dummy[i] == original.Dummy[i], "Dummy[" << i << "] does not match");

    R_ASSERT(result.Faces.size() == original.Faces.size(), "Face count does not match");
    for(int i = 0; i < result.Faces.size(); i++)
        R_ASSERT(result.Faces[i] == original.Faces[i], "Faces[" << i << "] does not match");

    R_ASSERT(result == original, "Result does not match the original");
}
void Test_Entity(const RmfFile::Entity& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Entity result{};
    { // Type Name
        int typeNameLength = ss.get();
        R_ASSERT(std::strlen(RmfFile::Entity::TypeName) + 1 == typeNameLength, "Entity saved incorrect type (based on length)");
        ss.ignore(typeNameLength);
    }
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT_TEST(VisGroup);
    R_ASSERT_TEST(DisplayColor);

    R_ASSERT(result.Brushes.size() == original.Brushes.size(), "Brush count does not match");
    for(int i = 0; i < result.Brushes.size(); i++)
        R_ASSERT(result.Brushes[i] == original.Brushes[i], "Brushes[" << i << "] does not match");

    R_ASSERT_TEST(Classname);

    for(int i = 0; i < RmfFile::Entity::Dummy_Length; i++)
        R_ASSERT(result.Dummy[i] == original.Dummy[i], "Dummy[" << i << "] does not match");

    R_ASSERT_TEST(EntityFlags);

    R_ASSERT(Decay::IsSame(result.Values, original.Values), "Key-Values do not match");

    for(int i = 0; i < RmfFile::Entity::Dummy2_Length; i++)
        R_ASSERT(result.Dummy2[i] == original.Dummy2[i], "Dummy2[" << i << "] does not match");

    R_ASSERT_TEST(Position);

    for(int i = 0; i < RmfFile::Entity::Dummy3_Length; i++)
        R_ASSERT(result.Dummy3[i] == original.Dummy3[i], "Dummy3[" << i << "] does not match");

    R_ASSERT(result == original, "Result does not match the original");
}
void Test_Group(const RmfFile::Group& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Group result{};
    { // Type Name
        int typeNameLength = ss.get();
        R_ASSERT(std::strlen(RmfFile::Group::TypeName) + 1 == typeNameLength, "Group saved incorrect type (based on length)");
        ss.ignore(typeNameLength);
    }
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT_TEST(VisGroup);
    R_ASSERT_TEST(DisplayColor);

    R_ASSERT(result.Brushes.size() == original.Brushes.size(), "Brush count does not match");
    for(int i = 0; i < result.Brushes.size(); i++)
        R_ASSERT(result.Brushes[i] == original.Brushes[i], "Brushes[" << i << "] does not match");

    R_ASSERT(result.Entities.size() == original.Entities.size(), "Entity count does not match");
    for(int i = 0; i < result.Entities.size(); i++)
        R_ASSERT(result.Entities[i] == original.Entities[i], "Entities[" << i << "] does not match");

    R_ASSERT(result.Groups.size() == original.Groups.size(), "Group count does not match");
    for(int i = 0; i < result.Groups.size(); i++)
        R_ASSERT(result.Groups[i] == original.Groups[i], "Groups[" << i << "] does not match");

    R_ASSERT(result == original, "Result does not match the original");
}
void Test_Corner(const RmfFile::Corner& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Corner result{};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT_TEST(Position);
    R_ASSERT_TEST(Index);
    R_ASSERT(result.NameOverride_str() == original.NameOverride_str(), "Name Override does not match");

    R_ASSERT(Decay::IsSame(result.Values, original.Values), "Key-Values do not match");

    R_ASSERT(result == original, "Result does not match the original");
}
void Test_PathType(const RmfFile::PathType& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::PathType result{};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT(result == original, "Result does not match the original");
}
void Test_Path(const RmfFile::Path& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Path result{};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT(result.Name_str() == original.Name_str(), "Name does not match");
    R_ASSERT(result.Class_str() == original.Class_str(), "Class does not match");
    R_ASSERT_TEST(Type);

    R_ASSERT(result.Corners.size() == original.Corners.size(), "Corner count does not match");
    for(int i = 0; i < result.Corners.size(); i++)
        R_ASSERT(result.Corners[i] == original.Corners[i], "Corners[" << i << "] does not match");

    R_ASSERT(result == original, "Result does not match the original");
}
void Test_Camera(const RmfFile::Camera& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::Camera result{};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT_TEST(EyePosition);
    R_ASSERT_TEST(LookPosition);

    R_ASSERT(result == original, "Result does not match the original");
}
void Test_World(const RmfFile::World& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    RmfFile::World result{};
    { // Type Name
        int typeNameLength = ss.get();
        R_ASSERT(std::strlen(RmfFile::World::TypeName) + 1 == typeNameLength, "World saved incorrect type (based on length)");
        ss.ignore(typeNameLength);
    }
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT_TEST(VisGroup);
    R_ASSERT_TEST(DisplayColor);

    R_ASSERT(result.Brushes.size() == original.Brushes.size(), "Brush count does not match");
    for(int i = 0; i < result.Brushes.size(); i++)
        R_ASSERT(result.Brushes[i] == original.Brushes[i], "Brushes[" << i << "] does not match");

    R_ASSERT(result.Entities.size() == original.Entities.size(), "Entity count does not match");
    for(int i = 0; i < result.Entities.size(); i++)
        R_ASSERT(result.Entities[i] == original.Entities[i], "Entities[" << i << "] does not match");

    R_ASSERT(result.Groups.size() == original.Groups.size(), "Group count does not match");
    for(int i = 0; i < result.Groups.size(); i++)
        R_ASSERT(result.Groups[i] == original.Groups[i], "Groups[" << i << "] does not match");

    R_ASSERT_TEST(Classname);

    for(int i = 0; i < RmfFile::World::Dummy_Length; i++)
        R_ASSERT(result.Dummy[i] == original.Dummy[i], "Dummy[" << i << "] does not match");

    R_ASSERT_TEST(EntityFlags);

    R_ASSERT(Decay::IsSame(result.Values, original.Values), "Key-Values do not match");

    for(int i = 0; i < RmfFile::World::Dummy2_Length; i++)
        R_ASSERT(result.Dummy2[i] == original.Dummy2[i], "Dummy2[" << i << "] does not match");

    R_ASSERT(result.Paths.size() == original.Paths.size(), "Path count does not match");
    for(int i = 0; i < result.Paths.size(); i++)
        R_ASSERT(result.Paths[i] == original.Paths[i], "Paths[" << i << "] does not match");

    R_ASSERT(result == original, "Result does not match the original");
}

int main()
{
    // Vis Group
    {
        std::cout << "Vis Group" << std::endl;

        Test_VisGroup(
            RmfFile::VisGroup(
                "abcd",
                { 1, 2, 3 },
                0,
                false
            )
        );
        Test_VisGroup(
            RmfFile::VisGroup(
                "aBcD",
                { 66, 6, 0 },
                1,
                true
            )
        );

        std::cout << std::endl;
    }

    // Face
    RmfFile::Face face0;
    RmfFile::Face face1;
    {
        std::cout << "Face" << std::endl;

        face0 = RmfFile::Face(
            "abcd",
            { 1, 0, 0 }, 0, // X / U
            { 0, -1, 0 }, 0, // Y / V
            0,
            { 1.0, 1.0 }
        );
        face0.emplace({ 0, 0, 0 });
        face0.emplace({ 1, 1, 0 });
        face0.emplace({ 1, 0, 0 });

        face1 = {
            "aBcD",
            { 1, 0, 0 }, 1, // X / U
            { 0, -1, 0 }, -1, // Y / V
            45,
            { -1.0, 3.6 }
        };
        face1.emplace({ 0, 0, 123 });
        face1.emplace({ 1, 1, 123 });
        face1.emplace({ 1, 0, 123 });

        Test_Face(face0);
        Test_Face(face1);

        std::cout << std::endl;
    }

    // Brush
    RmfFile::Brush brush0;
    RmfFile::Brush brush1;
    {
        std::cout << "Brush" << std::endl;

        brush0 = RmfFile::Brush(
            0,
            { 1, 2, 3 }
        );
        brush0.Faces.emplace_back(face0);
        brush0.Faces.emplace_back(face0);
        brush0.Faces.emplace_back(face0);
        brush0.Faces.emplace_back(face0);

        brush1 = RmfFile::Brush(
            1,
            { 0, 66, 3 }
        );
        brush1.Faces.emplace_back(face0);
        brush1.Faces.emplace_back(face0);
        brush1.Faces.emplace_back(face0);
        brush1.Faces.emplace_back(face0);
        brush1.Faces.emplace_back(face1);

        Test_Solid(brush0);
        Test_Solid(brush1);

        std::cout << std::endl;
    }

    // Entity
    RmfFile::Entity entity0;
    RmfFile::Entity entity1;
    RmfFile::Entity entity2;
    {
        std::cout << "Entity" << std::endl;

        entity0 = RmfFile::Entity(
            0,
            { 1, 2, 3 },
            "worldspawn",
            0,
            { 11, 22, 33 }
        );
        entity0.Brushes.emplace_back(brush0);
#ifdef RMF_INCLUDE_WAD
        entity0.Values["wad"] = R"(\half-life\valve\half-life.wad)";
#endif
        entity0.Values["skybox"] = "city_01";

        entity1 = RmfFile::Entity(
            1,
            { 11, 22, 33 },
            "worldspawn",
            0,
            { 11, 22, 33 }
        );
        entity1.Brushes.emplace_back(brush0);
        entity1.Brushes.emplace_back(brush1);
#ifdef RMF_INCLUDE_WAD
        entity1.Values["wad"] = R"(\half-life\valve\half-life.wad)";
#endif
        entity1.Values["skybox"] = "city_17";

        entity2 = RmfFile::Entity(
            0,
            { 16, 26, 36 },
            "info",
            0,
            { 0, 0, 0 }
        );

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
                brush0,
                brush1
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
                brush0
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
#ifdef RMF_INCLUDE_WAD
                { "wad", "\\half-life\\valve\\half-life.wad" },
#endif
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
