#include "main.hpp"

#include "Decay/Common.hpp"

#pragma region bsp2obj
int Help_bsp2obj(int argc, const char** argv)
{
    if(argc == 0)
        std::cout << "bsp2obj ";
    else
        std::cout << argv[0] << ' ';
    std::cout << "<map.bsp> [file.obj] [file.mtl] [textures_dir=`file.mtl`/../textures]" << std::endl;

    std::cout << "Converts BSP to OBJ with optional material file and texture export." << std::endl;
    std::cout << "Textures mentioned but not packed inside the BSP will contain placeholder data." << std::endl;
    return 0;
}
int Exec_bsp2obj(int argc, const char** argv)
{
    if(argc <= 1) // Only script name
    {
        std::cerr << "No path to BSP provided" << std::endl;
        return 1;
    }

    std::filesystem::path bspFilename(argv[1]);
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
        if(argc == 2)
            objFilename = std::filesystem::path(bspFilename).replace_extension(".obj");
        else
        {
            objFilename = argv[2];
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
        if(argc >= 4)
        {
            mtlFilename = argv[3];
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
        if(argc >= 5)
        {
            texturesDirectory = argv[4];
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

    using namespace Decay::Bsp::v30;

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
#pragma endregion

#pragma region bsp2wad
int Help_bsp2wad(int argc, const char** argv)
{
    if(argc == 0)
        std::cout << "bsp2wad ";
    else
        std::cout << argv[0] << ' ';
    std::cout << "<map.bsp> [map.wad] [new_map.bsp]" << std::endl;

    std::cout << "Extracts textures from BSP to WAD." << std::endl;
    std::cout << "If `new_map.bsp` is supplied, new BSP is created without those textures (only referenced, not packed). It won't reference the new WAD file." << std::endl;
    return 0;
}
int Exec_bsp2wad(int argc, const char** argv)
{
    if(argc <= 1) // Only script name
    {
        std::cerr << "No path to BSP provided" << std::endl;
        return 1;
    }

    // BSP
    std::filesystem::path bspFilename(argv[1]);
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
        if(argc == 2)
            wadFilename = std::filesystem::path(bspFilename).replace_extension(".wad");
        else
        {
            wadFilename = argv[2];

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

    using namespace Decay::Bsp::v30;
    using namespace Decay::Wad::Wad3;

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

    auto textures = bsp->GetTextures();

    auto count = WadFile::AddToFile(wadFilename, textures, {}, {});
    if(count == 0)
        std::cerr << "No textures to add" << std::endl;
    else
        std::cout << "Added " << count << " textures to " << wadFilename << std::endl;


    // BSP without packed textures
    if(argc >= 4)
    {
        std::filesystem::path outBspFilename(argv[3]);
        {
            if(outBspFilename.empty())
            {
                std::cerr << "Output BSP file path is empty" << std::endl;
                return 1;
            }
            if(outBspFilename.extension() != ".bsp")
                std::cerr << "Output BSP file path does not have .bsp extension" << std::endl;
            if(std::filesystem::exists(outBspFilename) && !std::filesystem::is_regular_file(outBspFilename))
            {
                std::cerr << "Output BSP file exists but does not refer to valid file" << std::endl;
                return 1;
            }
        }

        for(std::size_t i = 0; i < textures.size(); i++)
            textures[i] = textures[i].CopyWithoutData();

        bsp->SetTextures(textures);

        bsp->Save(outBspFilename);

        std::cout << "Saved BSP without packed textures to " << outBspFilename << std::endl;
    }

    return 0;
}
#pragma endregion

#pragma region bsp_lightmap
int Help_bsp_lightmap(int argc, const char** argv)
{
    if(argc == 0)
        std::cout << "bsp_lightmap ";
    else
        std::cout << argv[0] << ' ';
    std::cout << "<map.bsp> [lightmap.png]" << std::endl;

    std::cout << "Extracts per-face lightmap and packs them into few big lightmaps." << std::endl;
    std::cout << "Big lightmap(s) have \"holes\" (unused pixels)." << std::endl;
    return 0;
}
int Exec_bsp_lightmap(int argc, const char** argv)
{
    if(argc <= 1) // Only script name
    {
        std::cerr << "No path to BSP provided" << std::endl;
        return 1;
    }

    std::filesystem::path bspFilename(argv[1]);
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
        if(argc == 2)
        {
            exportLight = "lightmap.png";
        }
        else
        {
            exportLight = argv[2];
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
    using namespace Decay::Bsp::v30;

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
#pragma endregion
