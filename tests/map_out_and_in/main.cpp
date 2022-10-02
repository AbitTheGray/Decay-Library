#include "Decay/Map/MapFile.hpp"

using namespace Decay::Map;

void Test_Face(const MapFile::Face& original, MapFile::EngineVariant variant)
{
    std::stringstream ss;
    original.Write(ss, variant);
    R_ASSERT(ss.good(), "Stream is not in a good state");

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Face result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    for(int i = 0; i < MapFile::Face::PlaneVertexCount; i++)
    {
        R_ASSERT(result.PlaneVertices[i] == original.PlaneVertices[i], "Plane vertex at index " << i << " does not match");
    }
    R_ASSERT(result.Texture == original.Texture, "Face texture does not match");
    if(variant != MapFile::EngineVariant::IdTech2)
        R_ASSERT(result.UAxis == original.UAxis, "U Axis does not match");
    R_ASSERT(result.UOffset == original.UOffset, "U Offset does not match");
    if(variant != MapFile::EngineVariant::IdTech2)
        R_ASSERT(result.VAxis == original.VAxis, "V Axis does not match");
    R_ASSERT(result.VOffset == original.VOffset, "V Offset does not match");
    R_ASSERT(result.Rotation == original.Rotation, "Rotation does not match");
    R_ASSERT(result.Scale == original.Scale, "Scale does not match");
    //R_ASSERT(result == original, "Result does not match the original");
}
void Test_Brush(const MapFile::Brush& original)
{
    std::stringstream ss;
    original.Write(ss, MapFile::EngineVariant::GoldSrc);
    R_ASSERT(ss.good(), "Stream is not in a good state");

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Brush result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT(result.Faces.size() == original.Faces.size(), "Number of faces does not match");
    for(int i = 0; i < result.Faces.size(); i++)
    {
        R_ASSERT(result.Faces[i] == original.Faces[i], "Face at index " << i << " does not match");
    }
    R_ASSERT(result == original, "Brush does not match");
}
void Test_Entity(const MapFile::Entity& original)
{
    std::stringstream ss;
    original.Write(ss, MapFile::EngineVariant::GoldSrc);
    R_ASSERT(ss.good(), "Stream is not in a good state");

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Entity result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");
    R_ASSERT(Decay::IsSame(result.Values, original.Values), "Entity Values does not match");
    R_ASSERT(Decay::IsSame(result.Brushes, original.Brushes), "Entity Brushes does not match");
    //R_ASSERT(result == original, "Result does not match the original"); //TODO
}

