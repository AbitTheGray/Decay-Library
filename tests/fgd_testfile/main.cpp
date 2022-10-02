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
        R_ASSERT(fgd.IncludeFiles.size() == 1, "file.fgd should contain only 1 `@Include`");
        R_ASSERT(std::find(fgd.IncludeFiles.begin(), fgd.IncludeFiles.end(), "base.fgd") != fgd.IncludeFiles.end(), "file.fgd should contain \"base.fgd\"");

        R_ASSERT(fgd.Classes.size() == 7, "file.fgd should contain exactly 7 classes");
        R_ASSERT(fgd.Classes.contains("info_player_spawn"), "file.fgd does not contain \"info_player_spawn\"");
        FgdFile::Class& clss = fgd.Classes["info_player_spawn"];
        R_ASSERT(clss.Type == "PointClass", "info_player_spawn should be @PointClass");
        R_ASSERT(clss.Options.size() == 1, "info_player_spawn should have exactly 1 option");
        R_ASSERT(clss.Options[0].Name == "base", "info_player_spawn should have base option");
        R_ASSERT(clss.Options[0].Params.size() == 1, "info_player_spawn's base() option should have 1 parameter");
        R_ASSERT(clss.Options[0].Params[0].Name == "Humanoid", "info_player_spawn's base parameter should be \"Humanoid\"");
        R_ASSERT(clss.Options[0].Params[0].Quoted == false, "info_player_spawn's base parameter \"Humanoid\" should not be quoted");
        R_ASSERT(clss.Description == "Player spawn point. You should have some for each team (except for deathmatch maps).", "");
        R_ASSERT(clss.Properties.size() == 1, "info_player_spawn should have only 1 property");
        R_ASSERT(clss.Properties["team"].Codename == "team", "info_player_spawn does not have \"team\" property");
        R_ASSERT(clss.Properties["team"].Type == "choices", "info_player_spawn's \"team\" property is of incorrect type");
        R_ASSERT(clss.Properties["team"].DisplayName == "Player team (default is deathmatch)", "info_player_spawn's \"team\" property has incorrect display name");
        R_ASSERT(clss.Properties["team"].DefaultValue == "0", "info_player_spawn's \"team\" property has incorrect default value");
        R_ASSERT(clss.Properties["team"].FlagsOrChoices.size() == 3, "info_player_spawn's \"team\" property should have 3 choices");
    }
    else if(std::string("base.fgd") == argv[1])
    {
        R_ASSERT(fgd.AutoVisGroups.size() == 1, "base.fgd should have only 1 AutoVisGroup");
        R_ASSERT(fgd.AutoVisGroups[0].DisplayName == "Humanoid", "base.fgd's AutoVisGroup should be \"Humanoid\"");
        R_ASSERT(fgd.AutoVisGroups[0].Child.size() == 2, "base.fgd's AutoVisGroup \"Humanoid\" should have only 2 children");
        R_ASSERT(fgd.AutoVisGroups[0].Child[0].EntityClasses.size() == 1, "base.fgd's AutoVisGroup children should have only 1 value");
        R_ASSERT(fgd.AutoVisGroups[0].Child[1].EntityClasses.size() == 1, "base.fgd's AutoVisGroup children should have only 1 value");

        R_ASSERT(fgd.MaterialExclusion.size() == 2, "base.fgd should have exactly 2 material exclusions");
        R_ASSERT(fgd.MaterialExclusion.contains("debug"), "base.fgd should contain \"debug\" in its material exclusion");
        R_ASSERT(fgd.MaterialExclusion.contains("bin"), "base.fgd should contain \"bin\" in its material exclusion");

        R_ASSERT(fgd.MapSize.has_value(), "base.fgd should have @MapSize");
        R_ASSERT(fgd.MapSize.value() == glm::i32vec2(-512, 512), "base.fgd @MapSize value shoud be -512 to 512");
    }
    else
        std::cerr << "Unknown testing FGD file" << std::endl;

    std::cout << fgd;

    {
        std::fstream out = std::fstream(argv[1] + std::string("_out"), std::ios_base::out);
        out << fgd;
    }
}
