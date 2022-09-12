#include "Decay/Fgd/FgdFile.hpp"

using namespace Decay::Fgd;

int main(int argc, const char* argv[])
{
    if(argc < 2)
        throw std::runtime_error("Plrease provide path to FGD file");
    std::cout << argv[1] << std::endl;

    std::fstream in = std::fstream(argv[1], std::ios_base::in);
    FgdFile fgd(in);

    std::cout << fgd;

    {
        std::fstream out = std::fstream(argv[1] + std::string("_out"), std::ios_base::out);
        out << fgd;
    }
}
