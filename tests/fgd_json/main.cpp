#include "Decay/Fgd/FgdFile.hpp"

int main()
{
    using namespace Decay::Fgd;

#ifdef DECAY_JSON_LIB
    {
        std::fstream in = std::fstream("file.fgd", std::ios_base::in);
        FgdFile fgd(in);

        nlohmann::json j = fgd.ExportAsJson();

        std::cout << j.dump(2);

        std::fstream out = std::fstream("fgd.json", std::ios_base::out);
        out << j.dump(2);
    }
#endif
}
