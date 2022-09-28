#include "BspEntities.hpp"

namespace Decay::Bsp::v30
{
    void BspEntities::ExportJson(const std::filesystem::path& filename) const
    {
        std::fstream out = std::fstream(filename, std::ios_base::out);
        out << "{" << std::endl;
        out << "  \"entities\": [" << std::endl;
        for(int e = 0; e < Entities.size(); e++)
        {
            const auto& entity = Entities[e];

            out << "    {" << std::endl;
            auto it = entity.begin();
            for(int ei = 0; ei < entity.size(); ei++, it++)
            {
                const auto& kv = *it;
                out << "      \"" << kv.first << "\": \"" << kv.second << "\""; //TODO escape `"` and `\` in `kv.second`
                if(ei != entity.size() - 1) //TODO change `ei` to `remaining` and check against it
                    out << ",";
                out << std::endl;
            }
            out << "    }";
            if(e != Entities.size() - 1)
                out << ",";
            out << std::endl;
        }
        out << "  ]" << std::endl;
        out << "}" << std::endl;
    }
#ifdef DECAY_JSON_LIB
    nlohmann::json BspEntities::AsJson() const
    {
        using namespace nlohmann;
        json j = {};
        {
            json& jEntities = j["entities"];
            jEntities = json::array();
            for(const auto& entity : Entities)
            {
                json je = {};
                {
                    for(const auto& kv : entity)
                        je[kv.first] = kv.second;
                }
                jEntities.emplace_back(je);
            }
        }
        return j;
    }
    BspEntities::BspEntities(const nlohmann::json& j)
    {
        const auto& jEntities = j.find("entities");
        if(jEntities != j.end())
        {
            if(jEntities->type() != nlohmann::detail::value_t::array && jEntities->type() != nlohmann::detail::value_t::null)
                throw std::runtime_error("BSP Entities must be a JSON array");

            for(const nlohmann::json& jEnt : *jEntities)
            {
                Entity ent;
                {
                    for(nlohmann::json::const_iterator it = jEnt.begin(); it != jEnt.end(); ++it)
                        ent[it.key()] = it.value();
                }
                emplace(ent);
            }
        }
    }
#endif
}
