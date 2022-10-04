#include "main.hpp"

#include "Decay/Common.hpp"
#include "cxxopts.hpp"

#include "Decay/Wad/Wad3/WadFile.hpp"

#include "util.hpp"

#pragma region --wad_add
cxxopts::Options Options_wad_add(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "wad_add" : argv[0], "Add texture(s) into WAD");

    options.add_options("Input")
       ("f,file", "WAD file", cxxopts::value<std::string>(), "<file.wad>")
    ;
    options.add_options("Add")
       ("texture", "Textures to add (filesystem paths)", cxxopts::value<std::vector<std::string>>(), "<texture.png>")
       ("image", "Images to add (filesystem paths)", cxxopts::value<std::vector<std::string>>(), "<image.png>")
       ("font_atlas", "Images to add (filesystem paths)", cxxopts::value<std::vector<std::string>>(), "<font_atlas.png>")
    ;
    options.add_options("Output")
       ("output", "Result WAD file", cxxopts::value<std::vector<std::string>>(), "<output.wad>")
    ;

    options.positional_help("-f <file.wad> <textures...");
    options.parse_positional("texture");

    options.set_width(200);
    return options;
}
int Help_wad_add(int argc, const char** argv)
{
    std::cout << Options_wad_add(argc, argv).help({ "Input", "Add", "Output" }) << std::endl;
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
                try
                {
                    int charWidth = 16; //TODO Add cxxopts option

                    WadFile::Font font{};
                    {
                        WadFile::Image fontImage = WadFile::Image::FromFile(tPath);
                        font.Width = fontImage.Width;
                        font.Height = fontImage.Height;
                        R_ASSERT(font.Width == 256, "Font image must have width 256");
                        R_ASSERT(font.Height % 16 == 0, "Font image must have width 256");
                        font.Data = std::move(fontImage.Data);
                        font.Palette = std::move(fontImage.Palette);
                    }
                    {
                        font.RowCount = 16;
                        font.RowHeight = font.Height / 16;

                        for(int y = 0, i = 0; y < 16; y++)
                        {
                            for(int x = 0; x < 16; x++, i++)
                            {
                                font.Characters[i].Width = charWidth;
                                font.Characters[i].Offset = (y * font.Width * font.RowHeight) + (x * charWidth);
                            }
                        }
                    }
                    fonts[tPath.filename().replace_extension().string()] = font;
                }
                catch(std::runtime_error& ex)
                {
                    std::cerr << "Failed to font atlas from '" << tPath << "' - " << ex.what() << std::endl;
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
        WadFile::AddToFile(wadPath, textures, fonts, images); //TODO output WAD
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

#pragma region --wad
cxxopts::Options Options_wad(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "wad" : argv[0], "Info and dumping WAD");

    options.add_options("Input")
       ("f,file", "WAD file", cxxopts::value<std::string>(), "<file.wad>")
    ;
    options.add_options("Output")
       ("info", "List information and content of the file")
       ("dump", "Dump content into the directory", cxxopts::value<std::vector<std::string>>(), "<directory>")
    ;

    options.positional_help("-f <file.wad> ...");

    options.set_width(200);
    return options;
}
int Help_wad(int argc, const char** argv)
{
    std::cout << Options_wad(argc, argv).help({ "Input", "Output" }) << std::endl;
    return 0;
}
int Exec_wad(int argc, const char** argv)
{
    auto options = Options_wad(argc, argv);
    auto result = options.parse(argc, argv);

    using namespace Decay::Wad::Wad3;

#pragma region --file
    WadFile wad{};
    {
        std::filesystem::path wadPath;
        if(GetFilePath_NewOrOverride(result, "file", wadPath, ".wad"))
        {
            wad = WadFile(wadPath);
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --info
    if(result.count("info"))
    {
        std::cout << "Entries: " << wad.GetItems().size() << std::endl;

        std::cout << "Textures: " << wad.GetTextureCount() << std::endl;
        for(const auto& kv : wad.ReadAllTextures_Map())
            std::cout << "\t- " << kv.first << " (" << kv.second.Width << " x " << kv.second.Height << ")" << std::endl;

        std::cout << "Images: " << wad.GetImageCount() << std::endl;
        for(const auto& kv : wad.ReadAllImages_Map())
            std::cout << "\t- " << kv.first << " (" << kv.second.Width << " x " << kv.second.Height << ")" << std::endl;

        std::cout << "Fonts: " << wad.GetFontCount() << std::endl;
        for(const auto& kv : wad.ReadAllFonts_Map())
            std::cout << "\t- " << kv.first << " (" << kv.second.Width << " x " << kv.second.Height << ")" << std::endl;
    }
#pragma endregion

#pragma region --dump
    if(result.count("dump"))
    {
        //TODO
    }
#pragma endregion

    return 0;
}
#pragma endregion
