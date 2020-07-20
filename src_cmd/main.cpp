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
                "lightmap",
                Command{
                        Exec_lightmap,
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

int Exec_bsp2obj(int argc, const char** argv)
{
    if(argc == 0)
    {
        std::cerr << "No path to BSP provided" << std::endl;
        return 1;
    }

    std::filesystem::path bspFilename(argv[0]);
    {
        if(bspFilename.empty())
        {
            std::cerr << "BSP file path is empty" << std::endl;
            return 1;
        }
        if(bspFilename.extension() != ".bsp")
            std::cerr << "BSP file path does not have .bsp extension" << std::endl;
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

    std::filesystem::path objFilename;
    {
        if(argc == 1)
            objFilename = std::filesystem::path(bspFilename).replace_extension(".obj");
        else
        {
            objFilename = argv[1];
            if(objFilename.empty())
            {
                std::cerr << "OBJ file path is empty" << std::endl;
                return 1;
            }
            if(objFilename.extension() != ".obj")
                std::cerr << "OBJ file path does not have .obj extension" << std::endl;
        }
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

int Exec_bsp2wad(int argc, const char** argv)
{
    if(argc == 0)
    {
        std::cerr << "No path to BSP provided" << std::endl;
        return 1;
    }

    // BSP
    std::filesystem::path bspFilename(argv[0]);
    {
        if(bspFilename.empty())
        {
            std::cerr << "BSP file path is empty" << std::endl;
            return 1;
        }
        if(bspFilename.extension() != ".bsp")
            std::cerr << "BSP file path does not have .bsp extension" << std::endl;
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

    // WAD
    std::filesystem::path wadFilename;
    {
        if(argc == 1)
            wadFilename = std::filesystem::path(bspFilename).replace_extension(".wad");
        else
        {
            wadFilename = argv[1];

            if(wadFilename.empty())
            {
                std::cerr << "BSP file path is empty" << std::endl;
                return 1;
            }
            if(wadFilename.extension() != ".wad")
                std::cerr << "WAD file path does not have .wad extension" << std::endl;
        }

        if(std::filesystem::exists(wadFilename) && !std::filesystem::is_regular_file(wadFilename))
        {
            std::cerr << "WAD file exists but does not refer to valid file" << std::endl;
            return 1;
        }
    }

    //TODO BSP without textures
    if(argc >= 3)
        std::cerr << "Creating texture-less BSP is not yet supported" << std::endl;

    using namespace Decay::Bsp;
    using namespace Decay::Wad;

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

    WadFile::AddToFile(wadFilename, bsp->GetTextures(), {}, {});

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
        if(wadFilename.extension() != ".wad")
            std::cerr << "WAD file path does not have .wad extension" << std::endl;
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

int Exec_lightmap(int argc, const char** argv)
{
    if(argc == 0)
    {
        std::cerr << "No path to BSP provided" << std::endl;
        return 1;
    }

    std::filesystem::path bspFilename(argv[0]);
    {
        if(bspFilename.empty())
        {
            std::cerr << "BSP file path is empty" << std::endl;
            return 1;
        }
        if(bspFilename.extension() != ".bsp")
            std::cerr << "BSP file path does not have .bsp extension" << std::endl;
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

    std::filesystem::path exportLight;
    {
        if(argc == 1)
        {
            exportLight = "lightmap.png";
        }
        else
        {
            exportLight = argv[1];
            if(exportLight.empty())
            {
                std::cerr << "Export lightmap path is empty" << std::endl;
                return 1;
            }
        }

        if(std::filesystem::exists(exportLight))
        {
            if(!std::filesystem::is_regular_file(exportLight))
            {
                std::cerr << "Export lightmap path exists but is not a file" << std::endl;
                return 1;
            }
        }
    }

    using namespace Decay;
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

    std::string extension = exportLight.extension();
    assert(extension.size() > 1);
    assert(extension[0] == '.');

    std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec3* data)> writeFunc = ImageWriteFunction_RGB(extension);

    writeFunc(exportLight.c_str(), tree->Light.Width, tree->Light.Height, tree->Light.Data.data());

    return 0;
}
