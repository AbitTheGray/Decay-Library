#include "FgdFile.hpp"

#ifdef DECAY_JSON_LIB
namespace nlohmann
{
    using namespace Decay::Fgd;
    template<>
    struct adl_serializer<FgdFile::Class>
    {
        inline static void to_json  (      json& j, const FgdFile::Class&);
        //inline static void from_json(const json& j,       FgdFile::Class&);
    };
}

namespace nlohmann
{
    using namespace Decay::Fgd;
    void adl_serializer<FgdFile::Class>::to_json(json& j, const FgdFile::Class& clss)
    {
        j["type"] = clss.Type;
        if(!clss.Options.empty())
        {
            json& jOptions = j["options"];
            jOptions = json::array();

            for(const auto& option : clss.Options)
            {
                json jOption = {};
                {
                    jOption["name"] = option.Name;

                    if(!option.Params.empty())
                    {
                        json& jParams = jOption["params"];
                        for(const auto& optionParam : option.Params)
                        {
                            if(optionParam.Quoted)
                                jParams.emplace_back("\"" + optionParam.Name + "\"");
                            else
                                jParams.emplace_back(optionParam.Name);
                        }
                    }
                }
                jOptions.emplace_back(jOption);
            }
        }
        j["codename"] = clss.Codename;
        if(!clss.Description.empty())
            j["description"] = clss.Description;

        if(!clss.Properties.empty())
        {
            json& jProperties = j["properties"];
            jProperties = json::array();
            for(const auto& cl : clss.Properties)
            {
                json jProp = {};
                jProp["codename"] = cl.Codename;
                jProp["type"] = cl.Type;
                if(cl.ReadOnly)
                    jProp["readonly"] = true;
                if(!cl.DisplayName.empty())
                    jProp["name"] = cl.DisplayName;
                if(!cl.DefaultValue.empty())
                    jProp["default"] = cl.DefaultValue;

                if(!cl.FlagsOrChoices.empty())
                {
                    if(cl.Type == "flags")
                    {
                        json& jFlags = jProp["flags"];
                        for(const auto& flag : cl.FlagsOrChoices)
                        {
                            json jFlag = {};
                            {
                                jFlag["name"] = flag.DisplayName;
                                jFlag["default"] = flag.Default;
                            }
                            jFlags[flag.Index] = jFlag;
                        }
                    }
                    else
                    {
                        json& jFlags = jProp["choices"];
                        for(const auto& flag : cl.FlagsOrChoices)
                            jFlags[flag.Index] = flag.DisplayName;
                    }
                }

                jProperties.emplace_back(jProp);
            }
        }
        if(FgdFile::Class::HasType(clss.IO, FgdFile::InputOutputType::Input))
        {
            json& jProperties = j["inputs"];
            jProperties = json::array();
            for(const auto& io : clss.IO)
            {
                if(io.Type != FgdFile::InputOutputType::Input)
                    continue;

                json jIO = {};
                jIO["name"] = io.Name;
                jIO["type"] = io.Type;
                if(!io.Description.empty())
                    jIO["description"] = io.Description;

                jProperties.emplace_back(jIO);
            }
        }
        if(FgdFile::Class::HasType(clss.IO, FgdFile::InputOutputType::Output))
        {
            json& jProperties = j["outputs"];
            jProperties = json::array();
            for(const auto& io : clss.IO)
            {
                if(io.Type != FgdFile::InputOutputType::Output)
                    continue;

                json jIO = {};
                jIO["name"] = io.Name;
                jIO["type"] = io.Type;
                if(!io.Description.empty())
                    jIO["description"] = io.Description;

                jProperties.emplace_back(jIO);
            }
        }
    }
}

namespace Decay::Fgd
{
    nlohmann::json FgdFile::ExportAsJson(bool orderClassesByDependency) const
    {
        using namespace nlohmann;
        ordered_json j = {};
        {
            // Includes
            if(!IncludeFiles.empty())
            {
                ordered_json& jObj = j["include_files"];
                jObj = json::array();
                for(const auto& include : IncludeFiles)
                    jObj.emplace_back(include);
            }

            // Map size
            if(MapSize.has_value())
            {
                j["map_size"] = { MapSize.value().x, MapSize.value().y };
            }

            // Material Exclusion
            if(!MaterialExclusion.empty())
            {
                ordered_json& jObj = j["material_exclusion"];
                jObj = json::array();
                for(const auto& materialDir : MaterialExclusion)
                    jObj.emplace_back(materialDir);
            }

            // Auto Vis Group
            if(!AutoVisGroups.empty())
            {
                ordered_json& jObj = j["auto_vis_groups"];
                jObj = json::object();
                for(const auto& avg : AutoVisGroups)
                {
                    json jAVG = {};
                    {
                        for(const auto& avgc : avg.Child)
                        {
                            json jAVGC = json::array();
                            {
                                for(const auto& ec : avgc.EntityClasses)
                                    jAVGC.emplace_back(ec);
                            }
                            jAVG[avgc.DisplayName] = jAVGC;
                        }
                    }
                    jObj[avg.DisplayName] = jAVG;
                }
            }

            // Classes
            if(!Classes.empty())
            {
                ordered_json& jObj = j["classes"];
                jObj = ordered_json::array();//TODO Use nlohmann::ordered_json
                if(orderClassesByDependency)
                {
                    std::vector<std::string> orderedClasses = OrderClassesByDependency();
                    R_ASSERT(orderedClasses.size() == Classes.size());
                    for(const auto& clssName : orderedClasses)
                    {
                        bool found = false;
                        for(const auto& clss : Classes) // Will process only 1 class with correct name (key of the map)
                        {
                            //THINK use lower-case name as `key` of `Classes` to be able to use lookup
                            if(StringCaseInsensitiveEqual(clss.first, clssName)) // Correct class
                            {
                                json jClss = clss;
                                jObj.emplace_back(std::move(jClss)); // Cannot use `clss` directly as it uses `json` and not `ordered_json`

                                found = true;
                                break;
                            }
                        }
                        R_ASSERT(found);
                    }
                }
                else
                {
                    for(const auto& clss : Classes)
                    {
                        json jClss = clss;
                        jObj.emplace_back(std::move(jClss)); // Cannot use `clss` directly as it uses `json` and not `ordered_json`
                    }
                }
            }
        }
        return j;
    }
}
#endif
