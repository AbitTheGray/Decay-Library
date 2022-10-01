#include "Decay/Fgd/FgdFile.hpp"

int main()
{
    using namespace Decay::Fgd;

    FgdFile fgd;
    {
        std::cout << std::filesystem::current_path() << std::endl;
        std::fstream in("main.fgd", std::ios_base::in);
        fgd = FgdFile(in);
    }
    {
        std::vector<std::filesystem::path> filesToIgnore{};
        filesToIgnore.emplace_back(std::filesystem::canonical("main.fgd"));

        fgd.ProcessIncludes(".", filesToIgnore);
    }

    R_ASSERT(fgd.MapSize.has_value());
    R_ASSERT(fgd.MapSize.value() == glm::i32vec2(-512, 512));

    R_ASSERT(fgd.Classes.size() == 1);
    auto& clss = *fgd.Classes.begin();
    R_ASSERT(clss.first == "worldspawn");
    R_ASSERT(clss.second.Properties.size() ==4);

    const auto& pMessage = clss.second.Properties["message"];
    R_ASSERT(pMessage.Type == "string");
    R_ASSERT(pMessage.DisplayName == "Message at the beginning of the map");

    const auto& pSkybox = clss.second.Properties["skybox"];
    R_ASSERT(pSkybox.Type == "string");
    R_ASSERT(pSkybox.DisplayName == "Skybox texture (cubemap)");

    const auto& pBaseProp = clss.second.Properties["baseProp"];
    R_ASSERT(pBaseProp.Type == "integer");

    const auto& pMainProp = clss.second.Properties["mainProp"];
    R_ASSERT(pMainProp.Type == "integer");
}
