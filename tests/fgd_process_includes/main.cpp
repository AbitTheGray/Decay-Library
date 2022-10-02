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

    R_ASSERT(fgd.MapSize.has_value(), "FGD did not load @MapSize from base2.fgd");
    R_ASSERT(fgd.MapSize.value() == glm::i32vec2(-512, 512), "FGD load incorrect @MapSize value");

    R_ASSERT(fgd.Classes.size() == 1, "Invalid number of classes");
    auto& clss = *fgd.Classes.begin();
    R_ASSERT(clss.first == "worldspawn", "Class is not a worldspawn");
    R_ASSERT(clss.second.Properties.size() == 4, "Class has unexpected number of properties");

    const auto& pMessage = clss.second.Properties["message"];
    R_ASSERT(pMessage.Type == "string", "`message` property is of unexpected type - does it even exist?");
    R_ASSERT(pMessage.DisplayName == "Message at the beginning of the map", "Incorrect displayname for `message` - maybe it got overwritten?");

    const auto& pSkybox = clss.second.Properties["skybox"];
    R_ASSERT(pSkybox.Type == "string", "`skybox` property is of unexpected type - does it even exist?");
    R_ASSERT(pSkybox.DisplayName == "Skybox texture (cubemap)", "Incorrect skybox for `message` - maybe it got overwritten?");

    const auto& pBaseProp = clss.second.Properties["baseProp"];
    R_ASSERT(pBaseProp.Type == "integer", "`baseProp` property is of unexpected type - does it even exist?");

    const auto& pMainProp = clss.second.Properties["mainProp"];
    R_ASSERT(pMainProp.Type == "integer", "`mainProp` property is of unexpected type - does it even exist?");
}
