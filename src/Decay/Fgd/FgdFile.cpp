#include "FgdFile.hpp"

#include <unordered_map>
#include "Decay/CommonReadUtils.hpp"

// Utility functions
namespace Decay::Fgd
{
    template<typename TIn, typename TVal>
    [[nodiscard]] inline bool ContainsAll(const TIn& inside, const TVal& values)
    {
        for(const auto& val : values)
        {
            bool found = false;
            for(const auto& in : inside)
            {
                if(in == val)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                return false;
        }
        return true;
    }
    void AddClassAndDependencies(
        std::vector<std::string>& orderedClasses, ///< Output
        const std::string& className, ///< Name of the class currently being processed
        const std::set<std::string>& classDependencies, ///< Direct base-classes of current class
        std::unordered_map<std::string, std::set<std::string>>& dependencyGraph, ///< Dependency for the class (pre-processed "base" option)
        std::vector<std::string>& recursionPrevention ///< Classes which were already processed
    )
    {
        recursionPrevention.emplace_back(className);

        if(classDependencies.empty()) // No base classes
        {
            orderedClasses.emplace_back(className);
        }
        else if(ContainsAll(orderedClasses, classDependencies)) // All base classes already mentioned
        {
            orderedClasses.emplace_back(className);
        }
        else // Some baseclass is missing
        {
            for(const std::string& dep : classDependencies)
            {
                auto it = dependencyGraph.find(dep);
                if(it == dependencyGraph.end())
                    throw std::runtime_error("Failed to order classes - class '" + className + "' depends on '" + dep + "' which does not exist"); //OPTIMIZE better string concat

                // Not references to keep a copy
                auto itName = it->first;
                auto itDepend = it->second;

                // Check for recursion
                for(const auto& processedClass : recursionPrevention)
                    if(processedClass == itName)
                        throw std::runtime_error("Failed to process dependency for '" + itName + "' - recursion of base classes");

                // Remove the class from list
                dependencyGraph.erase(it);

                // Process the `it` class
                AddClassAndDependencies(orderedClasses, itName, itDepend, dependencyGraph, recursionPrevention);
            }

            // All base classes have been processed
            orderedClasses.emplace_back(className);
        }

        recursionPrevention.resize(recursionPrevention.size() - 1); //OPTIMIZE Remove last but keep original capacity
    }
    void ProcessDependency(
        const decltype(FgdFile::Classes)& classes,
        FgdFile::Class& targetClass,
        const FgdFile::Class& baseClass
    )
    {
        // Copy properties
        for(const auto& baseProperty : baseClass.Properties)
        {
            if(targetClass.Properties.contains(baseProperty.first))
            {
                std::cerr << "Class '" << baseClass.Codename << "' already contains property '" << baseProperty.first << '\'' << std::endl;
                continue;
            }

            targetClass.Properties[baseProperty.first] = baseProperty.second;
        }

        // Copy IO
        for(const auto& baseIO : baseClass.IO)
        {
            if(targetClass.IO.contains(baseIO.first))
            {
                std::cerr << "Class '" << baseClass.Codename << "' already contains IO '" << baseIO.first << '\'' << std::endl;
                continue;
            }

            targetClass.IO[baseIO.first] = baseIO.second;
        }

        // Copy options
        for(const FgdFile::Option& baseOption : baseClass.Options)
        {
            if(StringCaseInsensitiveEqual(baseOption.Name, "base"))
                continue; // Skipped for now

            targetClass.Options.emplace_back(baseOption);
        }

        // Process base classes of `baseClass`
        for(const FgdFile::Option& baseOption : baseClass.Options)
        {
            if(StringCaseInsensitiveEqual(baseOption.Name, "base"))
            {
                for(const FgdFile::OptionParam& baseOptionParam : baseOption.Params)
                {
                    R_ASSERT(!baseOptionParam.Quoted, "Base class name cannot be inside quoted string");

                    const auto it_baseBaseClass = classes.find(baseOptionParam.Name);//TODO case-insensitive
                    if(it_baseBaseClass == classes.end())
                    {
                        std::cerr << "Not found base class '" << baseOptionParam.Name << "', ignoring for now" << std::endl;
                        continue;
                    }

                    ProcessDependency(classes, targetClass, it_baseBaseClass->second);
                }
            }
        }
    }
}

namespace Decay::Fgd
{
    FgdFile::FgdFile(std::istream& in)
    {
        in >> *this;
        if(in.fail())
            throw std::runtime_error("Failed to read FGD file");
    }
    std::vector<std::string> FgdFile::Class::ValidTypes = {
        // GoldSrc + Source
        "BaseClass",
        "PointClass",
        "SolidClass",

        // Source
        "NPCClass",
        "KeyFrameClass",
        "MoveClass",
        "FilterClass"
    };
    int FgdFile::OptionParam::GetVectorSize() const
    {
        if(Quoted)
            return false;

        int dimension = 0;
        bool isPreviousChar_Number = false;
        bool isPreviousChar_Whitespace = false;
        for(char c : Name)
        {
            if(c == '-' || (c >= '0' && c <= '9'))
            {
                isPreviousChar_Number = true;
                isPreviousChar_Whitespace = false;

                continue;
            }
            else if(IsWhitespace(c))
            {
                if(isPreviousChar_Number)
                    dimension++;

                isPreviousChar_Number = false;
                isPreviousChar_Whitespace = true;

                continue;
            }
            else
                return 0; // Only whitespace and numeric characters (including '-') are valid for vectors
        }
        if(isPreviousChar_Number)
            dimension++;
        R_ASSERT(dimension <= 4);
        return dimension;
    }
    void FgdFile::Add(const FgdFile& toAdd)
    {
        // Map Size
        if(toAdd.MapSize.has_value())
        {
            if(MapSize.has_value())
            {
                int toAddSize = toAdd.MapSize.value().y - toAdd.MapSize.value().x;
                int thisSize  =       MapSize.value().y -       MapSize.value().x;
                if(toAddSize > thisSize)
                    MapSize = toAdd.MapSize;
            }
            else
                MapSize = toAdd.MapSize;
        }

        // Auto Vis Group
        if(!toAdd.AutoVisGroups.empty())
        {
            for(const auto& toAddAVG : toAdd.AutoVisGroups)
            {
                AutoVisGroup* existingAVG = nullptr;
                for(auto& avg : AutoVisGroups)
                {
                    if(avg.DisplayName == toAddAVG.DisplayName)
                    {
                        existingAVG = &avg;
                        break;
                    }
                }
                if(existingAVG == nullptr)
                {
                    AutoVisGroups.emplace_back(toAddAVG);
                    continue;
                }

                // Merge Auto-Vis-Group
                for(const auto& toAddChild : toAddAVG.Child)
                {
                    AutoVisGroup_Child* existingChild = nullptr;
                    for(auto& avgc : existingAVG->Child)
                    {
                        if(avgc.DisplayName == toAddChild.DisplayName)
                        {
                            existingChild = &avgc;
                            break;
                        }
                    }
                    if(existingChild == nullptr)
                    {
                        existingAVG->Child.emplace_back(toAddChild);
                        continue;
                    }

                    // Merge Auto-Vis-Group Child
                    for(const auto& toAddEntity : toAddChild.EntityClasses)
                        existingChild->EntityClasses.emplace(toAddEntity);
                }
            }
        }

        // Includes
        if(!toAdd.IncludeFiles.empty()) //FIXME
        {
            for(const auto& toAddInclude : toAdd.IncludeFiles)
            {
                bool found = false;
                for(const auto& existingInclude : IncludeFiles)
                {
                    if(StringCaseInsensitiveEqual(toAddInclude, existingInclude))
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                    IncludeFiles.emplace_back(toAddInclude);
            }
        }

        // Material Exclusions
        if(!toAdd.MaterialExclusion.empty())
        {
            for(const auto& toAddDir : toAdd.MaterialExclusion)
                MaterialExclusion.emplace(toAddDir);
        }

        // Classes
        if(!toAdd.Classes.empty())
        {
            for(const auto& toAddClass : toAdd.Classes)
            {
                if(!Classes.contains(toAddClass.first))
                {
                    Classes.emplace(toAddClass);
                    continue;
                }
                auto& existingClass = Classes[toAddClass.first];

                // Properties
                for(const auto& kv_toAddProperty : toAddClass.second.Properties)
                {
                    Property* existingProperty = nullptr;
                    for(auto& prop : existingClass.Properties)
                    {
                        if(prop.second.Codename == kv_toAddProperty.second.Codename)
                        {
                            existingProperty = &prop.second;
                            break;
                        }
                    }
                    if(existingProperty == nullptr)
                    {
                        existingClass.Properties[kv_toAddProperty.first] = kv_toAddProperty.second;
                        continue;
                    }

                    const auto& toAddProperty = kv_toAddProperty.second;

                    // Combine property
                    existingProperty->Type = toAddProperty.Type;
                    existingProperty->ReadOnly = toAddProperty.ReadOnly;
                    if(!toAddProperty.DisplayName.empty())
                        existingProperty->DisplayName = toAddProperty.DisplayName;
                    if(!toAddProperty.DefaultValue.empty())
                        existingProperty->DefaultValue = toAddProperty.DefaultValue;
                    if(!toAddProperty.Description.empty())
                        existingProperty->Description = toAddProperty.Description;
                    if(StringCaseInsensitiveEqual(existingProperty->Type, "flags") || StringCaseInsensitiveEqual(existingProperty->Type, "flags"))
                    {
                        for(const auto& toAddFC : toAddProperty.FlagsOrChoices)
                        {
                            PropertyFlagOrChoice* existingFlagChoice = nullptr;
                            for(auto& fc : existingProperty->FlagsOrChoices)
                            {
                                if(fc.Index == toAddFC.Index)
                                {
                                    existingFlagChoice = &fc;
                                    break;
                                }
                            }
                            if(existingFlagChoice == nullptr)
                            {
                                existingProperty->FlagsOrChoices.emplace_back(toAddFC);
                                continue;
                            }

                            // Combine Flag/Choice
                            if(!toAddFC.DisplayName.empty())
                                existingFlagChoice->DisplayName = toAddFC.DisplayName;
                            existingFlagChoice->Default = toAddFC.Default;
                        }
                    }
                    else
                        existingProperty->FlagsOrChoices.clear();
                }

                // Input / Output
                if(!toAddClass.second.IO.empty())
                {
                    for(const auto& toAddIO : toAddClass.second.IO)
                    {
                        InputOutput* existingIO = nullptr;
                        for(auto& io : existingClass.IO)
                        {
                            if(io.second.Type == toAddIO.second.Type && io.second.Name == toAddIO.second.Name)
                            {
                                existingIO = &io.second;
                                break;
                            }
                        }
                        if(existingIO == nullptr)
                        {
                            existingClass.IO[toAddIO.first] = toAddIO.second;
                            continue;
                        }

                        // Combine IO
                        existingIO->ParamType = toAddIO.second.ParamType;
                        if(!toAddIO.second.Description.empty())
                            existingIO->Description = toAddIO.second.Description;
                    }
                }
            }
        }
    }
    void FgdFile::Subtract(const FgdFile& toSub, bool ignoreDescription, bool ignorePropertyDisplayName)
    {
        // Map Size
        if(toSub.MapSize.has_value() && MapSize.has_value() && toSub.MapSize.value() == MapSize.value())
            MapSize = {};

        // Auto Vis Group
        if(!toSub.AutoVisGroups.empty())
        {
            for(const auto& toSubAutoVisGroup : toSub.AutoVisGroups)
            {
                auto foundAVG = std::find_if(AutoVisGroups.begin(), AutoVisGroups.end(), [&toSubAutoVisGroup](const FgdFile::AutoVisGroup& existing) { return existing.DisplayName == toSubAutoVisGroup.DisplayName; });//THINK case-insensitive search?
                if(foundAVG == AutoVisGroups.end()) // Not Found
                    continue;

                for(const auto& toSubAutoVisGroupChild : toSubAutoVisGroup.Child)
                {
                    auto foundAVGC = std::find_if(foundAVG->Child.begin(), foundAVG->Child.end(), [&toSubAutoVisGroupChild](const FgdFile::AutoVisGroup_Child& existing) { return existing.DisplayName == toSubAutoVisGroupChild.DisplayName; });//THINK case-insensitive search?
                    if(foundAVGC == foundAVG->Child.end()) // Not Found
                        continue;

                    foundAVG->Child.erase(foundAVGC);
                }

                if(foundAVG->Child.empty())
                    AutoVisGroups.erase(foundAVG);
            }
        }

        // Includes
        if(!toSub.IncludeFiles.empty())
        {
            for(const auto& includeFile : toSub.IncludeFiles)
            {
                auto foundFile = std::find(IncludeFiles.begin(), IncludeFiles.end(), includeFile);//THINK case-insensitive search?
                if(foundFile != IncludeFiles.end()) // Found -> Remove
                    IncludeFiles.erase(foundFile);
            }
        }

        // Material Exclusions
        if(!toSub.MaterialExclusion.empty())
        {
            for(const auto& matExclusion : toSub.MaterialExclusion)
            {
                auto foundMaterial = std::find(MaterialExclusion.begin(), MaterialExclusion.end(), matExclusion);//THINK case-insensitive search?
                if(foundMaterial != MaterialExclusion.end()) // Found -> Remove
                    MaterialExclusion.erase(foundMaterial);
            }
        }

        // Classes
        if(!toSub.Classes.empty())
        {
            // Process classes
            for(const auto& clss : toSub.Classes)
            {
                auto foundClass = Classes.find(clss.first);
                if(foundClass == Classes.end()) // Not found
                    continue;

                // Process properties
                {
                    auto& foundClassProperties = foundClass->second.Properties;
                    for(const auto& it_prop : clss.second.Properties)
                    {
                        const auto& prop = it_prop.second;

                        auto it_foundProperty = foundClassProperties.find(it_prop.first);
                        if(it_foundProperty == foundClassProperties.end()) // Not Found
                            continue;
                        auto& foundProperty = it_foundProperty->second;

                        bool isSame = prop.Type == foundProperty.Type;
                        isSame &= prop.ReadOnly == foundProperty.ReadOnly;
                        isSame &= (ignoreDescription || prop.Description == foundProperty.Description);
                        isSame &= (ignorePropertyDisplayName || prop.DisplayName == foundProperty.DisplayName);
                        isSame &= prop.DefaultValue == foundProperty.DefaultValue;
                        isSame &= prop.FlagsOrChoices.size() == foundProperty.FlagsOrChoices.size();
                        if(!prop.FlagsOrChoices.empty())
                            for(int i = 0; i < prop.FlagsOrChoices.size(); i++)
                                isSame &= prop.FlagsOrChoices[i] == foundProperty.FlagsOrChoices[i];

                        if(isSame)
                            foundClassProperties.erase(it_foundProperty);
                    }
                }

                // Process IO
                {
                    auto& foundClassIO = foundClass->second.IO;
                    for(const auto& it_io : clss.second.IO)
                    {
                        const auto& io = it_io.second;

                        auto it_foundIO = foundClassIO.find(it_io.first);
                        if(it_foundIO == foundClassIO.end()) // Not Found
                            continue;
                        auto& foundIO = it_foundIO->second;

                        bool isSame = io.Type == foundIO.Type;
                        isSame &= io.ParamType == foundIO.ParamType;
                        isSame &= (ignoreDescription || io.Description == foundIO.Description);

                        if(isSame)
                            foundClassIO.erase(it_foundIO);
                    }
                }

                // Delete class?
                {
                    bool isSame = clss.second.Type == foundClass->second.Type;
                    isSame &= (ignoreDescription || clss.second.Description == foundClass->second.Description);

                    // Options
                    {
                        isSame &= clss.second.Options.size() == foundClass->second.Options.size();
                        for(int oi = 0; isSame && oi < clss.second.Options.size(); oi++)
                        {
                            const auto& clssOption = clss.second.Options[oi];
                            const auto& foundOption = foundClass->second.Options[oi];

                            isSame &= StringCaseInsensitiveEqual(clssOption.Name, foundOption.Name);
                            isSame &= clssOption.Params.size() == foundOption.Params.size();

                            // Option Parameters
                            {
                                auto foundOptionParams = foundOption.Params;
                                for(int opi = 0; isSame && opi < clssOption.Params.size(); opi++) // Order-independent comparison
                                {
                                    const auto& clssOptionParam = clssOption.Params[oi];
                                    auto foundOptionParam = std::find_if(foundOptionParams.begin(), foundOptionParams.end(), [&clssOptionParam](const FgdFile::OptionParam& existing) { return existing.Quoted == clssOptionParam.Quoted && StringCaseInsensitiveEqual(existing.Name, clssOptionParam.Name); });
                                    if(foundOptionParam == foundOptionParams.end()) // Not Found
                                    {
                                        isSame = false;
                                        break;
                                    }
                                    else // Found
                                    {
                                        foundOptionParams.erase(foundOptionParam);
                                    }
                                }
                                if(!foundOptionParams.empty())
                                    isSame = false;
                            }
                        }
                    }
                    if(isSame && foundClass->second.Properties.empty() && foundClass->second.IO.empty()) // No additional properties/io
                    {
                        Classes.erase(foundClass); // Delete the class, they are same (or similar enough)
                    }
                }
            }
        }
    }
    void FgdFile::Include(const FgdFile& toAdd)
    {
        // Map Size
        if(toAdd.MapSize.has_value())
        {
            if(MapSize.has_value())
            {
                int toAddSize = toAdd.MapSize.value().y - toAdd.MapSize.value().x;
                int thisSize  =       MapSize.value().y -       MapSize.value().x;
                if(toAddSize > thisSize)
                    MapSize = toAdd.MapSize;
            }
            else
                MapSize = toAdd.MapSize;
        }

        // Auto Vis Group
        if(!toAdd.AutoVisGroups.empty())
        {
            for(const auto& toAddAVG : toAdd.AutoVisGroups)
            {
                AutoVisGroup* existingAVG = nullptr;
                for(auto& avg : AutoVisGroups)
                {
                    if(avg.DisplayName == toAddAVG.DisplayName)
                    {
                        existingAVG = &avg;
                        break;
                    }
                }
                if(existingAVG == nullptr)
                {
                    AutoVisGroups.emplace_back(toAddAVG);
                    continue;
                }

                // Merge Auto-Vis-Group
                for(const auto& toAddChild : toAddAVG.Child)
                {
                    AutoVisGroup_Child* existingChild = nullptr;
                    for(auto& avgc : existingAVG->Child)
                    {
                        if(avgc.DisplayName == toAddChild.DisplayName)
                        {
                            existingChild = &avgc;
                            break;
                        }
                    }
                    if(existingChild == nullptr)
                    {
                        existingAVG->Child.emplace_back(toAddChild);
                        continue;
                    }

                    // Merge Auto-Vis-Group Child
                    for(const auto& toAddEntity : toAddChild.EntityClasses)
                        existingChild->EntityClasses.emplace(toAddEntity);
                }
            }
        }

        // Includes
        if(!toAdd.IncludeFiles.empty()) //FIXME
        {
            for(const auto& toAddInclude : toAdd.IncludeFiles)
            {
                bool found = false;
                for(const auto& existingInclude : IncludeFiles)
                {
                    if(StringCaseInsensitiveEqual(toAddInclude, existingInclude))
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                    IncludeFiles.emplace_back(toAddInclude);
            }
        }

        // Material Exclusions
        if(!toAdd.MaterialExclusion.empty())
        {
            for(const auto& toAddDir : toAdd.MaterialExclusion)
                MaterialExclusion.emplace(toAddDir);
        }

        // Classes
        if(!toAdd.Classes.empty())
        {
            for(const auto& toAddClass : toAdd.Classes)
            {
                if(!Classes.contains(toAddClass.first))
                {
                    Classes.emplace(toAddClass);
                    continue;
                }
                auto& existingClass = Classes[toAddClass.first];

                // Properties
                for(const auto& kv_toAddProperty : toAddClass.second.Properties)
                {
                    Property* existingProperty = nullptr;
                    for(auto& prop : existingClass.Properties)
                    {
                        if(prop.second.Codename == kv_toAddProperty.second.Codename)
                        {
                            existingProperty = &prop.second;
                            break;
                        }
                    }
                    if(existingProperty == nullptr)
                    {
                        existingClass.Properties[kv_toAddProperty.first] = kv_toAddProperty.second;
                        continue;
                    }

                    const auto& toAddProperty = kv_toAddProperty.second;

                    // Combine property
                    if(existingProperty->DisplayName.empty())
                        existingProperty->DisplayName = toAddProperty.DisplayName;
                    if(existingProperty->DefaultValue.empty())
                        existingProperty->DefaultValue = toAddProperty.DefaultValue;
                    if(existingProperty->Description.empty())
                        existingProperty->Description = toAddProperty.Description;
                    if(StringCaseInsensitiveEqual(existingProperty->Type, "flags") || StringCaseInsensitiveEqual(existingProperty->Type, "flags"))
                    {
                        for(const auto& toAddFC : toAddProperty.FlagsOrChoices)
                        {
                            PropertyFlagOrChoice* existingFlagChoice = nullptr;
                            for(auto& fc : existingProperty->FlagsOrChoices)
                            {
                                if(fc.Index == toAddFC.Index)
                                {
                                    existingFlagChoice = &fc;
                                    break;
                                }
                            }
                            if(existingFlagChoice == nullptr)
                            {
                                existingProperty->FlagsOrChoices.emplace_back(toAddFC);
                                continue;
                            }

                            // Combine Flag/Choice
                            if(existingFlagChoice->DisplayName.empty())
                                existingFlagChoice->DisplayName = toAddFC.DisplayName;
                        }
                    }
                    else
                        existingProperty->FlagsOrChoices.clear();
                }

                // Input / Output
                if(!toAddClass.second.IO.empty())
                {
                    for(const auto& kv_toAddIO : toAddClass.second.IO)
                    {
                        InputOutput* existingIO = nullptr;
                        for(auto& io : existingClass.IO)
                        {
                            if(io.second.Type == kv_toAddIO.second.Type && io.second.Name == kv_toAddIO.second.Name)
                            {
                                existingIO = &io.second;
                                break;
                            }
                        }
                        if(existingIO == nullptr)
                        {
                            existingClass.IO[kv_toAddIO.first] = kv_toAddIO.second;
                            continue;
                        }

                        // Combine IO
                        if(existingIO->Description.empty())
                            existingIO->Description = kv_toAddIO.second.Description;
                    }
                }
            }
        }
    }
    void FgdFile::ProcessIncludes(const std::filesystem::path& relativeToDirectory, std::vector<std::filesystem::path>& filesToIgnore)
    {
        for(const std::string& includeFile : IncludeFiles)
        {
            if(includeFile.empty())
                continue; // Empty file
            std::filesystem::path fullIncludeFile = relativeToDirectory / includeFile; //THINK Process for absolute path?

            if(std::find(filesToIgnore.begin(), filesToIgnore.end(), fullIncludeFile) != filesToIgnore.end())
                continue; // File should be ignored / already processed
            filesToIgnore.emplace_back(fullIncludeFile);

            std::fstream in(fullIncludeFile, std::ios_base::in | std::ios_base::binary);
            FgdFile includeFgd(in);
            Include(includeFgd);
        }
    }
    std::vector<std::string> FgdFile::OrderClassesByDependency() const
    {
        // key = class
        // value = all of its base-classes
        std::unordered_map<std::string, std::set<std::string>> dependencyGraph = {};
        for(const auto& clss : Classes)
        {
            std::set<std::string> baseClasses = {};
            for(const auto& option : clss.second.Options)
            {
                if(StringCaseInsensitiveEqual(option.Name, "base"))
                {
                    for(const auto& baseClass : option.Params)
                    {
                        R_ASSERT(!baseClass.Quoted);
                        baseClasses.emplace(ToLowerAscii(baseClass.Name));
                    }
                }
            }
            dependencyGraph[ToLowerAscii(clss.first)] = baseClasses;
        }

        std::vector<std::string> orderedClasses = {};
        {
            std::vector<std::string> recursionPrevention{};
            recursionPrevention.reserve(10);

            while(!dependencyGraph.empty())
            {
                auto it = dependencyGraph.begin();

                // Not references to keep a copy
                auto itName = it->first;
                auto itDepend = it->second;

                // Remove the class from list
                dependencyGraph.erase(it);

                // Process the `it` class
                AddClassAndDependencies(orderedClasses, itName, itDepend, dependencyGraph, recursionPrevention);
                R_ASSERT(recursionPrevention.empty());
            }
        }
        R_ASSERT(dependencyGraph.empty());
        R_ASSERT(orderedClasses.size() == Classes.size());
        return orderedClasses;
    }
    decltype(FgdFile::Classes) FgdFile::ProcessClassDependency() const
    {
        auto orderedClasses = OrderClassesByDependency();
        decltype(Classes) processedClasses{};

        // Process dependencies
        // Discard base classes
        for(int i = 0; i < orderedClasses.size(); i++)
        {
            const auto& className = orderedClasses[i];
            auto it_clss = Classes.find(className);
            R_ASSERT(it_clss != Classes.end());
            Class& clss = processedClasses[className];
            clss = it_clss->second; // Copy

            if(StringCaseInsensitiveEqual(clss.Type, "BaseClass"))
                continue; // Base classes are skipped, there is no need for them anymore

            for(const Option& option : clss.Options)
            {
                if(StringCaseInsensitiveEqual(option.Name, "base"))
                {
                    for(const OptionParam& optionParam : option.Params)
                    {
                        R_ASSERT(!optionParam.Quoted, "Base class name cannot be inside quoted string");

                        const auto it_baseClass = Classes.find(optionParam.Name);//TODO case-insensitive
                        if(it_baseClass == Classes.end())
                        {
                            std::cerr << "Not found base class '" << optionParam.Name << "', ignoring for now" << std::endl;
                            continue;
                        }

                        ProcessDependency(Classes, clss, it_baseClass->second);
                    }
                }
            }
        }

        return processedClasses;
    }
}

// Stream Operators
namespace Decay::Fgd
{
    // Class >> Option >> Param
    std::istream& operator>>(std::istream& in, FgdFile::OptionParam& optionParam)
    {
        // This script processed only single `param` from the preview below:
        // key()
        // key(param)
        // key(param, param)
        // key(param, param, param)

        if(!in.good())
            return in;
        IgnoreWhitespace(in);
        if(!in.good())
            return in;

        int c = in.peek();
        if(c == EOF || c == ')')
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }

        optionParam.Quoted = (c == '\"');
        if(optionParam.Quoted) // Quoted
        {
            optionParam.Name = str(ReadQuotedString(in));
            return in;
        }
        else // Not quoted
        {
            std::vector<char> name;
            while(true)
            {
                auto name2 = ReadUntilWhitespaceOrAny(
                    in,
                    std::array<char, 2>{
                        ',', // Start of next parameter
                        ')' // End of parameters
                    }
                );
                if(name2.empty())
                {
                    in.setstate(std::ios_base::failbit);
                    return in;
                }
                Combine(name, name2);

                // Check for number -> consider it a vector
                if(IsNumber(name, true, true))
                {
                    c = in.peek();
                    if(IsWhitespace(c))
                    {
                        in.ignore();
                        name.emplace_back(' ');
                        continue;
                    }
                }

                optionParam.Name = str(name);
                return in;
            }
        }
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::OptionParam& optionParam)
    {
        if(optionParam.Quoted)
        {
            out << '\"';
            for(char c : optionParam.Name) //TODO TO utility function
            {
                switch(c)
                {
                    default:
                        out << c;
                        break;
                    case '\"':
                        out << "\"";
                        break;
                    case '\\':
                        out << "\\\\";
                        break;
                    case '\n':
                        out << "\\n";
                        break;
                }
            }
            out << '\"';
        }
        else
            out << optionParam.Name;
        return out;
    }

    // Class >> Option
    std::istream& operator>>(std::istream& in, FgdFile::Option& option)
    {
        IgnoreWhitespace(in);
        auto name = ReadUntilWhitespaceOr(in, '(' /* Start of parameters */ );
        option.Name = str(name);

        option.Params.clear();
        int c = in.peek();
        if(c == '(')
        {
            in.ignore(); // Skip the '(' char

            FgdFile::OptionParam op = {};

GOTO_OPTION_PARAM:
            IgnoreWhitespace(in);

            in >> op;
            if(in.fail()) // Problem processing - could be fail or no valid text (end of option params)
            {
                in.clear(in.rdstate() & ~std::istream::failbit);
                c = in.peek();
                switch(c)
                {
                    case ')': // End of params
                        in.ignore(); // Skip the ')' character
                        break;
                    default:
                        throw std::runtime_error("Unexpected character in option parameter");
                }
            }
            else // not fail
            {
                option.Params.emplace_back(op);

                IgnoreWhitespace(in);

                c = in.peek();
                switch(c)
                {
                    case ')': // End of params
                        in.ignore(); // Skip the ')' character
                        break;
                    case ',': // Next param
                        in.ignore(); // Skip the ',' character
                        goto GOTO_OPTION_PARAM;
                    default:
                        throw std::runtime_error("Unexpected character after option parameter");
                }
            }
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::Option& option)
    {
        R_ASSERT(!option.Name.empty());
        R_ASSERT(option.Name.find(' ') == std::string::npos); //TODO Check for other than alphanumeric characters
        out << option.Name;
        if(StringCaseInsensitiveEqual(option.Name, "halfgridsnap")) // `halfgridsnap` is exception and does not have `()` after it.
        {
            if(!option.Params.empty())
                throw std::runtime_error("Entity class option `halfgridsnap` cannot have any parameters");
        }
        else
        {
            out << '(';

            for(int i = 0; i < option.Params.size(); i++)
            {
                if(i != 0)
                    out << ", ";

                out << option.Params[i];
            }

            out << ')';
        }
        return out;
    }

    // Class >> Property >> FlagOrChoice
    std::istream& operator>>(std::istream& in, FgdFile::PropertyFlagOrChoice& propertyFlag)
    {
        if(!in.good())
            return in;

        // "string" : "string"
        // "float" : "string"
        // <int> : "string"
        // <int> : "string" : <int>

        IgnoreWhitespace(in);

        int c = in.peek();
        switch(c)
        {
            case EOF:
                in.setstate(std::ios_base::failbit);
                return in;
            case '\"':
            {
                auto index = ReadQuotedString(in);
                propertyFlag.Index = str(index);
                break;
            }
            default:
            {
                int index;
                in >> index;
                if(!in.good())
                    return in;
                propertyFlag.Index = std::to_string(index);
                break;
            }
        }

        IgnoreWhitespace(in);

        c = in.peek();
        if(c != ':')
            throw std::runtime_error("Property Flag does not have a name");
        in.ignore();

        IgnoreWhitespace(in);

        c = in.peek();
        if(c != '\"')
            throw std::runtime_error("Property Flag does not contain valid name");
        auto name = ReadQuotedString(in);
        propertyFlag.DisplayName = str(name);

        IgnoreWhitespace(in);

        c = in.peek();
        if(c == ':') // Has default value
        {
            in.ignore(); // Skip ':'

            IgnoreWhitespace(in);

            auto str = ReadUntilWhitespace(in);
            propertyFlag.Default = (!str.empty() && str[0] != '0');
        }
        else
            propertyFlag.Default = false;

        return in;
    }
    std::ostream& FgdFile::PropertyFlagOrChoice::Write(std::ostream& out, bool includeDefault) const
    {
        if(GuessIndexType() == ValueType::Integer)
            out << Index;
        else
            out << '\"' << Index << '\"';

        out << " : \"" << DisplayName << "\"";

        if(includeDefault)
            out << " : " << Default;
        return out;
    }

    // Class >> Property
    std::istream& operator>>(std::istream& in, FgdFile::Property& property)
    {
        IgnoreWhitespace(in);
        if(in.eof())
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }

        int c;

        // Codename + Type
        {
            {
                auto codename = ReadUntilWhitespaceOr(in, '(' /* Start of parameter */);
                if(codename.empty())
                    throw std::runtime_error("Codename of property (inside class) cannot be empty - at least 1 character before '(' (property type)");
                property.Codename = str(codename);
            }

            c = in.peek();
            if(c != '(')
                throw std::runtime_error("Property must have its type inside round brackets");
            in.ignore();

            IgnoreWhitespace(in);

            {
                auto type = ReadUntilWhitespaceOr(in, ')' /* End of parameter */ );
                if(type.empty())
                    throw std::runtime_error("Type of property cannot be empty and `void` is not valid (cannot contain data), consider using `string` as it is the most versatile");
                property.Type = str(type);
            }

            IgnoreWhitespace(in);

            c = in.peek();
            if(c != ')')
                throw std::runtime_error("Property's type must end its round brackets");
            in.ignore();
        }

        IgnoreWhitespace(in);

        // Readonly
        property.ReadOnly = TryReadText(in, "readonly");
        if(in.eof())
            return in;

        IgnoreWhitespace(in);

        c = in.peek();
        if(c == EOF)
            return in;
        if(c == ':')
        {
            in.ignore(); // Skip ':'
            IgnoreWhitespace(in);

            if(in.peek() == '\"') // Has DisplayName
                property.DisplayName = str(ReadQuotedString(in));

            IgnoreWhitespace(in);

            c = in.peek();
            if(c == EOF)
                return in;
            if(c == ':')
            {
                in.ignore(); // Skip ':'
                IgnoreWhitespace(in);

                c = in.peek();
                if(c == '\"') // Has quoted default value
                    property.DefaultValue = str(ReadQuotedString(in));
                else if(c == '-' || (c >= '0' && c <= '9'))
                    property.DefaultValue = str(ReadOnlyNumber(in, true));

                IgnoreWhitespace(in);

                c = in.peek();
                if(c == EOF)
                    return in;
                if(c == ':')
                {
                    in.ignore(); // Skip ':'
                    IgnoreWhitespace(in);

                    property.Description = str(ReadQuotedString(in));
                }
            }
        }

        IgnoreWhitespace(in);

        c = in.peek();
        if(c == EOF)
            return in;
        if(c == '=') // Flag or Choices item
        {
#ifdef FGD_PROPERTY_ITEMS_LIMIT_FLAGS_CHOICES
            if(!StringCaseInsensitiveEqual(property.Type, "flags") && !StringCaseInsensitiveEqual(property.Type, "choices"))
                throw std::runtime_error("List of values (for a property) is only available for `flags` and `choices` types");
#endif

            in.ignore(); // Skip '='

            IgnoreWhitespace(in);

            c = in.peek();
            R_ASSERT(c == '[');
            in.ignore(); // Skip '['

            while(true)
            {
                IgnoreWhitespace(in);

                c = in.peek();
                if(c == ']') // End of items
                {
                    in.ignore(); // Skip ']'
                    break;
                }
                else if(c == EOF) [[unlikely]]
                    throw std::runtime_error("End-of-File reached inside `flags` or `choices` items");
                else
                {
                    FgdFile::PropertyFlagOrChoice item;
                    in >> item;
                    if(in.fail())
                        throw std::runtime_error("Failed to read `flags` or `choices` items");
                    property.FlagsOrChoices.emplace_back(item);
                }
            }
        }
        return in;
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::Property& property)
    {
        R_ASSERT(!property.Codename.empty());
        R_ASSERT(!property.Type.empty());
        out << property.Codename << '(' << property.Type << ')';
        if(property.ReadOnly)
            out << " readonly";

        if(!property.DisplayName.empty() || !property.DefaultValue.empty() || !property.Description.empty()) // A column separated by ':' exists
        {
            out << " :";

            if(!property.DisplayName.empty())
                out << " \"" << property.DisplayName << '\"';

            if(!property.DefaultValue.empty() || !property.Description.empty()) // A column separated by ':' exists
            {
                if(!property.DisplayName.empty()) // No space if Description is empty to create "::"
                    out << ' ';

                out << ':';

                if(!property.DefaultValue.empty())
                {
                    auto defaultValueType = FgdFile::GuessTypeFromString(property.DefaultValue);
                    if(defaultValueType == FgdFile::ValueType::Integer)
                        out << ' ' << property.DefaultValue;
                    else
                        out << " \"" << property.DefaultValue << '\"';
                }

                if(!property.Description.empty()) // A column separated by ':' exists
                {
                    if(!property.DefaultValue.empty()) // No space if Description is empty to create "::" or even ":::"
                        out << ' ';

                    out << ": \"" << property.Description << '\"';
                }
            }
        }

        if(!property.FlagsOrChoices.empty())
        {
            if(StringCaseInsensitiveEqual(property.Type, "choices"))
            {
                out << " = \n";
                out << "\t[ \n";

                for(const FgdFile::PropertyFlagOrChoice& pf : property.FlagsOrChoices)
                {
                    out << "\t\t";
                    pf.Write(out, false);
                    out << '\n';
                }

                out << "\t]";
            }
            else if(StringCaseInsensitiveEqual(property.Type, "flags"))
            {
                out << " = \n";
                out << "\t[\n";

                for(const FgdFile::PropertyFlagOrChoice& pf : property.FlagsOrChoices)
                {
                    out << "\t\t";
                    pf.Write(out, true);
                    out << '\n';
                }

                out << "\t]";
            }
            else
                throw std::runtime_error("List of values is only valid for `choices` and `flags` types");
        }
        return out;
    }

    // Class >> Input / Output >> Type
    std::istream& operator>>(std::istream& in, FgdFile::InputOutputType& type)
    {
        if(!in.good())
            return in;
        IgnoreWhitespace(in);
        std::vector<char> typeStr = ReadUntilWhitespace(in);
        if(StringCaseInsensitiveEqual(str_view(typeStr), "input"))
            type = FgdFile::InputOutputType::Input;
        else if(StringCaseInsensitiveEqual(str_view(typeStr), "output"))
            type = FgdFile::InputOutputType::Output;
        else
        {
            in.seekg(-typeStr.size(), std::ios_base::cur);
            in.setstate(std::ios_base::failbit);
        }
        return in;
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::InputOutputType& type)
    {
        switch(type)
        {
            case FgdFile::InputOutputType::Input:
                out << "input";
                break;
            case FgdFile::InputOutputType::Output:
                out << "output";
                break;
            default:
                throw std::runtime_error("Invalid type of Input/Output type");
        }
        return out;
    }

    // Class >> Input / Output
    std::istream& operator>>(std::istream& in, FgdFile::InputOutput& io)
    {
        // input <name>(<param>) : "comment"
        // output <name>(<param>) : "comment"
        int c;
        if(!in.good())
            return in;

        in >> io.Type;
        if(!in.good())
            return in;

        IgnoreWhitespace(in);

        auto name = ReadUntilWhitespaceOr(in, '(' /* Start of parameter */);
        if(in.peek() != '(') // Ended by whitespace character
            throw std::runtime_error("Input/Output name must be followed by '(' inside which must be a parameter type, for no parameter use \"(void)\"");
        if(name.empty())
            throw std::runtime_error("Input/Output name cannot be empty");
        in.ignore(); // Skip '('
        io.Name = str(name);

        IgnoreWhitespace(in);

        auto param = ReadUntilWhitespaceOrAny(
            in,
            std::array<char, 2> {
                ',', // Multiple parameters = invalid
                ')' // End of parameter
            }
        );
        c = in.peek();
        if(c != ')') // Ended by whitespace character
        {
            IgnoreWhitespace(in);

            c = in.peek();
            if(c != ')') // Whitespace followed by character other than end of the parameter
            {
                if(c == ',')
                    throw std::runtime_error("Input/Output parameter must be followed by ')', there cannot be multiple parameters");
                else
                    throw std::runtime_error("Input/Output parameter must be followed by ')'");
            }
        }
        in.ignore(); // Skip ')'
        if(param.empty())
        {
#ifdef FGD_IO_PARAM_VOID
            param = { 'v', 'o', 'i', 'd' };
#else
            throw std::runtime_error("Input/Output parameter cannot be empty");
#endif
        }
        io.ParamType = str(param);

        IgnoreWhitespace(in);

        // Description
        {
            c = in.peek();
            if(c == ':')
            {
                in.ignore();

                IgnoreWhitespace(in);

                io.Description = str(ReadQuotedString(in));
            }
            else
                io.Description = {};
        }

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::InputOutput& io)
    {
        out << io.Type << ' ' << io.Name << '(';
        if(io.ParamType.empty()) // Just a safety, should not be needed
            out << "void";
        else
            out << io.ParamType;
        out << ')';
        if(!io.Description.empty())
        {
            out << " : \"" << io.Description << '\"';
        }
        return out;
    }

    // Class
    std::istream& operator>>(std::istream& in, FgdFile::Class& clss)
    {
        IgnoreWhitespace(in);
        int c;

        // Header
        {
            c = in.peek();
            if(c != '@')
            {
                in.setstate(std::ios_base::failbit);
                return in;
            }
            in.ignore();

            std::vector<char> type = ReadUntilWhitespaceOr(in, ':' /* Separator between class/option and codename */ );
            clss.Type = str(type);
#ifdef FGD_CLASS_VALIDATE
            {
                bool found = false;
                for(const auto& validClass : FgdFile::Class::ValidTypes)
                {
                    if(StringCaseInsensitiveEqual(validClass, str_view(type)))
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                    throw std::runtime_error("Is '" + str(type) + "' a valid FGD class?");
            }
#endif

            // Options
            while(true)
            {
                IgnoreWhitespace(in);

                c = in.peek();
                if(c == EOF)
                    throw std::runtime_error("End-of-File too soon, class must end by `[]` even when empty");
                else if(c == '=') // End of options
                {
                    in.ignore(); // Skip '='
                    break;
                }
                else // Option
                {
                    FgdFile::Option option;
                    in >> option;
                    if(in.fail())
                        throw std::runtime_error("Failed to parse class option");
                    clss.Options.emplace_back(option);
                }
            }

            IgnoreWhitespace(in);

            {
                std::vector<char> codename = ReadUntilWhitespaceOrAny(
                    in,
                    std::array<char, 2> {
                        ':', // Optional description
                        '[' // Start of body
                    }
                );
                if(codename.empty())
                    throw std::runtime_error("Class codename cannot be empty");
                clss.Codename = str(codename);
            }

            IgnoreWhitespace(in);

            clss.Description = {};
            c = in.peek();
            if(c == ':') // Optional description
            {
                in.ignore(); // Skip ':'

                IgnoreWhitespace(in);

                try
                {
                    std::vector<char> description = ReadQuotedString(in);
                    clss.Description = str(description);
                }
                catch(std::exception& ex)
                {
                    throw std::runtime_error(std::string("Failed to read class description - ") + ex.what());
                }
            }
        }

        // Body
        {
            IgnoreWhitespace(in);

            c = in.peek();
            R_ASSERT(c == '[');
            in.ignore();

            while(true)
            {
                IgnoreWhitespace(in);

                c = in.peek();
                R_ASSERT(in.good() || in.eof());
                if(in.bad() || in.fail())
                    throw std::runtime_error("Failed reading FGD class from stream");
                if(c == ']')
                {
                    in.ignore(); // Skip ']'
                    if(in.bad() || in.fail())
                        throw std::runtime_error("Failed reading FGD class from stream");
                    break;
                }
                else if(c == EOF) [[unlikely]]
                    throw std::runtime_error("End-of-File inside a class");
                else
                {
                    // Input / Output
                    {
                        FgdFile::InputOutput io;
                        in >> io;
                        if(in.good())
                        {
                            clss.IO[io.Name] = io;
                            continue;
                        }
                        else
                        {
                            in.clear(in.rdstate() & ~std::istream::failbit);
                            R_ASSERT(in.good() || in.eof());
                        }
                    }

                    // Property
                    {
                        FgdFile::Property property;
                        in >> property;
                        if(in.good())
                        {
                            clss.Properties[property.Codename] = property;
                            continue;
                        }
                        else
                        {
                            in.clear(in.rdstate() & ~std::istream::failbit);
                            R_ASSERT(in.good() || in.eof());
                        }
                    }

                    throw std::runtime_error("Class can only contain properties and Inputs/Outputs");
                }
            }
        }
        return in;
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::Class& clss)
    {
        { // Header
            R_ASSERT(!clss.Type.empty());
            R_ASSERT(clss.Type.find(' ') == std::string::npos);
            out << '@' << clss.Type;
            if(!clss.Options.empty())
            {
                for(const FgdFile::Option& option : clss.Options)
                    out << ' ' << option;
            }

            R_ASSERT(!clss.Codename.empty());
            R_ASSERT(clss.Codename.find(' ') == std::string::npos);
            out << " = " << clss.Codename;

            if(!clss.Description.empty())
                out << " : \"" << clss.Description << '\"';

            if(clss.Properties.empty() && clss.IO.empty())
            {
                out << " []";
                return out;
            }
            out << '\n';
        }

        out << "[\n";

        // Properties
        for(const auto& property : clss.Properties)
        {
            out << '\t' << property.second << '\n';
        }
        bool hadPrevious = !clss.Properties.empty();

        // Input
        if(FgdFile::Class::HasType(clss.IO, FgdFile::InputOutputType::Input))
        {
            if(hadPrevious) // Just empty line between Inputs and Properties
                out << '\n';

            for(const auto& io: clss.IO)
                if(io.second.Type == FgdFile::InputOutputType::Input)
                    out << '\t' << io.second << '\n';
        }

        // Output
        if(FgdFile::Class::HasType(clss.IO, FgdFile::InputOutputType::Output))
        {
            if(hadPrevious) // Just empty line between Outputs and Inputs/Properties
                out << '\n';

            for(const auto& io: clss.IO)
                if(io.second.Type == FgdFile::InputOutputType::Output)
                    out << '\t' << io.second << '\n';
        }

        out << ']';
        return out;
    }

    // Auto Vis Group - Child
    std::istream& operator>>(std::istream& in, FgdFile::AutoVisGroup_Child& avgc)
    {
        IgnoreWhitespace(in);

        int c = in.peek();
        if(c != '\"')
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }
        std::vector<char> childName = ReadQuotedString(in);
        if(childName.empty())
            throw std::runtime_error("`@AutoVisGroup` child must have a name (empty does not count)");
        avgc.DisplayName = str(childName);

        IgnoreWhitespace(in);

        c = in.peek();
        if(c != '[')
            throw std::runtime_error("`@AutoVisGroup` child must be followed by '=', then name of the group and '[' after it");
        in.ignore();

        while(true) // Entity Classes
        {
            IgnoreWhitespace(in);

            c = in.peek();
            switch(c)
            {
                case '\"':
                {
                    std::vector<char> entityClass = ReadQuotedString(in);
                    if(entityClass.empty())
                        throw std::runtime_error("Entity class inside `@AutoVisGroup` cannot be empty");
                    avgc.EntityClasses.emplace(str(entityClass));
                    break;
                }
                case ']':
                    in.ignore();
                    return in;
                default:
                    throw std::runtime_error("`@AutoVisGroup` child must end by ']' and can only contain entity classes as quoted text (inside '\"')");
            }
        }
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::AutoVisGroup_Child& avgc)
    {
        out << "\t\"" << avgc.DisplayName << "\"\n";
        out << "\t[\n";
        for(const std::string& entityName : avgc.EntityClasses)
        {
            if(entityName.empty())
                continue;
            out << "\t\t\"" << entityName << "\"\n";
        }
        out << "\t]";
        return out;
    }

    // Auto Vis Group
    std::istream& operator>>(std::istream& in, FgdFile::AutoVisGroup& avg)
    {
        IgnoreWhitespace(in);

        int c = in.peek();
        if(c == EOF)
        {
            in.setstate(std::ios_base::eofbit);
            return in;
        }
        if(c != '@')
            throw std::runtime_error("`@AutoVisGroup` must start with '@'");
        std::vector<char> classname = ReadUntilWhitespaceOr(in, '=' /* Start of values */);
        if(!StringCaseInsensitiveEqual(str_view(classname), "@AutoVisGroup"))
        {
            in.seekg(-classname.size(), std::ios_base::cur); // Return object name back to stream
            in.setstate(std::ios_base::failbit);
            return in;
        }

        IgnoreWhitespace(in);

        c = in.peek();
        if(c != '=')
            throw std::runtime_error("`@AutoVisGroup` must be followed by '='");
        in.ignore();

        IgnoreWhitespace(in);

        // Display Name
        {
            std::vector<char> displayName = ReadQuotedString(in);
            if(displayName.empty())
                throw std::runtime_error("`@AutoVisGroup` cannot have empty name");
            avg.DisplayName = str(displayName);
        }

        IgnoreWhitespace(in);

        c = in.peek();
        if(c != '[')
            throw std::runtime_error("`@AutoVisGroup` must be followed by '=', then name of the group and '[' after it");
        in.ignore();

        while(true) // Child
        {
            FgdFile::AutoVisGroup_Child avgc = {};
            in >> avgc;
            if(in.fail())
            {
                in.clear(in.rdstate() & ~std::istream::failbit);
                break;
            }

            avg.Child.emplace_back(avgc);
        }

        c = in.peek();
        if(c != ']')
            throw std::runtime_error("`@AutoVisGroup` must end by ']'");
        in.ignore();

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::AutoVisGroup& avg)
    {
        out << "@AutoVisGroup = \"" << avg.DisplayName << "\"\n";
        out << "[\n";
        for(const auto& ch : avg.Child)
            out << ch << '\n';
        out << ']';
        return out;
    }

    // FGD File
    std::istream& operator>>(std::istream& in, FgdFile& fgd)
    {
        while(true)
        {
            IgnoreWhitespace(in);

            if(in.eof())
                return in;
            int c = in.peek();
            if(in.fail())
                throw std::runtime_error("Failed reading FGD from stream");
            if(c == EOF || in.eof())
                return in;
            if(c != '@')
                throw std::runtime_error("Unexpected character inside FGD file - expected a class (or other object) that starts with '@'");

            std::vector<char> object = ReadUntilWhitespaceOr(in, '(' /* Start of `@mapsize` data */);
#ifdef DEBUG
            std::cout << str_view(object) << std::endl;
#endif

            if(object.empty())
                return in;
            else if(StringCaseInsensitiveEqual(str_view(object), "@mapsize")) [[unlikely]]
            {
                if(fgd.MapSize.has_value())
                    throw std::runtime_error("Single FGD file cannot contain more than one `@mapsize`");

                if(in.peek() != '(')
                    throw std::runtime_error("`@mapsize` must be followed by '(', see documentation");
                in.ignore(); // Skip '('

                IgnoreWhitespace(in);

                // Min
                std::vector<char> valueMin = ReadOnlyNumber(in, true);
                if(valueMin.empty())
                    throw std::runtime_error("Minimum value of `@mapsize` cannot be empty");
                for(int vci = 0; vci < valueMin.size(); vci++)
                {
                    char vc = valueMin[vci];
                    if(vci == 0 && vc == '-') // '-' is only allowed at the begining, '+' is not allowed
                        continue;
                    if(vc >= '0' && vc <= '9')
                        continue;

                    throw std::runtime_error("Minimum value of `@mapsize` contains invalid character");
                }

                IgnoreWhitespace(in);

                // Separator between parameters
                c = in.peek();
                if(c != ',')
                {
                    if(c == ')')
                        throw std::runtime_error("Unexpected character between parameters of `@mapsize`, 2 parameters must be present but ')' was reached after 1st parameter");
                    else
                        throw std::runtime_error("Unexpected character between parameters of `@mapsize`");
                }
                in.ignore(); // Skip ','

                IgnoreWhitespace(in);

                // Max
                std::vector<char> valueMax = ReadOnlyNumber(in, true);
                if(valueMax.empty())
                    throw std::runtime_error("Maximum value of `@mapsize` cannot be empty");
                for(int vci = 0; vci < valueMax.size(); vci++)
                {
                    char vc = valueMax[vci];
                    if(vci == 0 && vc == '-') // '-' is only allowed at the begining, '+' is not allowed
                        continue;
                    if(vc >= '0' && vc <= '9')
                        continue;
                    throw std::runtime_error("Maximum value of `@mapsize` contains invalid character");
                }

                c = in.peek();
                R_ASSERT(c == ')');
                in.ignore();

                int min, max;
                try // Min
                {
                    min = std::stoi(str(valueMin));
                }
                catch(std::out_of_range& ex)
                {
                    throw std::out_of_range("Minimum value of `@mapsize` is out of range of Int32");
                }
                catch(std::invalid_argument& ex)
                {
                    throw std::out_of_range("Value of `@mapsize` minimum value is not valid");
                }
                try // Max
                {
                    max = std::stoi(str(valueMax));
                }
                catch(std::out_of_range& ex)
                {
                    throw std::out_of_range("Maximum value of `@mapsize` is out of range of Int32");
                }
                catch(std::invalid_argument& ex)
                {
                    throw std::out_of_range("Value of `@mapsize` maximum value is not valid");
                }

                fgd.MapSize = glm::i32vec2{ min, max };
            }
            else if(StringCaseInsensitiveEqual(str_view(object), "@include")) [[unlikely]]
            {
                IgnoreWhitespace(in);

                std::vector<char> filename;
                try
                {
                    filename = ReadQuotedString(in);
                }
                catch(std::exception& ex)
                {
                    throw std::runtime_error(std::string("Failed to get `@include` path - ") + ex.what());//OPTIMIZE better joining of `const char[]` and `const char*`
                }
                if(filename.empty())
                    throw std::runtime_error("Cannot `@include` empty path");

                fgd.IncludeFiles.emplace_back(str(filename));
            }
            else if(StringCaseInsensitiveEqual(str_view(object), "@MaterialExclusion")) [[unlikely]]
            {
                IgnoreWhitespace(in);

                c = in.peek();
                if(c != '[')
                    throw std::runtime_error("Content of `@MaterialExclusion` must be inside square brackets `[` + `]`");
                in.ignore();

                while(true)
                {
                    IgnoreWhitespace(in);

                    c = in.peek();
                    switch(c)
                    {
                        case '\"':
                        {
                            std::vector<char> dir = ReadQuotedString(in);
                            if(dir.empty())
                                throw std::runtime_error("Entry of `@MaterialExclusion` cannot be empty text (just `\"\"`), if you want the list to be empty just use `[]`");

                            fgd.MaterialExclusion.emplace(str(dir));
                            break;
                        }

                        case ']': // End of content
                            in.ignore(); // Skip ']'
                            //break 2;
                            goto GOTO_OUTSIDE_MATERIAL_EXCLUSION_WHILE;
                        case EOF: // End of file (let's be lenient and allow it as the end of list)
                            break;

                        case ',':
                            throw std::runtime_error("Content of `@MaterialExclusion` must end by `]` and contain only names of directories quoted by '\"', entries are not separated by ','");
                        default:
                            throw std::runtime_error("Content of `@MaterialExclusion` must end by `]` and contain only names of directories quoted by '\"'");
                    }
                }
                GOTO_OUTSIDE_MATERIAL_EXCLUSION_WHILE:;
            }
            else if(StringCaseInsensitiveEqual(str_view(object), "@AutoVisGroup")) [[unlikely]]
            {
                in.seekg(-object.size(), std::ios_base::cur); // Return object name back to stream

                FgdFile::AutoVisGroup avg;
                in >> avg;
                if(in.fail())
                    throw std::runtime_error("Failed to parse @AutoVisGroup");

                //TODO Create function `bool TryCombine(AutoVisGroup& from)`

                // Add to existing if possible
                bool found_avg = false;
                for(auto& fgd_avg : fgd.AutoVisGroups) // Loop through existing AutoVisGroups
                {
                    if(fgd_avg.DisplayName == avg.DisplayName)
                    {
                        for(auto& avgc : avg.Child) // Loop through child that we want to insert
                        {
                            bool found_avgc = false;
                            for(auto& fgd_avgc : fgd_avg.Child) // Loop through child that exist - try to find matching ones
                            {
                                if(fgd_avgc.DisplayName == avgc.DisplayName)
                                {
                                    for(auto& avgc_e : avgc.EntityClasses) // Loop through entities we want in the AutoVisGroup's Child
                                        fgd_avgc.EntityClasses.emplace(avgc_e);

                                    found_avgc = true;
                                    break;
                                }
                            }

                            if(!found_avgc)
                                fgd_avg.Child.emplace_back(avgc);
                        }

                        found_avg = true;
                        break;
                    }
                }

                if(!found_avg)
                    fgd.AutoVisGroups.emplace_back(avg);
            }
            else // Class ?
            {
                in.seekg(-object.size(), std::ios_base::cur); // Return object name back to stream

                FgdFile::Class clss;
                in >> clss;
                if(in.fail())
                    throw std::runtime_error("Failed to read class inside FGD file");
#ifdef DEBUG
                std::cout << clss.Codename << std::endl;
#endif
                if(fgd.Classes.contains(clss.Codename))
                    throw std::runtime_error("Class '" + clss.Codename + "' already exists in the FGD file");
                fgd.Classes.emplace(clss.Codename, clss);
            }
        }
        return in;
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile& fgd)
    {
        // Comment header
        {
            out << "// Generated FGD file\n";

            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            out << "// " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '\n';
        }

        if(fgd.MapSize.has_value())
        {
            out << '\n';
            auto val = fgd.MapSize.value();
            out << "// " << (val.y - val.x) << " total\n";
            out << "@mapsize(" << val.x << ", " << val.y << ")\n";
        }

        if(!fgd.IncludeFiles.empty())
        {
            out << '\n';
            for(const auto& filePath : fgd.IncludeFiles)
                out << "@include \"" << filePath << "\"\n";
        }

        if(!fgd.MaterialExclusion.empty())
        {
            out << '\n';
            out << "@MaterialExclusion\n";
            out << "[\n";
            out << "\t// Names of the sub-directories we don't want to load materials from\n";
            for(const auto& dirName : fgd.MaterialExclusion)
                out << "\t\"" << dirName << "\"\n";
            out << "]\n";
        }

        if(!fgd.Classes.empty())
        {
            out << '\n';
            out << "// " << fgd.Classes.size() << " classes\n";
            for(const auto& cls : fgd.Classes)
                out << cls.second << '\n';
        }

        if(!fgd.AutoVisGroups.empty())
        {
            out << '\n';
            out << "// " << fgd.AutoVisGroups.size() << " Auto Vis Groups\n";
            for(const auto& avg : fgd.AutoVisGroups)
                out << avg << '\n';
        }

        return out;
    }
}
