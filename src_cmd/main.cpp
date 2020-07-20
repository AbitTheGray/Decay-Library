#include "main.hpp"

#include <Decay/Common.hpp>

std::map<std::string, Command> Commands = {
        {
                "help",
                Command{
                        Exec_help,
                        {},
                        "Show this help"
                }
        },
        {
                "bsp2obj",
                Command{
                        Exec_bsp2obj,
                        "<map.bsp> <file.obj> [file.mtl] [textures_dir=`file.mtl`/../textures]",
                        "Extract OBJ (model) from BSP (map), including packed textures"
                }
        },
        {
                "bsp2wad",
                Command{
                        Exec_bsp2wad,
                        "<map.bsp> [map.wad] [new_map.bsp]",
                        "Extracts textures from BSP to WAD"
                }
        },
        {
                "wad_add",
                Command{
                        Exec_wad_add,
                        "<file.wad> <texture...",
                        "Add textures to WAD"
                }
        },
        {
                "bsp_lightmap",
                Command{
                        Exec_bsp_lightmap,
                        "<map.bsp> <lightmap.png>",
                        "Extracts lightmap texture"
                }
        }
};

int main(int argc, const char** argv)
{
    if(argc == 1)
        return Exec_help(0, nullptr);

    auto cmd_it = Commands.find(argv[1]);
    if(cmd_it == Commands.end())
    {
        std::cerr << "Command not found" << std::endl;
        return 1;
    }
    else
    {
        return cmd_it->second.Exec(argc-2, argv+2);
    }
}

static const char* ansi_reset = "\033[m";
static const char* ansi_bold = "\033[1m";
static const char* ansi_description = "\033[36m";

int Exec_help(int argc, const char** argv)
{
    for(auto& it : Commands)
    {
        auto& command = it.second;
        std::cout << "\t" << ansi_bold << it.first << ansi_reset;

        if(!command.Help_Params.empty())
            std::cout << ' ' << command.Help_Params;

        assert(!command.Help_Description.empty());
        std::cout << "\t\t" << ansi_description << command.Help_Description << ansi_reset << std::endl;
    }

    return 0;
}
