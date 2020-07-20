#include "main.hpp"

#include <Decay/Common.hpp>

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
        if(std::filesystem::exists(wadFilename) && !std::filesystem::is_regular_file(wadFilename))
        {
            std::cerr << "WAD file path exists but does not refer to valid file" << std::endl;
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
            std::cerr << ex.what() << std::endl;
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
