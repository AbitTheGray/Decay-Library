#include "Decay/Fgd/FgdFile.hpp"

int main()
{
    using namespace Decay::Fgd;

#ifdef DECAY_JSON_LIB
    {
        std::fstream in = std::fstream("file.fgd", std::ios_base::in);
        FgdFile fgd(in);

        {
            std::cout << std::endl;
            std::cout << "================" << std::endl;
            std::cout << "| Not ordered  |" << std::endl;
            std::cout << "================" << std::endl;
            std::cout << std::endl;
            nlohmann::json j = fgd.ExportAsJson(false);

            std::cout << j.dump(2);

            std::fstream out = std::fstream("fgd.json", std::ios_base::out);
            out << j.dump(2);
        }
        {
            std::cout << std::endl;
            std::cout << "================" << std::endl;
            std::cout << "| Ordered      |" << std::endl;
            std::cout << "================" << std::endl;
            std::cout << std::endl;
            nlohmann::json j = fgd.ExportAsJson(true);

            std::cout << j.dump(2);

            std::fstream out = std::fstream("fgd_ordered.json", std::ios_base::out);
            out << j.dump(2);
        }
    }
#endif
}
