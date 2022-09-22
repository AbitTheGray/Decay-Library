#include "main.hpp"

#include "Decay/Common.hpp"
#include "cxxopts.hpp"

#pragma region wad_add
cxxopts::Options Options_wad_add(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "wad_add" : argv[0], "Add texture(s) into WAD");

    options.add_options("Input")
       ("f,file", "WAD file", cxxopts::value<std::string>(), "<file.wad>")
    ;
    options.add_options("Output")
       ("textures", "Textures to add (filesystem paths)", cxxopts::value<std::vector<std::string>>(), "<texture.png>")
        //TODO images
        //TODO fonts
    ;

    options.positional_help("-f <file.wad> <textures...");
    options.parse_positional("textures");
    return options;
}
int Help_wad_add(int argc, const char** argv)
{
    std::cout << Options_wad_add(argc, argv).help({ "Input", "Output" }) << std::endl;
    std::cout << "Does not parse textures/content in the WAD, only WAD header" << std::endl;
    return 0;
}
int Exec_wad_add(int argc, const char** argv)
{
    auto options = Options_wad_add(argc, argv);
    auto result = options.parse(argc, argv);

#pragma region --file
    if(!result.count("file"))
    {
        const char* errorMsg = "You need to specify input file, use `--file path/to/file.wad`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }
    std::filesystem::path wadPath;
    {
        wadPath = result["file"].as<std::string>();
        if(std::filesystem::exists(wadPath) && !std::filesystem::is_regular_file(wadPath))
        {
            const char* errorMsg = "`--file` must point to a valid existing file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(wadPath.extension() != ".wad")
            std::cerr << "WARNING: WAD file path should have `.wad` extension" << std::endl;
    }
#pragma endregion

    using namespace Decay::Wad::Wad3;

#pragma region --textures
    std::vector<WadFile::Texture> textures = {};
    if(result.count("textures"))
    {
        std::vector<std::string> texturePaths = result["textures"].as<std::vector<std::string>>();

#pragma region Add textures
        {
            textures.reserve(texturePaths.size());
            for(std::filesystem::path tPath : texturePaths)
            {
                if(tPath.empty())
                    continue;
                if(!std::filesystem::exists(tPath))
                {
                    std::string errorMsg = "Texture file '" + std::string(tPath) + "' not found";
#ifdef DEBUG
                    throw std::runtime_error(errorMsg);
#else
                    std::cerr << errorMsg << std::endl;
#endif
                    return 1;
                }
                if(!std::filesystem::is_regular_file(tPath))
                {
                    std::string errorMsg = "Texture file '" + std::string(tPath) + "' does not refer to valid file";
#ifdef DEBUG
                    throw std::runtime_error(errorMsg);
#else
                    std::cerr << errorMsg << std::endl;
#endif
                    return 1;
                }

                try
                {
                    auto texture = WadFile::Texture::FromFile(tPath);
                    textures.emplace_back(texture);
                }
                catch(std::runtime_error& ex)
                {
                    std::cerr << "Failed to load texture from '" << tPath << "' - " << ex.what() << std::endl;
                    continue;
                }
            }
        }
#pragma endregion
    }
#pragma endregion

    if(!textures.empty())
    {
        try
        {
            //TODO images
            //TODO fonts
            WadFile::AddToFile(wadPath, textures, {}, {});
        }
        catch(std::runtime_error& ex)
        {
            std::cerr << "Failed to add textures into WAD file" << std::endl;
            return 1;
        }

        std::cout << "Textures successfully added into WAD file" << std::endl;
    }
    else
    {
        std::cout << "No textures to add into WAD file" << std::endl;
    }
    return 0;
}
#pragma endregion
