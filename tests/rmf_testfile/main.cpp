#include "Decay/Rmf/RmfFile.hpp"

#include <glm/gtx/string_cast.hpp>

using namespace Decay::Rmf;

int main(int argc, const char* argv[])
{
    if(argc < 2)
        throw std::runtime_error("Please provide path to RMF file");
    std::cout << argv[1] << std::endl;

    std::fstream in = std::fstream(argv[1], std::ios_base::in | std::ios_base::binary);
    RmfFile rmf(in);

#ifdef DEBUG
    std::cout << "Active camera: " << rmf.ActiveCamera << std::endl;
    std::cout << "Camera count: " << rmf.Cameras.size() << std::endl;
    for(const auto& camera : rmf.Cameras)
        std::cout << "\t- " << glm::to_string(camera.EyePosition) << " -> " << glm::to_string(camera.LookPosition) << std::endl;
#endif

    {
        std::fstream out = std::fstream(argv[1] + std::string("_out"), std::ios_base::out);
        out << rmf;
    }

    std::fstream in2 = std::fstream(argv[1] + std::string("_out"), std::ios_base::in | std::ios_base::binary);
    RmfFile rmf2(in2);
}
