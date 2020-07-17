#include "main.hpp"

#include "../src/stb/stb_image_write.cpp"

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
                "wad_add",
                Command{
                        Exec_wad_add,
                        "<file.wad> <texture...",
                        "Add textures to WAD"
                }
        },
        {
                "lightmap_extract",
                Command{
                        Exec_lightmap_extract,
                        "<map.bsp> <directory>",
                        "Extracts lightmap textures"
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

int Exec_bsp2obj(int argc, const char** argv)
{
    if(argc == 0)
    {
        std::cerr << "No path to BSP provided" << std::endl;
        return 1;
    }
    if(argc == 1)
    {
        std::cerr << "No output path (OBJ model file) provided" << std::endl;
        return 1;
    }

    std::filesystem::path bspFilename(argv[0]);
    {
        if(bspFilename.empty())
        {
            std::cerr << "BSP file path is empty" << std::endl;
            return 1;
        }
        if(!std::filesystem::exists(bspFilename))
        {
            std::cerr << "BSP file not found" << std::endl;
            return 1;
        }
        if(!std::filesystem::is_regular_file(bspFilename))
        {
            std::cerr << "BSP file path does not refer to valid file" << std::endl;
            return 1;
        }
    }

    std::filesystem::path objFilename(argv[1]);
    {
        if(objFilename.empty())
        {
            std::cerr << "OBJ file path is empty" << std::endl;
            return 1;
        }
        if(objFilename.extension() != ".obj")
            std::cerr << "OBJ file path does not have .obj extension" << std::endl;
    }

    std::filesystem::path mtlFilename;
    {
        if(argc >= 3)
        {
            mtlFilename = argv[2];
            if(mtlFilename.empty())
            {
                std::cerr << "MTL file path is empty" << std::endl;
                return 1;
            }
            if(mtlFilename.extension() != ".mtl")
                std::cerr << "MTL file path does not have .mtl extension" << std::endl;
        }
        else
            mtlFilename = std::filesystem::path(objFilename).replace_extension(".mtl");
    }

    std::filesystem::path texturesDirectory;
    {
        if(argc >= 4)
        {
            texturesDirectory = argv[3];
            if(texturesDirectory.empty())
            {
                std::cerr << "Textures directory path is empty" << std::endl;
                return 1;
            }
            if(!std::filesystem::exists(texturesDirectory))
                std::filesystem::create_directory(texturesDirectory);
            if(!std::filesystem::is_directory(texturesDirectory))
            {
                std::cerr << "Textures directory path does not point to directory" << std::endl;
                return 1;
            }
        }
        else
            texturesDirectory = mtlFilename.parent_path() / "textures";
    }

    using namespace Decay::Bsp;

    std::shared_ptr<BspFile> bsp;
    try
    {
        bsp = std::make_shared<BspFile>(bspFilename);
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "BSP file could not be read" << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    std::shared_ptr<BspTree> tree;

    try
    {
        tree = std::make_shared<BspTree>(bsp);
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "Failed to parse structure from BSP file" << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    try
    {
        tree->ExportFlatObj(
                objFilename,
                std::filesystem::relative(mtlFilename, objFilename.parent_path())
        );
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "OBJ file could not be exported" << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    std::cout << "OBJ saved" << std::endl;

    try
    {
        tree->ExportMtl(
                mtlFilename,
                std::filesystem::relative(texturesDirectory, mtlFilename.parent_path()),
                ".png"
        );
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "MTL file could not be exported" << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    std::cout << "MTL saved" << std::endl;

    try
    {
        tree->ExportTextures(
                texturesDirectory,
                ".png",
                true
        );
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "Failed to export and extract textures" << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    std::cout << "Textures saved" << std::endl;

    return 0;
}

int Exec_wad_add(int argc, const char** argv)
{
    if(argc == 0)
    {
        std::cerr << "No path to WAD provided" << std::endl;
        return 1;
    }
    if(argc == 1)
    {
        std::cerr << "No textures provided" << std::endl;
        return 1;
    }

    std::filesystem::path wadFilename(argv[0]);
    {
        if(wadFilename.empty())
        {
            std::cerr << "WAD file path is empty" << std::endl;
            return 1;
        }
        if(!std::filesystem::exists(wadFilename))
        {
            std::cerr << "WAD file not found" << std::endl;
            return 1;
        }
        if(!std::filesystem::is_regular_file(wadFilename))
        {
            std::cerr << "WAD file path does not refer to valid file" << std::endl;
            return 1;
        }
    }

    using namespace Decay::Wad;

    std::vector<WadFile::Texture> textures(argc-1);
    for(int i = 1; i < argc; i++)
    {
        std::filesystem::path tPath = argv[i];
        if(tPath.empty())
            continue;
        if(!std::filesystem::exists(tPath))
        {
            std::cerr << "Texture file '" << tPath << "' not found" << std::endl;
            return 1;
        }
        if(!std::filesystem::is_regular_file(tPath))
        {
            std::cerr << "Texture file '" << tPath << "' does not refer to valid file" << std::endl;
            return 1;
        }

        try
        {
            auto texture = WadFile::Texture::FromFile(tPath);
            textures.emplace_back(texture);
        }
        catch(std::runtime_error& ex)
        {
            std::cerr << "Failed to load texture from " << tPath << std::endl;
            continue;
        }
    }

    try
    {
        WadFile::AddToFile(wadFilename, textures, {}, {});
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "Failed to add textures to WAD file" << std::endl;
        return 1;
    }

    std::cout << "Textures successfully added to WAD file" << std::endl;
    return 0;
}

int Exec_lightmap_extract(int argc, const char** argv)
{
    if(argc == 0)
    {
        std::cerr << "No path to BSP provided" << std::endl;
        return 1;
    }
    if(argc == 1)
    {
        std::cerr << "No textures export directory provided" << std::endl;
        return 1;
    }

    std::filesystem::path bspFilename(argv[0]);
    {
        if(bspFilename.empty())
        {
            std::cerr << "BSP file path is empty" << std::endl;
            return 1;
        }
        if(!std::filesystem::exists(bspFilename))
        {
            std::cerr << "BSP file not found" << std::endl;
            return 1;
        }
        if(!std::filesystem::is_regular_file(bspFilename))
        {
            std::cerr << "BSP file path does not refer to valid file" << std::endl;
            return 1;
        }
    }

    std::filesystem::path exportDir(argv[1]);
    {
        if(exportDir.empty())
        {
            std::cerr << "Export directory path is empty" << std::endl;
            return 1;
        }

        if(std::filesystem::exists(exportDir))
        {
            if(!std::filesystem::is_directory(exportDir))
            {
                std::cerr << "Export directory path exists but is not a directory" << std::endl;
                return 1;
            }
        }
        else // ! exists(exportDir)
        {
            if(!std::filesystem::create_directory(exportDir))
            {
                std::cerr << "Export directory could not be created" << std::endl;
                return 1;
            }
        }
    }

    using namespace Decay::Bsp;

    std::shared_ptr<BspFile> bsp;
    try
    {
        bsp = std::make_shared<BspFile>(bspFilename);
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "BSP file could not be read" << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    std::shared_ptr<BspTree> tree;

    try
    {
        tree = std::make_shared<BspTree>(bsp);
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "Failed to parse structure from BSP file" << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    for(auto& lightmap : tree->Lightmaps)
    {
        std::stringstream ss;
        ss << "light_" << lightmap->Index << ".png";
        std::filesystem::path dstFile(exportDir / ss.str());
        stbi_write_png(dstFile.c_str(), lightmap->Width, lightmap->Height, 3, lightmap->Data.data(), lightmap->Width * 3);
    }

    return 0;
}
