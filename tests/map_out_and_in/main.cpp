#include "Decay/Map/MapFile.hpp"

using namespace Decay::Map;

void Test_Face(const MapFile::Face& original, MapFile::EngineVariant variant)
{
    std::stringstream ss;
    original.Write(ss, variant);
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Face result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    for(int i = 0; i < MapFile::Face::PlaneVertexCount; i++)
    {
        R_ASSERT(result.PlaneVertices[i] == original.PlaneVertices[i]);
    }
    R_ASSERT(result.Texture == original.Texture);
    if(variant != MapFile::EngineVariant::IdTech2)
        R_ASSERT(result.UAxis == original.UAxis);
    R_ASSERT(result.UOffset == original.UOffset);
    if(variant != MapFile::EngineVariant::IdTech2)
        R_ASSERT(result.VAxis == original.VAxis);
    R_ASSERT(result.VOffset == original.VOffset);
    R_ASSERT(result.Rotation == original.Rotation);
    R_ASSERT(result.Scale == original.Scale);
    //R_ASSERT(result == original);
}
void Test_Brush(const MapFile::Brush& original)
{
    std::stringstream ss;
    original.Write(ss, MapFile::EngineVariant::GoldSrc);
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Brush result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.Faces.size() == original.Faces.size());
    for(int i = 0; i < result.Faces.size(); i++)
    {
        R_ASSERT(result.Faces[i] == original.Faces[i]);
    }
    R_ASSERT(result == original);
}
void Test_Entity(const MapFile::Entity& original)
{
    std::stringstream ss;
    original.Write(ss, MapFile::EngineVariant::GoldSrc);
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Entity result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.Values.size() == original.Values.size());
    for(const auto& originalKv : original.Values)
    {
        R_ASSERT(result.Values.contains(originalKv.first));
        R_ASSERT(result.Values[originalKv.first] == originalKv.second);
    }
    R_ASSERT(result.Brushes.size() == original.Brushes.size());
    for(int i = 0; i < result.Brushes.size(); i++)
    {
        R_ASSERT(result.Brushes[i] == original.Brushes[i]);
    }
    //R_ASSERT(result == original); //TODO
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
