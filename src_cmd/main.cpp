#include "main.hpp"

#define COMMAND(a_name, a_description) \
    {\
        #a_name,\
        Command {\
            Exec_##a_name,\
            Help_##a_name,\
            a_description\
        }\
    }

std::unordered_map<std::string, Command> Commands = {
    COMMAND(help, "Show this help"),
    COMMAND(bsp2obj, "Extract OBJ (model) from BSP (map), including packed textures"),
    COMMAND(bsp2wad, "Extracts textures from BSP to WAD"),
    COMMAND(wad_add, "Add textures to WAD"),
    COMMAND(wad, "Info and dumping WAD"),
    COMMAND(bsp_lightmap, "Extracts lightmap texture"),
    COMMAND(bsp_entity, "Manipulate BSP entities"),
    COMMAND(map2rmf, "Convert MAP to RMF format (in-development map)"),
    COMMAND(rmf2map, "Convert RMf to MAP format (in-development map)"),
    COMMAND(fgd, "Manipulate FGD files (entity definitions)")
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

            R_ASSERT(!command.Help_Description.empty(), "Command help description cannot be empty");
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
