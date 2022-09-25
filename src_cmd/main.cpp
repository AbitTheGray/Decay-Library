#include "main.hpp"

#include "Decay/Common.hpp"

std::unordered_map<std::string, Command> Commands = {
    {
        "help",
        Command{
            Exec_help,
            Exec_help,
            "Show this help"
        }
    },
    {
        "bsp2obj",
        Command{
            Exec_bsp2obj,
            Help_bsp2obj,
            "Extract OBJ (model) from BSP (map), including packed textures"
        }
    },
    {
        "bsp2wad",
        Command{
            Exec_bsp2wad,
            Help_bsp2wad,
            "Extracts textures from BSP to WAD"
        }
    },
    {
        "wad_add",
        Command{
            Exec_wad_add,
            Help_wad_add,
            "Add textures to WAD"
        }
    },
    {
        "bsp_lightmap",
        Command{
            Exec_bsp_lightmap,
            Help_bsp_lightmap,
            "Extracts lightmap texture"
        }
    }
};

int main(int argc, const char** argv)
{
    if(argc == 1) // Only program name
        return Exec_help(0, nullptr);

    auto cmd_it = Commands.find(argv[1]);
    if(cmd_it == Commands.end())
    {
        std::cerr << "Command not found" << std::endl;
        return 1;
    }
    else
    {
        return cmd_it->second.Exec(argc - 1, argv + 1);
    }
}

static const char* ansi_reset = "\033[m";
static const char* ansi_commandName = "\033[1m";
static const char* ansi_description = "\033[36m";

int Exec_help(int argc, const char** argv)
{
    if(argc < 2)
    {
        // Standard basic help
        for(auto& it : Commands)
        {
            auto& command = it.second;
            std::cout << "\t" << ansi_commandName << it.first << ansi_reset;

            assert(!command.Help_Description.empty());
            std::cout << "\t\t" << ansi_description << command.Help_Description << ansi_reset << std::endl;
        }

        return 0;
    }
    else // Detailed per-command help
    {
        for(auto& it : Commands)
            if(argv[1] == it.first)
                return it.second.HelpExec(argc - 1, argv + 1);

        std::cerr << "Failed to find requested command - are you using latest version?" << std::endl;
        return 1;
    }
}
