#include "main.hpp"

#include "cxxopts.hpp"

#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspTree.hpp"

#include "Decay/Fgd/FgdFile.hpp"

#pragma region bsp2obj
cxxopts::Options Options_bsp2obj(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "bsp2obj" : argv[0], "Conversion from BSP to 3D model (Wavefront OBJ)");

    options.add_options("Input")
       ("f,file", "BSP file (the map)", cxxopts::value<std::string>(), "<map.bsp>")
    ;
    options.add_options("Output")
       ("obj", "OBJ file (result 3D model)", cxxopts::value<std::string>(), "<map.obj>")
       ("mtl", "MTL file (texture mapping for OBJ file)", cxxopts::value<std::string>(), "<map.mtl>")
       ("textures", "Export textures to directory", cxxopts::value<std::string>(), "<texture_directory>")
    ;

    return options;
}
int Help_bsp2obj(int argc, const char** argv)
{
    std::cout << Options_bsp2obj(argc, argv).help({"Input", "Output"}) << std::endl;
    std::cout << "OBJ file format: https://en.wikipedia.org/wiki/Wavefront_.obj_file" << std::endl;
    return 0;
}
int Exec_bsp2obj(int argc, const char** argv)
{
    auto options = Options_bsp2obj(argc, argv);
    auto result = options.parse(argc, argv);

#pragma region --file
    if(!result.count("file"))
    {
        const char* errorMsg = "You need to specify input file, use `--file path/to/map.bsp`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }
    using namespace Decay::Bsp::v30;
    std::shared_ptr<BspFile> bsp;
    std::shared_ptr<BspTree> bspTree;
    {
        std::filesystem::path bspPath = result["file"].as<std::string>();
        if(!std::filesystem::exists(bspPath) || !std::filesystem::is_regular_file(bspPath))
        {
            const char* errorMsg = "`--file` must point to a valid file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(bspPath.extension() != ".bsp")
            std::cerr << "WARNING: BSP file path should have `.bsp` extension" << std::endl;
        try
        {
            bsp = std::make_shared<BspFile>(bspPath);
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "Failed to read/parse BSP file - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }
        try
        {
            bspTree = std::make_shared<BspTree>(bsp);
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "Failed to parse BSP file into high-level structure (BspTree) - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }
    }
#pragma endregion

#pragma region --obj
    std::filesystem::path objPath = {};
    if(result.count("obj"))
    {
        objPath = result["obj"].as<std::string>();
        if(std::filesystem::exists(objPath) && !std::filesystem::is_regular_file(objPath))
        {
            const char* errorMsg = "`--obj` must point to non-existing file or existing valid file (if you do not want to export as OBJ into a file, omit `--obj`)";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(objPath.extension() != ".obj")
            std::cerr << "WARNING: OBJ file path should have `.obj` extension" << std::endl;
    }
#pragma endregion

#pragma region --mtl
    std::filesystem::path mtlPath = {};
    if(result.count("mtl"))
    {
        mtlPath = result["mtl"].as<std::string>();
        if(std::filesystem::exists(mtlPath) && !std::filesystem::is_regular_file(mtlPath))
        {
            const char* errorMsg = "`--mtl` must point to non-existing file or existing valid file (if you do not want to export texture mapping for OBJ into a file, omit `--mtl`)";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(mtlPath.extension() != ".mtl")
            std::cerr << "WARNING: MTL file path should have `.mtl` extension" << std::endl;
    }
#pragma endregion

#pragma region --textures
    std::filesystem::path texturesDir = {};
    if(result.count("textures"))
    {
        texturesDir = result["textures"].as<std::string>();
        if(std::filesystem::exists(texturesDir))
        {
            if(!std::filesystem::is_directory(texturesDir))
            {
                const char* errorMsg = "`--textures` must point to a valid directory or path where a directory can be created";
#ifdef DEBUG
                throw std::runtime_error(errorMsg);
#else
                std::cerr << errorMsg << std::endl;
#endif
                return 1;
            }
        }
        else // !exists(texturesDir)
        {
            try
            {
                std::filesystem::create_directories(texturesDir);
            }
            catch(...)
            {
                std::cerr << "Failed to create directory for textures, they won't be exported." << std::endl;
                texturesDir = std::filesystem::path();
            }
        }
    }
#pragma endregion

#pragma region OBJ export
    if(!objPath.empty())
    {
        try
        {
            bspTree->ExportFlatObj(
                objPath,
                mtlPath.empty() ? std::filesystem::path{} : std::filesystem::relative(mtlPath, objPath.parent_path())
            );
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "`--obj` could not be exported - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }
    }
#pragma endregion

#pragma region MTL export
    if(!mtlPath.empty())
    {
        try
        {
            bspTree->ExportMtl(
                mtlPath,
                texturesDir.empty() ? std::filesystem::path() : std::filesystem::relative(texturesDir, mtlPath.parent_path()),
                ".png"
            );
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "`--mtl` could not be exported - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }
    }
#pragma endregion

#pragma region Texture export
    if(!texturesDir.empty())
    {
        try
        {
            bspTree->ExportTextures(
                texturesDir,
                ".png",
                true
            );
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "Failed to extract/export textures - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }
    }
#pragma endregion

    return 0;
}
#pragma endregion

#pragma region bsp2wad
cxxopts::Options Options_bsp2wad(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "bsp2wad" : argv[0], "Extract textures from BSP into WAD");

    options.add_options("Input")
       ("f,file", "BSP file (the map)", cxxopts::value<std::string>(), "<map.bsp>")
    ;
    options.add_options("Output")
       ("wad", "WAD file (where to put textures)", cxxopts::value<std::string>(), "<map.wad>")
       ("newbsp", "BSP file for map without packed textures", cxxopts::value<std::string>(), "<map.bsp>")
       ("newbspwad", "How to mention the new WAD in `--newbsp` BSP (omitting this will not add any WAD into the map)", cxxopts::value<std::string>(), R"(<\half-life\valve\map.wad>)")
    ;

    return options;
}
int Help_bsp2wad(int argc, const char** argv)
{
    std::cout << Options_bsp2wad(argc, argv).help({"Input", "Output"}) << std::endl;
    return 0;
}
int Exec_bsp2wad(int argc, const char** argv)
{
    auto options = Options_bsp2wad(argc, argv);
    auto result = options.parse(argc, argv);

#pragma region --file
    if(!result.count("file"))
    {
        const char* errorMsg = "You need to specify input file, use `--file path/to/map.bsp`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }
    using namespace Decay::Bsp::v30;
    std::shared_ptr<BspFile> bsp;
    {
        std::filesystem::path bspPath = result["file"].as<std::string>();
        if(!std::filesystem::exists(bspPath) || !std::filesystem::is_regular_file(bspPath))
        {
            const char* errorMsg = "`--file` must point to a valid file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(bspPath.extension() != ".bsp")
            std::cerr << "WARNING: BSP file path should have `.bsp` extension" << std::endl;
        try
        {
            bsp = std::make_shared<BspFile>(bspPath);
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "Failed to read/parse BSP file - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }
    }
#pragma endregion

#pragma region --wad + Add Textures
    if(result.count("wad"))
    {
        std::filesystem::path wadPath = result["wad"].as<std::string>();
        if(std::filesystem::exists(wadPath) && !std::filesystem::is_regular_file(wadPath))
        {
            const char* errorMsg = "`--wad` must point to a valid existing WAD file (or non-existing one to create a new one)";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }

        using namespace Decay::Wad::Wad3;
        auto textures = bsp->GetTextures();

        auto count = WadFile::AddToFile(wadPath, textures, {}, {});
        if(count == 0)
            std::cerr << "No textures to add" << std::endl;
        else
            std::cout << "Added " << count << " textures into " << wadPath << std::endl;
    }
#pragma endregion

#pragma region --newbsp
    // BSP without packed textures
    if(result.count("newbsp"))
    {
        std::filesystem::path outBspPath = result["newbsp"].as<std::string>();
        {
            if(outBspPath.extension() != ".bsp")
                std::cerr << "WARNING: Output BSP (`--newbsp`) file path does not have .bsp extension" << std::endl;
            if(std::filesystem::exists(outBspPath) && !std::filesystem::is_regular_file(outBspPath))
            {
                const char* errorMsg = "Output BSP file (`--newbsp`) exists but does not refer to valid file";
#ifdef DEBUG
                throw std::runtime_error(errorMsg);
#else
                std::cerr << errorMsg << std::endl;
#endif
                return 1;
            }
        }

#pragma region Textures
        using namespace Decay::Wad::Wad3;
        auto textures = bsp->GetTextures();

        for(std::size_t i = 0; i < textures.size(); i++)
            textures[i] = textures[i].CopyWithoutData();

        bsp->SetTextures(textures);
#pragma endregion

#pragma region --newbspwad
        if(result.count("--newbspwad"))
        {
            std::string bspWadPath = result["newbspwad"].as<std::string>();
            if(!bspWadPath.empty())
            {
                if(!bspWadPath.starts_with(R"(\half-life\)"))
                    std::cerr << "WARNING: Path for `--newbspwad` should start by \\half-life\\" << std::endl;

                //TODO add into "wad"
            }
        }
#pragma endregion

        bsp->Save(outBspPath);

        std::cout << "Saved BSP without packed textures to " << outBspPath << std::endl;
    }

    return 0;
}
#pragma endregion

#pragma region bsp_lightmap
cxxopts::Options Options_bsp_lightmap(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "bsp_lightmap" : argv[0], "Extracts per-face lightmap and packs them into a big lightmap");

    options.add_options("Input")
       ("f,file", "BSP file (the map)", cxxopts::value<std::string>(), "<map.bsp>")
    ;
    options.add_options("Output")
       ("lightmap", "WAD file (where to put textures)", cxxopts::value<std::string>(), "<lightmap.png>")
       //TODO "hole" color
    ;

    return options;
}
int Help_bsp_lightmap(int argc, const char** argv)
{
    std::cout << Options_bsp_lightmap(argc, argv).help({ "Input", "Output" }) << std::endl;
    std::cout << "Lightmap(s) have \"holes\" (unused pixels)." << std::endl;
    std::cout << "This is currently only useful to preview light on the map as there is no way to generate lightmap UV coordinates for OBJ file." << std::endl; //TODO move the lightmap into `bsp2obj` and add option to generate lightmap + lightmap UV
    return 0;
}
int Exec_bsp_lightmap(int argc, const char** argv)
{
    auto options = Options_bsp_lightmap(argc, argv);
    auto result = options.parse(argc, argv);

#pragma region --file
    if(!result.count("file"))
    {
        const char* errorMsg = "You need to specify input file, use `--file path/to/map.bsp`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }
    using namespace Decay::Bsp::v30;
    std::shared_ptr<BspFile> bsp;
    {
        std::filesystem::path bspPath = result["file"].as<std::string>();
        if(!std::filesystem::exists(bspPath) || !std::filesystem::is_regular_file(bspPath))
        {
            const char* errorMsg = "`--file` must point to a valid file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(bspPath.extension() != ".bsp")
            std::cerr << "WARNING: BSP file path should have `.bsp` extension" << std::endl;
        try
        {
            bsp = std::make_shared<BspFile>(bspPath);
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "Failed to read/parse BSP file - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }
    }
#pragma endregion


#pragma region --lightmap
    if(result.count("lightmap"))
    {
        std::filesystem::path exportLight = result["lightmap"].as<std::string>();

        if(std::filesystem::exists(exportLight))
        {
            if(!std::filesystem::is_regular_file(exportLight))
            {
                std::cerr << "Export lightmap path exists but is not a file" << std::endl;
                return 1;
            }
        }


#pragma region Export lightmap
        std::shared_ptr<BspTree> bspTree;
        try
        {
            bspTree = std::make_shared<BspTree>(bsp);
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "Failed to parse BSP file into high-level structure (BspTree) - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }

        std::string extension = exportLight.extension();
        R_ASSERT(extension.size() > 1);
        R_ASSERT(extension[0] == '.');

        std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec3* data)> writeFunc = Decay::ImageWriteFunction_RGB(extension);

        writeFunc(exportLight.c_str(), bspTree->Light.Width, bspTree->Light.Height, bspTree->Light.Data.data());
#pragma endregion
    }
#pragma endregion

    return 0;
}
#pragma endregion

#pragma region bsp_entity
cxxopts::Options Options_bsp_entity(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "bsp_entity" : argv[0], "Manipulate BSP Entities");

    options.add_options("Input")
       ("f,file", "BSP file (the map)", cxxopts::value<std::string>(), "<map.bsp>")
    ;
    options.add_options("Manipulate")
       ("add", "", cxxopts::value<std::string>(), "<entities.kv>")
       ("add_json", "", cxxopts::value<std::string>(), "<entities.json>")
       ("replace", "", cxxopts::value<std::string>(), "<entities.kv>")
       ("replace_json", "", cxxopts::value<std::string>(), "<entities.json>")
    ;
    options.add_options("Output")
       ("validate", "Use FGD file to validate entities", cxxopts::value<std::string>(), "<gamemode.fgd>")
       ("o,outbsp", "Output BSP file (after changes, requires `--file`)", cxxopts::value<std::string>(), "<map.bsp>")
       ("extract", "Extract entities as key-value file", cxxopts::value<std::string>(), "<map_entities.kv>")
       ("extract_json", "Extract entities as JSON file", cxxopts::value<std::string>(), "<map_entities.json>")
    ;

    return options;
}
int Help_bsp_entity(int argc, const char** argv)
{
    std::cout << Options_bsp_entity(argc, argv).help({"Input", "Manipulate", "Output"}) << std::endl;
    return 0;
}
int Exec_bsp_entity(int argc, const char** argv)
{
    auto options = Options_bsp_entity(argc, argv);
    auto result = options.parse(argc, argv);

    using namespace Decay::Bsp::v30;
    BspEntities entities{};

#pragma region --file
    if(!result.count("file"))
    {
        const char* errorMsg = "You need to specify input file, use `--file path/to/map.bsp`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }
    if(result.count("file"))
    {
        std::filesystem::path bspPath = result["file"].as<std::string>();
        if(!std::filesystem::exists(bspPath) || !std::filesystem::is_regular_file(bspPath))
        {
            const char* errorMsg = "`--file` must point to a valid file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(bspPath.extension() != ".bsp")
            std::cerr << "WARNING: BSP file path should have `.bsp` extension" << std::endl;
        try
        {
            std::shared_ptr<BspFile> bsp = std::make_shared<BspFile>(bspPath);
            R_ASSERT(bsp != nullptr);
            entities = BspEntities(*bsp);
        }
        catch(std::runtime_error& ex)
        {
            const char* errorMsg = "Failed to read/parse BSP file - ";
#ifdef DEBUG
            throw std::runtime_error(errorMsg + std::string(ex.what()));
#else
            std::cerr << errorMsg << ex.what() << std::endl;
#endif
            return 1;
        }
    }
#pragma endregion

#pragma region --replace (+json)
    if(result.count("replace"))
    {
        //TODO
    }
    if(result.count("replace_json"))
    {
        //TODO
    }
#pragma endregion

#pragma region --add (+json)
    if(result.count("add"))
    {
        //TODO
    }
    if(result.count("add_json"))
    {
        //TODO
    }
#pragma endregion

#pragma region --validate (FGD)
    if(result.count("validate"))
    {
        std::filesystem::path validatePath = result["validate"].as<std::string>();
        if(!std::filesystem::exists(validatePath) || !std::filesystem::is_regular_file(validatePath))
        {
            const char* errorMsg = "`--validate` must point to a valid file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(validatePath.extension() != ".fgd")
            std::cerr << "WARNING: FGD file to validate entities should have `.fgd` extension" << std::endl;

        using namespace Decay::Fgd;
        std::fstream in(validatePath, std::ios_base::in);
        FgdFile fgd(in);
        fgd.ProcessClassDependency();

        for(int i = 0; i < entities.size(); i++)
        {
            const auto& entity = entities[i];

            auto it_classname = entity.find("classname");
            if(it_classname == entity.end())
                throw std::runtime_error("Entity at index " + std::to_string(i) + " does not have a classname");
            const std::string& classname = it_classname->second;

            auto it_fgdClass = fgd.Classes.find(classname);
            if(it_fgdClass == fgd.Classes.end())
                throw std::runtime_error("Entity class " + classname + " was not found in FGD");
            const auto& fgdClass = it_fgdClass->second;

            //TODO For Source variant, don't forget to add IO to those checks as well

            // Does the entity have additional properties?
            // Does the entity have correct types on properties?
            for(const auto& property : entity)
            {
                const auto it_fgdProperty = fgdClass.Properties.find(property.first);
                if(it_fgdProperty == fgdClass.Properties.end())
                    throw std::runtime_error("Property " + property.first + " of entity " + classname + " was not found in provided FGD");
                const auto& fgdProperty = it_fgdProperty->second;

                if(Decay::StringCaseInsensitiveEqual(fgdProperty.Type, "choices"))
                    continue; // We don't care about type inside choices
                const auto valueType = FgdFile::GuessTypeFromString(property.second);
                if(Decay::StringCaseInsensitiveEqual(fgdProperty.Type, "flags") && valueType != FgdFile::ValueType::Integer)
                    throw std::runtime_error("Property of type `flags` can only have integer value");
                if(Decay::StringCaseInsensitiveEqual(fgdProperty.Type, "integer") && valueType != FgdFile::ValueType::Integer)
                    throw std::runtime_error("Property of type `integer` can only have integer value");
                if(Decay::StringCaseInsensitiveEqual(fgdProperty.Type, "float") && valueType != FgdFile::ValueType::Float)
                    throw std::runtime_error("Property of type `float` can only have integer or floating-point value");
            }

            // Does the entity miss some properties?
            for(const auto& kv_fgdProperty : fgdClass.Properties)
            {
                const auto it_property = entity.find(kv_fgdProperty.first);
                if(it_property == entity.end())
                    throw std::runtime_error("Entity " + classname + " (at index " + std::to_string(i) + ") is missing " + kv_fgdProperty.first + " property");
            }
        }
    }
#pragma endregion

#pragma region --extract (+json)
#   pragma region --extract
    if(result.count("extract"))
    {
        std::filesystem::path extractPath = result["extract"].as<std::string>();
        if(std::filesystem::exists(extractPath) && !std::filesystem::is_regular_file(extractPath))
        {
            const char* errorMsg = "`--extract` must not exist or be a valid file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(extractPath.extension() != ".kv")
            std::cerr << "WARNING: Entity KV file should have `.kv` extension" << std::endl;

        std::fstream out(extractPath, std::ios_base::out | std::ios_base::binary);
        out << entities; //THINK check empty
    }
#   pragma endregion
#   pragma region --extract_json
    if(result.count("extract_json"))
    {
        std::filesystem::path extractJsonPath = result["extract_json"].as<std::string>();
        if(std::filesystem::exists(extractJsonPath) && !std::filesystem::is_regular_file(extractJsonPath))
        {
            const char* errorMsg = "`--extract_json` must not exist or be a valid file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(extractJsonPath.extension() != ".json")
            std::cerr << "WARNING: Entity JSON (--extract_json) file should have `.json` extension" << std::endl;

        std::fstream out(extractJsonPath, std::ios_base::out);
        out << entities.AsJson().dump(4); //THINK check empty
    }
#   pragma endregion
#pragma endregion

#pragma region --outbsp
    if(result.count("outbsp"))
    {
        //TODO
    }
#pragma endregion

    return 0;
}
#pragma endregion
