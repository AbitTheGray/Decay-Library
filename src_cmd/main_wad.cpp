#include "main.hpp"

#include "Decay/Common.hpp"
#include "cxxopts.hpp"

#include "Decay/Wad/Wad3/WadFile.hpp"

#include "util.hpp"

#pragma region wad_add
cxxopts::Options Options_wad_add(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "wad_add" : argv[0], "Add texture(s) into WAD");

    options.add_options("Input")
       ("f,file", "WAD file", cxxopts::value<std::string>(), "<file.wad>")
    ;
    options.add_options("Output")
       ("texture", "Textures to add (filesystem paths)", cxxopts::value<std::vector<std::string>>(), "<texture.png>")
       ("image", "Images to add (filesystem paths)", cxxopts::value<std::vector<std::string>>(), "<image.png>")
       ("font_atlas", "Images to add (filesystem paths)", cxxopts::value<std::vector<std::string>>(), "<font_atlas.png>")
    ;

    options.positional_help("-f <file.wad> <textures...");
    options.parse_positional("texture");

    options.set_width(200);
    return options;
}
int Help_wad_add(int argc, const char** argv)
{
    std::cout << Options_wad_add(argc, argv).help({ "Input", "Output" }) << std::endl;
    std::cout << "All files without `--image` or `--font_atlas` are added as textures (you can also explicitly use `--texture`." << std::endl;
    std::cout << "Does not parse textures/content in the WAD, only WAD header." << std::endl;
    return 0;
}
int Exec_wad_add(int argc, const char** argv)
{
    auto options = Options_wad_add(argc, argv);
    auto result = options.parse(argc, argv);

#pragma region --file
    std::filesystem::path wadPath;
    if(GetFilePath_NewOrOverride(result, "file", wadPath, ".wad"))
    {

    }
    else
        return 1;
#pragma endregion

    using namespace Decay::Wad::Wad3;

#pragma region --textures
    std::vector<WadFile::Texture> textures = {};
    if(result.count("texture"))
    {
        std::vector<std::filesystem::path> texturePaths{};
        if(GetFilePath_Existing(result, "texture", texturePaths, ".png"))
        {
            textures.reserve(texturePaths.size());
            for(const auto& tPath : texturePaths)
            {
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
        else
            return 1;
    }
#pragma endregion

#pragma region --images
    std::map<std::string, WadFile::Image> images = {};
    if(result.count("image"))
    {
        std::vector<std::filesystem::path> imagePaths{};
        if(GetFilePath_Existing(result, "image", imagePaths, ".png"))
        {
            for(const auto& tPath : imagePaths)
            {
                try
                {
                    auto image = WadFile::Image::FromFile(tPath);
                    images[tPath.filename().replace_extension().string()] = image;
                }
                catch(std::runtime_error& ex)
                {
                    std::cerr << "Failed to load image from '" << tPath << "' - " << ex.what() << std::endl;
                    continue;
                }
            }
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --font_atlas
    std::map<std::string, WadFile::Font> fonts = {};
    if(result.count("font_atlas"))
    {
        std::vector<std::filesystem::path> fontPaths{};
        if(GetFilePath_Existing(result, "font_atlas", fontPaths, ".png"))
        {
            for(const auto& tPath : fontPaths)
            {
                throw std::runtime_error("--font_atlas is not implemented");
                try
                {
                    /*
                    auto font = WadFile::Font::FromFile(tPath);
                    fonts[tPath.filename().replace_extension().string()] = font;
                    */
                }
                catch(std::runtime_error& ex)
                {
                    std::cerr << "Failed to font atlasfrom '" << tPath << "' - " << ex.what() << std::endl;
                    continue;
                }
            }
        }
        else
            return 1;
    }
#pragma endregion

    if(textures.empty() && images.empty() && fonts.empty())
    {
        std::cerr << "No textures, images or fonts to add into WAD file" << std::endl;
        return 1;
    }
    try
    {
        WadFile::AddToFile(wadPath, textures, fonts, images);
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "Failed to add textures into WAD file" << std::endl;
        return 1;
    }

    std::cout << "All successfully added into WAD file" << std::endl;
    return 0;
}
#pragma endregion
