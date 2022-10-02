#include "main.hpp"

#include "Decay/Common.hpp"
#include "cxxopts.hpp"

#include "Decay/Map/MapFile.hpp"
#include "Decay/Rmf/RmfFile.hpp"

#include "util.hpp"

#pragma region map2rmf
cxxopts::Options Options_map2rmf(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "map2rmf" : argv[0], "Convert from MAP to RMF");

    options.add_options("Input")
       ("f,file", "MAP file", cxxopts::value<std::string>(), "<file.map>")
    ;
    options.add_options("Output")
       ("rmf", "RMF file", cxxopts::value<std::string>(), "<file.rmf>")
    ;

    options.positional_help("-f <file.map> --rmf <file.rmf>");
    return options;
}
int Help_map2rmf(int argc, const char** argv)
{
    std::cout << Options_map2rmf(argc, argv).help({ "Input", "Output" }) << std::endl;
    return 0;
}
int Exec_map2rmf(int argc, const char** argv)
{
    auto options = Options_map2rmf(argc, argv);
    auto result = options.parse(argc, argv);

#pragma region --file
    if(!result.count("file"))
    {
        const char* errorMsg = "You need to specify input file, use `--file path/to/file.map`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }

    using namespace Decay::Map;
    MapFile mapFile;
    {
        std::filesystem::path mapPath = result["file"].as<std::string>();
        if(!std::filesystem::exists(mapPath) || !std::filesystem::is_regular_file(mapPath))
        {
            const char* errorMsg = "`--file` must point to a valid existing file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(mapPath.extension() != ".map")
            std::cerr << "WARNING: MAP file path should have `.map` extension" << std::endl;

        {
            std::fstream in(mapPath, std::ios_base::in);
            mapFile = MapFile(in);
        }
    }
#pragma endregion

#pragma region --rmf
    if(!result.count("rmf"))
    {
        const char* errorMsg = "You need to specify input file, use `--rmf path/to/file.rmf`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }
    std::filesystem::path rmfPath;
    {
        rmfPath = result["rmf"].as<std::string>();
        if(std::filesystem::exists(rmfPath) && !std::filesystem::is_regular_file(rmfPath))
        {
            const char* errorMsg = "`--rmf` must point to a valid existing file or non-existing file path";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(rmfPath.extension() != ".rmf")
            std::cerr << "WARNING: RMF file path should have `.rmf` extension" << std::endl;

    }
#pragma endregion

    using namespace Decay::Rmf;
    RmfFile rmfFile(mapFile);

    {
        std::fstream out(rmfPath, std::ios_base::out);
        out << rmfFile;
    }

    return 0;
}
#pragma endregion

#pragma region rmf2map
cxxopts::Options Options_rmf2map(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "rmf2map" : argv[0], "Convert from RMF to MAP");

    options.add_options("Input")
       ("f,file", "RMF file", cxxopts::value<std::string>(), "<file.rmf>")
    ;
    options.add_options("Output")
       ("map", "MAP file", cxxopts::value<std::string>(), "<file.map>")
    ;
    options.add_options("Engine Variant")
       ("goldsrc", "GoldSrc")
       ("idtech2", "IdTech 2")
    ;

    options.positional_help("-f <file.rmf> --map <file.map>");
    return options;
}
int Help_rmf2map(int argc, const char** argv)
{
    std::cout << Options_rmf2map(argc, argv).help({ "Input", "Output", "Engine Variant" }) << std::endl;
    return 0;
}
int Exec_rmf2map(int argc, const char** argv)
{
    auto options = Options_rmf2map(argc, argv);
    auto result = options.parse(argc, argv);

#pragma region --file
    if(!result.count("file"))
    {
        const char* errorMsg = "You need to specify input file, use `--file path/to/file.rmf`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }

    using namespace Decay::Rmf;
    RmfFile rmfFile;
    {
        std::filesystem::path rmfPath = result["file"].as<std::string>();
        if(!std::filesystem::exists(rmfPath) || !std::filesystem::is_regular_file(rmfPath))
        {
            const char* errorMsg = "`--file` must point to a valid existing file";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(rmfPath.extension() != ".rmf")
            std::cerr << "WARNING: RMF file path should have `.rmf` extension" << std::endl;

        {
            std::fstream in(rmfPath, std::ios_base::in);
            rmfFile = RmfFile(in);
        }
    }
#pragma endregion

#pragma region --map
    if(!result.count("map"))
    {
        const char* errorMsg = "You need to specify input file, use `--rmf path/to/file.map`";
#ifdef DEBUG
        throw std::runtime_error(errorMsg);
#else
        std::cerr << errorMsg << std::endl;
#endif
        return 1;
    }
    std::filesystem::path mapPath;
    {
        mapPath = result["map"].as<std::string>();
        if(std::filesystem::exists(mapPath) && !std::filesystem::is_regular_file(mapPath))
        {
            const char* errorMsg = "`--map` must point to a valid existing file or non-existing file path";
#ifdef DEBUG
            throw std::runtime_error(errorMsg);
#else
            std::cerr << errorMsg << std::endl;
#endif
            return 1;
        }
        if(mapPath.extension() != ".map")
            std::cerr << "WARNING: MAP file path should have `.map` extension" << std::endl;

    }
#pragma endregion

    using namespace Decay::Map;

#pragma region Engine Variant
    std::optional<MapFile::EngineVariant> variant = {};
    if(result.count("goldsrc"))
    {
        R_ASSERT(!variant.has_value(), "Multiple variants selected");
        variant = MapFile::EngineVariant::GoldSrc;
    }
    if(result.count("idtech2"))
    {
        R_ASSERT(!variant.has_value(), "Multiple variants selected");
        variant = MapFile::EngineVariant::IdTech2;
    }
#pragma endregion

    MapFile mapFile = (MapFile)rmfFile;

    {
        std::fstream out(mapPath, std::ios_base::out);
        mapFile.Write(out, variant.has_value() ? variant.value() : MapFile::EngineVariant::GoldSrc);
    }

    return 0;
}
#pragma endregion
