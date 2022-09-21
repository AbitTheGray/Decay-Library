#include "Decay/Map/MapFile.hpp"

using namespace Decay::Map;

void Test_Plane(const MapFile::Plane& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Plane result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    for(int i = 0; i < MapFile::Plane::PlaneVertexCount; i++)
    {
        assert(result.PlaneVertices[i] == original.PlaneVertices[i]);
    }
    assert(result.Texture == original.Texture);
    assert(result.UAxis == original.UAxis);
    assert(result.UOffset == original.UOffset);
    assert(result.VAxis == original.VAxis);
    assert(result.VOffset == original.VOffset);
    assert(result.Rotation == original.Rotation);
    assert(result.Scale == original.Scale);
    assert(result == original);
}
void Test_Brush(const MapFile::Brush& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Brush result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.Planes.size() == original.Planes.size());
    for(int i = 0; i < result.Planes.size(); i++)
    {
        assert(result.Planes[i] == original.Planes[i]);
    }
    assert(result == original);
}
void Test_Entity(const MapFile::Entity& original)
{
    std::stringstream ss;
    ss << original;
    assert(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Entity result = {};
    ss >> result;
    assert(ss.good() || ss.eof());
    assert(result.Values.size() == original.Values.size());
    for(const auto& originalKv : original.Values)
    {
        assert(result.Values.contains(originalKv.first));
        assert(result.Values[originalKv.first] == originalKv.second);
    }
    assert(result.Brushes.size() == original.Brushes.size());
    for(int i = 0; i < result.Brushes.size(); i++)
    {
        assert(result.Brushes[i] == original.Brushes[i]);
    }
    //assert(result == original); //TODO
}

int main()
{
    // Plane
    {
        std::cout << "Plane" << std::endl;

        Test_Plane({
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
                   });
        Test_Plane({
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
                   });

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
