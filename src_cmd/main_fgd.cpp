#include "main.hpp"

#include "Decay/Common.hpp"
#include "cxxopts.hpp"

#include "Decay/Fgd/FgdFile.hpp"

#include "util.hpp"

#pragma region wad_add
cxxopts::Options Options_fgd(int argc, const char** argv)
{
    cxxopts::Options options(argc == 0 ? "fgd" : argv[0], "Manipulate entity definitions");

    options.add_options("Input")
       ("f,file", "FGD file", cxxopts::value<std::string>(), "<file.fgd>")
       ("add", "FGD file to add", cxxopts::value<std::vector<std::string>>(), "<file.fgd>")
       ("include", "FGD file to include", cxxopts::value<std::vector<std::string>>(), "<file.fgd>")
       ("subtract", "FGD file to subtract", cxxopts::value<std::vector<std::string>>(), "<file.fgd>")
    ;
    options.add_options("Manipulate")
       ("process_includes", "Process @Include as if they were used in --include")
       ("include_dir", "Directory @Include files (--process_includes)", cxxopts::value<std::string>(), "<directory>")
       ("process_base", "Process (and discard) base classes")
    ;
    options.add_options("Output")
       ("output", "Save as FGD", cxxopts::value<std::string>(), "<file.fgd>")
#ifdef DECAY_JSON_LIB
       ("output_json", "Save as JSON", cxxopts::value<std::string>(), "<file.json>")
#endif
    ;

    options.positional_help("-f <file.fgd> ...");

    options.set_width(200);
    return options;
}
int Help_fgd(int argc, const char** argv)
{
    std::cout << Options_fgd(argc, argv).help({ "Input", "Manipulate", "Output" }) << std::endl;
    return 0;
}
int Exec_fgd(int argc, const char** argv)
{
    auto options = Options_fgd(argc, argv);
    auto result = options.parse(argc, argv);

    using namespace Decay::Fgd;

    FgdFile fgd{};

#pragma region --file
    std::filesystem::path fgdPath{};
    if(result.count("file"))
    {
        if(GetFilePath_Existing(result, "file", fgdPath, ".fgd"))
        {
            std::fstream in(fgdPath, std::ios_base::in);
            fgd = FgdFile(in);
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --add
    std::vector<std::filesystem::path> addPaths{};
    if(result.count("add"))
    {
        if(GetFilePath_Existing(result, "add", addPaths, ".fgd"))
        {
            for(const auto& addPath : addPaths)
            {
                std::fstream in(addPath, std::ios_base::in);
                FgdFile addFgd(in);
                fgd.Add(addFgd);
            }
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --include
    std::vector<std::filesystem::path> includePaths{};
    if(result.count("include"))
    {
        if(GetFilePath_Existing(result, "include", includePaths, ".fgd"))
        {
            for(const auto& includePath : includePaths)
            {
                std::fstream in(includePath, std::ios_base::in);
                FgdFile includeFgd(in);
                fgd.Include(includeFgd);
            }
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --subtract
    std::vector<std::filesystem::path> subtractPaths{};
    if(result.count("subtract"))
    {
        if(GetFilePath_Existing(result, "subtract", subtractPaths, ".fgd"))
        {
            for(const auto& subtractPath : subtractPaths)
            {
                std::fstream in(subtractPath, std::ios_base::in);
                FgdFile subtractFgd(in);
                fgd.Subtract(subtractFgd, true, true);
            }
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --process_includes
    if(result.count("process_includes"))
    {
        std::vector<std::filesystem::path> filesToIgnore{};
        {
            filesToIgnore.reserve(1 + addPaths.size() + includePaths.size() + subtractPaths.size());

            if(!fgdPath.empty())
                filesToIgnore.emplace_back(std::filesystem::canonical(fgdPath));

            for(const auto& addPath: addPaths)
                if(!addPath.empty())
                    filesToIgnore.emplace_back(std::filesystem::canonical(addPath));

            for(const auto& includePath: includePaths)
                if(!includePath.empty())
                    filesToIgnore.emplace_back(std::filesystem::canonical(includePath));

            for(const auto& subtractPath: subtractPaths)
                if(!subtractPath.empty())
                    filesToIgnore.emplace_back(std::filesystem::canonical(subtractPath));
        }

#   pragma region --process_includes
        std::filesystem::path relativeTo = ".";
        if(result.count("include_dir"))
        {
            relativeTo = result["include_dir"].as<std::string>();
#ifdef DEBUG
            std::cout << "--include_dir = " << relativeTo.string() << std::endl;
#endif
            if(!std::filesystem::exists(relativeTo) || !std::filesystem::is_directory(relativeTo))
            {
                std::cerr << "Directory from --include_dir does not exist" << std::endl;
                return false;
            }
        }
#   pragma endregion

        fgd.ProcessIncludes(relativeTo, filesToIgnore);
        fgd.IncludeFiles.clear();
    }
#pragma endregion

#pragma region --process_base
    if(result.count("process_base"))
    {
        fgd.Classes = fgd.ProcessClassDependency();
    }
#pragma endregion

#pragma region --output
    if(result.count("output"))
    {
        std::filesystem::path outputPath{};
        if(GetFilePath_NewOrOverride(result, "output", outputPath, ".fgd"))
        {
            std::fstream out(outputPath, std::ios_base::out);
            out << fgd;
        }
        else
            return 1;
    }
#pragma endregion

#pragma region --output_json
#   ifdef DECAY_JSON_LIB
    if(result.count("output_json"))
    {
        std::filesystem::path outputPath{};
        if(GetFilePath_NewOrOverride(result, "output_json", outputPath, ".json"))
        {
            std::fstream out(outputPath, std::ios_base::out);
            out << fgd.ExportAsJson().dump(4);
        }
        else
            return 1;
    }
#   endif
#pragma endregion

    return 0;
}
#pragma endregion
