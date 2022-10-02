#include "main.hpp"

#include "cxxopts.hpp"

#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspTree.hpp"

#include "Decay/Fgd/FgdFile.hpp"

#include "util.hpp"

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

    options.positional_help("-f <map.bsp> ...");

    options.set_width(200);
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

    options.positional_help("-f <map.bsp> ...");

    options.set_width(200);
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
    using namespace Decay::Bsp::v30;
    std::filesystem::path bspPath{};
    std::shared_ptr<BspFile> bsp;
    if(GetFilePath_Existing(result, "file", bspPath, ".bsp"))
    {
        try
        {
            bsp = std::make_shared<BspFile>(bspPath);
        }
        catch(std::runtime_error& ex)
        {
            std::cerr << "Failed to read/parse BSP file - " << ex.what() << std::endl;
            return 1;
        }
    }
    else
        return 1;
#pragma endregion

#pragma region --wad + Add Textures
    if(result.count("wad"))
    {
        std::filesystem::path wadPath{};
        if(GetFilePath_NewOrOverride(result, "wad", wadPath, ".wad"))
        {
            using namespace Decay::Wad::Wad3;
            auto textures = bsp->GetTextures();

            auto count = WadFile::AddToFile(wadPath, textures, {}, {});
            if(count == 0)
                std::cerr << "No textures to add" << std::endl;
            else
                std::cout << "Added " << count << " textures into " << wadPath << std::endl;
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --newbsp
    // BSP without packed textures
    if(result.count("newbsp"))
    {
        std::filesystem::path outBspPath = {};
        if(!GetFilePath_NewOrOverride(result, "newbsp", outBspPath, ".bsp"))
            return 1;

#pragma region Textures
        using namespace Decay::Wad::Wad3;
        auto textures = bsp->GetTextures();

        for(std::size_t i = 0; i < textures.size(); i++) // NOLINT(modernize-loop-convert)
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

                {
                    // add into "wad"
                    BspEntities entities(*bsp);
                    {
                        for(int i = 0; i < entities.size(); i++)
                        {
                            auto& ent = entities[i];
                            const auto classname = ent.find("classname");
                            if(classname == ent.end() || classname->second != "worldspawn")
                                continue;

                            auto& wad = ent["wad"];
                            if(wad.empty())
                                wad = bspWadPath;
                            else
                                wad += ';' + bspWadPath;
                            break;
                        }
                    }

                    // Convert to string
                    std::stringstream ss;
                    ss << entities;

                    // Save entities string into BSP
                    bsp->SetEntities(ss.str());
                }
#ifdef DEBUG
                {
                    BspEntities entities(*bsp);
                    {
                        for(int i = 0; i < entities.size(); i++)
                        {
                            auto& ent = entities[i];
                            const auto classname = ent.find("classname");
                            if(classname == ent.end() || classname->second != "worldspawn")
                                continue;

                            auto& wad = ent["wad"];
                            if(wad.find(bspWadPath) == std::string::npos)
                                throw std::runtime_error("Change in \"wad\" of \"worldspawn\" did not go through");
                            break;
                        }
                    }
                }
#endif
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

    options.positional_help("-f <map.bsp> ...");

    options.set_width(200);
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
    using namespace Decay::Bsp::v30;
    std::filesystem::path bspPath{};
    std::shared_ptr<BspFile> bsp;
    if(GetFilePath_Existing(result, "file", bspPath, ".bsp"))
    {
        try
        {
            bsp = std::make_shared<BspFile>(bspPath);
        }
        catch(std::runtime_error& ex)
        {
            std::cerr << "Failed to read/parse BSP file - " << ex.what() << std::endl;
            return 1;
        }
    }
    else
        return 1;
#pragma endregion

#pragma region --lightmap
    std::filesystem::path lightmapPath{};
    if(GetFilePath_Existing(result, "lightmap", lightmapPath, ".png"))
    {
        std::shared_ptr<BspTree> bspTree;
        try
        {
            bspTree = std::make_shared<BspTree>(bsp);
        }
        catch(std::runtime_error& ex)
        {
            std::cerr << "Failed to parse BSP file into high-level structure (BspTree) - " << ex.what() << std::endl;
            return 1;
        }

        std::string extension = lightmapPath.extension().string();
        R_ASSERT(extension.size() > 1, "Lightmap path must have a supported extension - extension is too short");
        R_ASSERT(extension[0] == '.', "Lightmap path must have a supported extension - no extension or no '.' character");

        std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec3* data)> writeFunc = Decay::ImageWriteFunction_RGB(extension);
        R_ASSERT(writeFunc != nullptr, "Lightmap path must have a supported extension - unsupported extension");

        writeFunc(lightmapPath.string().c_str(), bspTree->Light.Width, bspTree->Light.Height, bspTree->Light.Data.data());
    }
    else
        return 1;
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
       ("replace", "Replace cached (--file) entities by entities from KeyValue file", cxxopts::value<std::string>(), "<entities.kv>")
#ifdef DECAY_JSON_LIB
       ("replace_json", "Replace cached (--file) entities by entities from JSON file", cxxopts::value<std::string>(), "<entities.json>")
#endif
       ("add", "Add entities from KeyValue file to currently cached entities", cxxopts::value<std::vector<std::string>>(), "<entities.kv>")
#ifdef DECAY_JSON_LIB
       ("add_json", "Add entities from JSON file to currently cached entities", cxxopts::value<std::vector<std::string>>(), "<entities.json>")
#endif
    ;
    options.add_options("Output")
       ("validate", "Use FGD file to validate entities", cxxopts::value<std::string>(), "<gamemode.fgd>")
       ("o,outbsp", "Output BSP file (after changes, requires `--file`)", cxxopts::value<std::string>(), "<map.bsp>")
       ("extract", "Extract entities as key-value file", cxxopts::value<std::string>(), "<map_entities.kv>")
#ifdef DECAY_JSON_LIB
       ("extract_json", "Extract entities as JSON file", cxxopts::value<std::string>(), "<map_entities.json>")
#endif
    ;

    options.positional_help("-f <map.bsp> ...");

    options.set_width(200);
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
    std::shared_ptr<BspFile> bsp{}; ///< Can be NULL
    if(result.count("file"))
    {
        std::filesystem::path bspPath{};
        if(GetFilePath_Existing(result, "file", bspPath, ".bsp"))
        {
            try
            {
                bsp = std::make_shared<BspFile>(bspPath);
                R_ASSERT(bsp != nullptr, "Failed to load BSP");
                entities = BspEntities(*bsp);
            }
            catch(std::runtime_error& ex)
            {
                std::cerr << "Failed to read/parse BSP file - " << ex.what() << std::endl;
                return 1;
            }
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --replace (+json)
    if(result.count("replace"))
    {
        std::filesystem::path replacePath{};
        if(GetFilePath_Existing(result, "replace", replacePath, ".kv"))
        {
            std::fstream in(replacePath, std::ios_base::in);
            entities = BspEntities(in);
        }
        else
            return 1;
    }
#   ifdef DECAY_JSON_LIB
    if(result.count("replace_json"))
    {
        std::filesystem::path replacePath{};
        if(GetFilePath_Existing(result, "replace_json", replacePath, ".json"))
        {
            std::fstream in(replacePath, std::ios_base::in);
            nlohmann::json inJson = nlohmann::json::parse(in);
            entities = BspEntities(inJson);
        }
        else
            return 1;
    }
#   endif
#pragma endregion

#pragma region --add (+json)
    if(result.count("add"))
    {
        std::vector<std::filesystem::path> addPaths{};
        if(GetFilePath_Existing(result, "add", addPaths, ".kv"))
        {
            for(const auto& addPath : addPaths)
            {
                std::fstream in(addPath, std::ios_base::in);
                BspEntities newEntities(in);

                for(int i = 0; i < newEntities.size(); i++)
                    entities.emplace(newEntities[i]);
            }
        }
        else
            return 1;
    }
#   ifdef DECAY_JSON_LIB
    if(result.count("add_json"))
    {
        std::vector<std::filesystem::path> addPaths{};
        if(GetFilePath_Existing(result, "add_json", addPaths, ".json"))
        {
            for(const auto& addPath : addPaths)
            {
                std::fstream in(addPath, std::ios_base::in);
                nlohmann::json inJson = nlohmann::json::parse(in);
                BspEntities newEntities(inJson);

                for(int i = 0; i < newEntities.size(); i++)
                    entities.emplace(newEntities[i]);
            }
        }
        else
            return 1;
    }
#   endif
#pragma endregion

#pragma region --validate (FGD)
    if(result.count("validate"))
    {
        std::filesystem::path validatePath = result["validate"].as<std::string>();
        if(!std::filesystem::exists(validatePath) || !std::filesystem::is_regular_file(validatePath))
        {
            std::cerr << "`--validate` must point to a valid file" << std::endl;
            return 1;
        }
        if(validatePath.extension() != ".fgd")
            std::cerr << "WARNING: FGD file to validate entities should have `.fgd` extension" << std::endl;

        using namespace Decay::Fgd;
        std::fstream in(validatePath, std::ios_base::in);
        FgdFile fgd(in);
        {
            std::vector<std::filesystem::path> filesToIgnore{};
            filesToIgnore.emplace_back(std::filesystem::canonical(validatePath));
            fgd.ProcessIncludes(filesToIgnore[0].parent_path() /* Relative to the file */, filesToIgnore);
        }
        fgd.Classes = fgd.ProcessClassDependency();

        for(int i = 0; i < entities.size(); i++)
        {
            const auto& entity = entities[i];

            auto it_classname = entity.find("classname");
            if(it_classname == entity.end())
            {
                std::cerr << "Entity at index " << i << " does not have a classname" << std::endl;
                continue;
            }
            const std::string& classname = it_classname->second;

            auto it_fgdClass = fgd.Classes.find(classname);
            if(it_fgdClass == fgd.Classes.end())
            {
                std::cerr << "Entity class " << classname << " (at index " << i << ") was not found in FGD" << std::endl;
                continue;
            }
            const auto& fgdClass = it_fgdClass->second;

            //TODO For Source variant, don't forget to add IO to those checks as well

            // Does the entity have additional properties?
            // Does the entity have correct types on properties?
            for(const auto& property : entity)
            {
                if(property.first == "classname")
                    continue;

                const auto it_fgdProperty = fgdClass.Properties.find(property.first);
                if(it_fgdProperty == fgdClass.Properties.end())
                {
                    std::cerr << "Property " << property.first << " of entity " << classname << " (at index " << i << ") was not found in provided FGD" << std::endl;
                    continue;
                }
                const auto& fgdProperty = it_fgdProperty->second;

                if(Decay::StringCaseInsensitiveEqual(fgdProperty.Type, "choices"))
                    continue; // We don't care about type inside choices
                const auto valueType = FgdFile::GuessTypeFromString(property.second);
                if(Decay::StringCaseInsensitiveEqual(fgdProperty.Type, "flags") && valueType != FgdFile::ValueType::Integer)
                {
                    std::cerr << "Property " << property.first << " of entity " << classname << " (at index " << i << ") has type `flags` which can only have integer value" << std::endl;
                    continue;
                }
                if(Decay::StringCaseInsensitiveEqual(fgdProperty.Type, "integer") && valueType != FgdFile::ValueType::Integer)
                {
                    std::cerr << "Property " << property.first << " of entity " << classname << " (at index " << i << ") has type `integer` which can only have integer value" << std::endl;
                    continue;
                }
                if(Decay::StringCaseInsensitiveEqual(fgdProperty.Type, "float") && valueType != FgdFile::ValueType::Float && valueType != FgdFile::ValueType::Integer)
                {
                    std::cerr << "Property " << property.first << " of entity " << classname << " (at index " << i << ") has type `float` which can only have integer or floating-point value" << std::endl;
                    continue;
                }
            }

            // Does the entity miss some properties?
            for(const auto& kv_fgdProperty : fgdClass.Properties)
            {
                const auto it_property = entity.find(kv_fgdProperty.first);
                if(it_property == entity.end())
                {
                    std::cerr << "Entity " << classname << " (at index " << i << ") is missing " << kv_fgdProperty.first << " property" << std::endl;
                    continue;
                }
            }
        }
    }
#pragma endregion

#pragma region --extract (+json)
    if(result.count("extract"))
    {
        std::filesystem::path extractPath{};
        if(GetFilePath_NewOrOverride(result, "extract", extractPath, ".kv"))
        {
            std::fstream out(extractPath, std::ios_base::out);
            out << entities; //THINK check empty
        }
        else
            return 1;
    }
#   ifdef DECAY_JSON_LIB
    if(result.count("extract_json"))
    {
        std::filesystem::path extractPath{};
        if(GetFilePath_NewOrOverride(result, "extract_json", extractPath, ".json"))
        {
            std::fstream out(extractPath, std::ios_base::out);
            out << entities.AsJson().dump(4); //THINK check empty
        }
        else
            return 1;
    }
#   endif
#pragma endregion

#pragma region --outbsp
    if(result.count("outbsp"))
    {
        if(bsp == nullptr)
        {
            std::cerr << "--outbsp specified but valid BSP is not loaded - did you use --file ?" << std::endl;
            return 1;
        }

        std::filesystem::path outBspPath{};
        if(GetFilePath_NewOrOverride(result, "outbsp", outBspPath, ".bsp"))
        {
            std::fstream out(outBspPath, std::ios_base::out | std::ios_base::binary);
            {
                std::stringstream ss;
                ss << entities;
                bsp->SetEntities(ss.str());
            }
            out << bsp;
        }
        else
            return 1;
    }
#pragma endregion

    return 0;
}
#pragma endregion
