#pragma once

#include "Decay/Common.hpp"
#include "cxxopts.hpp"

#pragma region std::filesystem::path
inline bool GetFilePath_Existing(
    cxxopts::ParseResult&  result,
    const std::string&     optionName,
    std::filesystem::path& path,
    const std::string&     extensionCheck = {}
)
{
    if(!result.count(optionName))
    {
        std::cerr << "Option --" << optionName << " was not used" << std::endl;
        return false;
    }

    path = result[optionName].as<std::string>();
#ifdef DEBUG
    std::cout << "--" << optionName << " = " << path.string() << std::endl;
#endif
    if(!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
    {
        std::cerr << "File from --" << optionName << " does not exist" << std::endl;
        return false;
    }

    if(!extensionCheck.empty())
    {
        if(extensionCheck[0] != '.')
        {
            std::cerr << "`extensionCheck` should start with '.' character" << std::endl;
            return false;
        }
        if(path.extension() != extensionCheck)
        {
            std::cerr << "WARNING: File from --" << optionName << " should have extension '" << extensionCheck << "'" << std::endl;
        }
    }

    return true;
}
inline bool GetFilePath_NewOrOverride(
    cxxopts::ParseResult&  result,
    const std::string&     optionName,
    std::filesystem::path& path,
    const std::string&     extensionCheck = {}
)
{
    if(!result.count(optionName))
    {
        std::cerr << "Option --" << optionName << " was not used" << std::endl;
        return false;
    }

    path = result[optionName].as<std::string>();
#ifdef DEBUG
    std::cout << "--" << optionName << " = " << path.string() << std::endl;
#endif
    if(std::filesystem::exists(path) && !std::filesystem::is_regular_file(path))
    {
        std::cerr << "File from --" << optionName << " exist but is not a file" << std::endl;
        return false;
    }

    if(!extensionCheck.empty())
    {
        if(extensionCheck[0] != '.')
        {
            std::cerr << "`extensionCheck` should start with '.' character" << std::endl;
            return false;
        }
        if(path.extension() != extensionCheck)
        {
            std::cerr << "WARNING: File from --" << optionName << " should have extension '" << extensionCheck << "'" << std::endl;
        }
    }

    return true;
}
#pragma endregion

#pragma region std::vector<std::filesystem::path>
inline bool GetFilePath_Existing(
    cxxopts::ParseResult&               result,
    const std::string&                  optionName,
    std::vector<std::filesystem::path>& paths,
    const std::string&                  extensionCheck = {}
)
{
    if(!result.count(optionName))
    {
        std::cerr << "Option --" << optionName << " was not used" << std::endl;
        return false;
    }

    std::vector<std::string> stringPaths = result[optionName].as<std::vector<std::string>>();
    paths.reserve(stringPaths.size());
    for(std::filesystem::path path : stringPaths)
    {
#ifdef DEBUG
        std::cout << "--" << optionName << " = " << path.string() << std::endl;
#endif
        if(!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
        {
            std::cerr << "File from --" << optionName << " does not exist" << std::endl;
            continue;
        }

        if(!extensionCheck.empty())
        {
            if(extensionCheck[0] != '.')
            {
                std::cerr << "`extensionCheck` should start with '.' character" << std::endl;
                continue;
            }
            if(path.extension() != extensionCheck)
            {
                std::cerr << "WARNING: File from --" << optionName << " should have extension '" << extensionCheck << "'" << std::endl;
                continue;
            }
        }

        paths.emplace_back(path);
    }

    return true;
}
inline bool GetFilePath_NewOrOverride(
    cxxopts::ParseResult&               result,
    const std::string&                  optionName,
    std::vector<std::filesystem::path>& paths,
    const std::string&                  extensionCheck = {}
)
{
    if(!result.count(optionName))
    {
        std::cerr << "Option --" << optionName << " was not used" << std::endl;
        return false;
    }

    std::vector<std::string> stringPaths = result[optionName].as<std::vector<std::string>>();
    paths.reserve(stringPaths.size());
    for(std::filesystem::path path : stringPaths)
    {
#ifdef DEBUG
        std::cout << "--" << optionName << " = " << path.string() << std::endl;
#endif
        if(std::filesystem::exists(path) && !std::filesystem::is_regular_file(path))
        {
            std::cerr << "File from --" << optionName << " exist but is not a file" << std::endl;
            continue;
        }

        if(!extensionCheck.empty())
        {
            if(extensionCheck[0] != '.')
            {
                std::cerr << "`extensionCheck` should start with '.' character" << std::endl;
                continue;
            }
            if(path.extension() != extensionCheck)
            {
                std::cerr << "WARNING: File from --" << optionName << " should have extension '" << extensionCheck << "'" << std::endl;
                continue;
            }
        }

        paths.emplace_back(path);
    }

    return true;
}
#pragma endregion
