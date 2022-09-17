#include "Decay/Map/MapFile.hpp"

using namespace Decay::Map;

int main(int argc, const char* argv[])
{
    if(argc < 2)
        throw std::runtime_error("Please provide path to MAP file");
    std::cout << argv[1] << std::endl;

    std::fstream in = std::fstream(argv[1], std::ios_base::in);
    MapFile map(in);

    std::cout << map;
#ifdef DEBUG
    for(const auto& entity : map.Entities)
    {
        for(const auto& brush : entity.Brushes)
        {
            for(const auto& plane : brush.Planes)
            {
                const auto normal = plane.Normal();
                std::cout << "( " << std::to_string(normal.x) << " * x ) + ( " << std::to_string(normal.y) << " * y ) + ( " << std::to_string(normal.z) << " * z )" << " = " << std::to_string(plane.DistanceFromOrigin()) << std::endl;
            }
        }
    }
#endif

    {
        std::fstream out = std::fstream(argv[1] + std::string("_out"), std::ios_base::out);
        out << map;
    }
}
