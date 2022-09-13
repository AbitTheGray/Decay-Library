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
        assert(fgd.IncludeFiles.size() == 1);
        assert(fgd.IncludeFiles.contains("base.fgd"));

        assert(fgd.Classes.size() == 7);
        FgdFile::Class* clss = nullptr;
        for(FgdFile::Class& c : fgd.Classes)
        {
            if(c.Codename == "info_player_spawn")
            {
                clss = &c;
                break;
            }
        }
        assert(clss != nullptr);
        assert(clss->Type == "PointClass");
        assert(clss->Options.size() == 1);
        assert(clss->Options[0].Name == "base");
        assert(clss->Options[0].Params.size() == 1);
        assert(clss->Options[0].Params[0].Name == "Humanoid");
        assert(clss->Options[0].Params[0].Quoted == false);
        assert(clss->Description == "Player spawn point. You should have some for each team (except for deathmatch maps).");
        assert(clss->Properties.size() == 1);
        assert(clss->Properties[0].Codename == "team");
        assert(clss->Properties[0].Type == "choices");
        assert(clss->Properties[0].DisplayName == "Player team (default is deathmatch)");
        assert(clss->Properties[0].DefaultValue == "0");
        assert(clss->Properties[0].FlagsOrChoices.size() == 3);
    }
    else if(std::string("base.fgd") == argv[1])
    {
        assert(fgd.AutoVisGroups.size() == 1);
        assert(fgd.AutoVisGroups[0].DisplayName == "Humanoid");
        assert(fgd.AutoVisGroups[0].Child.size() == 2);
        assert(fgd.AutoVisGroups[0].Child[0].EntityClasses.size() == 1);
        assert(fgd.AutoVisGroups[0].Child[1].EntityClasses.size() == 1);

        assert(fgd.MaterialExclusion.size() == 2);
        assert(fgd.MaterialExclusion.contains("debug"));
        assert(fgd.MaterialExclusion.contains("bin"));

        assert(fgd.MapSize.has_value());
        assert(fgd.MapSize.value() == glm::i32vec2(-512, 512));
    }
    else
        std::cerr << "Unknown testing FGD file" << std::endl;

    std::cout << fgd;

    {
        std::fstream out = std::fstream(argv[1] + std::string("_out"), std::ios_base::out);
        out << fgd;
    }
}