int main()
{
    // Face
    {
        std::cout << "Face" << std::endl;

        MapFile::Face face0 = {
            {
                { 0, 0, 0 },
                { 1, 1, 0 },
                { 1, 0, 0 }
            },
            "abcd",
            { 1, 0, 0 }, 0,
            { 0, -1, 0 }, 0,
            0,
            { 1.0, 1.0 }
        };
        MapFile::Face face1 = {
            {
                { 0, 0, 123 },
                { 1, 1, 123 },
                { 1, 0, 123 }
            },
            "aBcD",
            { 1, 0, 0 }, 1,
            { 0, -1, 0 }, -1,
            45,
            { -1.0, 3.6 }
        };

        Test_Face(face0, MapFile::EngineVariant::IdTech2);
        Test_Face(face1, MapFile::EngineVariant::IdTech2);

        Test_Face(face0, MapFile::EngineVariant::GoldSrc);
        Test_Face(face1, MapFile::EngineVariant::GoldSrc);

        std::cout << std::endl;
    }
    // Brush
    {
        std::cout << "Brush" << std::endl;

        Test_Brush({
                       {
                           {
                               {
                                   { -48, 48, 128 },
                                   { 48, 48, 128 },
                                   { 48, -48, 128 }
                               },
                               "CRATE13",
                               { 1, 0, 0 }, 48,
                               { 0, -1, 0 }, 48,
                               0,
                               { 1.0, 1.0 }
                           },
                           {
                               {
                                   { -48, -48, 0 },
                                   { 48, -48, 0 },
                                   { 48, 48, 0 }
                               },
                               "CRATE13",
                               { 1, 0, 0 }, 48,
                               { 0, -1, 0 }, 48,
                               0,
                               { 1.0, 1.0 }
                           },
                           {
                               {
                                   { -48, 48, 128 },
                                   { -48, -48, 128 },
                                   { -48, -48, 0 }
                               },
                               "CRATE11",
                               { 0, 1, 0 }, 48,
                               { 0, 0, -1 }, 128,
                               0,
                               { 1.0, 1.0 }
                           },
                           {
                               {
                                   { 48, 48, 0 },
                                   { 48, -48, 0 },
                                   { 48, -48, 128 }
                               },
                               "CRATE11",
                               { 0, 1, 0 }, 48,
                               { 0, 0, -1 }, 128,
                               0,
                               { 1.0, 1.0 }
                           },
                           {
                               {
                                   { 48, 48, 128 },
                                   { -48, 48, 128 },
                                   { -48, 48, 0 }
                               },
                               "CRATE10",
                               { 1, 0, 0 }, 48,
                               { 0, 0, -1 }, 128,
                               0,
                               { 1.0, 1.0 }
                           },
                           {
                               {
                                   { 48, -48, 0 },
                                   { -48, -48, 0 },
                                   { -48, -48, 128 }
                               },
                               "CRATE10",
                               { 1, 0, 0 }, 48,
                               { 0, 0, -1 }, 128,
                               0,
                               { 1.0, 1.0 }
                           }
                       }
                   });

        std::cout << std::endl;
    }
    // Entity
    {
        std::cout << "Entity" << std::endl;

        MapFile::Entity entity = {};
        entity.Values.emplace("classname", "func_breakable");
        entity.Values.emplace("spawnflags", "256");
        entity.Values.emplace("rendercolor", "0 0 0");
        entity.Values.emplace("health", "100");
        entity.Values.emplace("material", "1");
        entity.Brushes.emplace_back(MapFile::Brush{
                                        {
                                            {
                                                {
                                                    { -48, 48, 128 },
                                                    { 48, 48, 128 },
                                                    { 48, -48, 128 }
                                                },
                                                "CRATE13",
                                                { 1, 0, 0 }, 48,
                                                { 0, -1, 0 }, 48,
                                                0,
                                                { 1.0, 1.0 }
                                            },
                                            {
                                                {
                                                    { -48, -48, 0 },
                                                    { 48, -48, 0 },
                                                    { 48, 48, 0 }
                                                },
                                                "CRATE13",
                                                { 1, 0, 0 }, 48,
                                                { 0, -1, 0 }, 48,
                                                0,
                                                { 1.0, 1.0 }
                                            },
                                            {
                                                {
                                                    { -48, 48, 128 },
                                                    { -48, -48, 128 },
                                                    { -48, -48, 0 }
                                                },
                                                "CRATE11",
                                                { 0, 1, 0 }, 48,
                                                { 0, 0, -1 }, 128,
                                                0,
                                                { 1.0, 1.0 }
                                            },
                                            {
                                                {
                                                    { 48, 48, 0 },
                                                    { 48, -48, 0 },
                                                    { 48, -48, 128 }
                                                },
                                                "CRATE11",
                                                { 0, 1, 0 }, 48,
                                                { 0, 0, -1 }, 128,
                                                0,
                                                { 1.0, 1.0 }
                                            },
                                            {
                                                {
                                                    { 48, 48, 128 },
                                                    { -48, 48, 128 },
                                                    { -48, 48, 0 }
                                                },
                                                "CRATE10",
                                                { 1, 0, 0 }, 48,
                                                { 0, 0, -1 }, 128,
                                                0,
                                                { 1.0, 1.0 }
                                            },
                                            {
                                                {
                                                    { 48, -48, 0 },
                                                    { -48, -48, 0 },
                                                    { -48, -48, 128 }
                                                },
                                                "CRATE10",
                                                { 1, 0, 0 }, 48,
                                                { 0, 0, -1 }, 128,
                                                0,
                                                { 1.0, 1.0 }
                                            }
                                        }
                                    });
        Test_Entity(entity);

        std::cout << std::endl;
    }
}
