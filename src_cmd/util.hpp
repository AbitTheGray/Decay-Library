#pragma once

#include "Decay/Common.hpp"
#include "cxxopts.hpp"

inline bool GetFilePath_Existing(cxxopts::ParseResult& result, const std::string& optionName, std::filesystem::path& path, const std::string& extensionCheck = {})
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
        R_ASSERT(extensionCheck[0] == '.', "`extensionCheck` should start with '.' character");
        if(path.extension() != extensionCheck)
        {
            std::cerr << "WARNING: File from --" << optionName << " should have extension '" << extensionCheck << "'" << std::endl;
        }
    }

    return true;
}
inline bool GetFilePath_NewOrOverride(cxxopts::ParseResult& result, const std::string& optionName, std::filesystem::path& path, const std::string& extensionCheck = {})
{
    if(!result.count(optionName))
    {
        std::cerr << "Option --" << optionName << " was not used" << std::endl;
        return false;
    }

    path = result[optionName].as<std::string>();
    if(std::filesystem::exists(path) && !std::filesystem::is_regular_file(path))
    {
        std::cerr << "File from --" << optionName << " exist but is not a file" << std::endl;
        return false;
    }

    if(!extensionCheck.empty())
    {
        R_ASSERT(extensionCheck[0] == '.', "`extensionCheck` should start with '.' character");
        if(path.extension() != extensionCheck)
        {
            std::cerr << "WARNING: File from --" << optionName << " should have extension '" << extensionCheck << "'" << std::endl;
        }
    }

    return true;
}
