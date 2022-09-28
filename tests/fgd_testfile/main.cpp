#include "Decay/Fgd/FgdFile.hpp"

using namespace Decay::Fgd;

int main(int argc, const char* argv[])
{
    if(argc < 2)
        throw std::runtime_error("Please provide path to FGD file");
    std::cout << argv[1] << std::endl;

    std::fstream in = std::fstream(argv[1], std::ios_base::in);
    FgdFile fgd(in);

    if(std::string("file.fgd") == argv[1])
    {
        R_ASSERT(fgd.IncludeFiles.size() == 1);
        R_ASSERT(std::find(fgd.IncludeFiles.begin(), fgd.IncludeFiles.end(), "base.fgd") != fgd.IncludeFiles.end());

        R_ASSERT(fgd.Classes.size() == 7);
        R_ASSERT(fgd.Classes.contains("info_player_spawn"));
        FgdFile::Class& clss = fgd.Classes["info_player_spawn"];
        R_ASSERT(clss.Type == "PointClass");
        R_ASSERT(clss.Options.size() == 1);
        R_ASSERT(clss.Options[0].Name == "base");
        R_ASSERT(clss.Options[0].Params.size() == 1);
        R_ASSERT(clss.Options[0].Params[0].Name == "Humanoid");
        R_ASSERT(clss.Options[0].Params[0].Quoted == false);
        R_ASSERT(clss.Description == "Player spawn point. You should have some for each team (except for deathmatch maps).");
        R_ASSERT(clss.Properties.size() == 1);
        R_ASSERT(clss.Properties["team"].Codename == "team");
        R_ASSERT(clss.Properties["team"].Type == "choices");
        R_ASSERT(clss.Properties["team"].DisplayName == "Player team (default is deathmatch)");
        R_ASSERT(clss.Properties["team"].DefaultValue == "0");
        R_ASSERT(clss.Properties["team"].FlagsOrChoices.size() == 3);
    }
    else if(std::string("base.fgd") == argv[1])
    {
        R_ASSERT(fgd.AutoVisGroups.size() == 1);
        R_ASSERT(fgd.AutoVisGroups[0].DisplayName == "Humanoid");
        R_ASSERT(fgd.AutoVisGroups[0].Child.size() == 2);
        R_ASSERT(fgd.AutoVisGroups[0].Child[0].EntityClasses.size() == 1);
        R_ASSERT(fgd.AutoVisGroups[0].Child[1].EntityClasses.size() == 1);

        R_ASSERT(fgd.MaterialExclusion.size() == 2);
        R_ASSERT(fgd.MaterialExclusion.contains("debug"));
        R_ASSERT(fgd.MaterialExclusion.contains("bin"));

        R_ASSERT(fgd.MapSize.has_value());
        R_ASSERT(fgd.MapSize.value() == glm::i32vec2(-512, 512));
    }
    else
        std::cerr << "Unknown testing FGD file" << std::endl;

    std::cout << fgd;

    {
        std::fstream out = std::fstream(argv[1] + std::string("_out"), std::ios_base::out);
        out << fgd;
    }
}
