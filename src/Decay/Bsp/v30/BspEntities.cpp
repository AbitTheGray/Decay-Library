#include "BspEntities.hpp"

#include "Decay/CommonReadUtils.hpp"

namespace Decay::Bsp::v30
{
    std::vector<std::map<std::string, std::string>> BspEntities::ParseEntities(std::istream& in)
    {
        std::vector<std::map<std::string, std::string>> entities = {};
        std::map<std::string, std::string> entity = {};

        int c;

        while(true) // Entity
        {
            IgnoreWhitespace(in);
            if(in.eof())
                break;
            R_ASSERT(in.good());

            c = in.get();
            if(c == EOF)
                break;
            if(c != '{')
                throw std::runtime_error("Entity starts by invalid character");

            while(true)
            {
                IgnoreWhitespace(in);
                R_ASSERT(in.good());

                c = in.peek();
                if(c == '}')
                    break;
                else if(c == '\"')
                {
                    std::vector<char> key = ReadQuotedString(in);
                    IgnoreWhitespace(in);
                    std::vector<char> value = ReadQuotedString(in);
                    IgnoreWhitespace(in);

                    entity.emplace(str(key), str(value));
                }
                else
                    throw std::runtime_error("Unexpected character inside entity");
            }
            IgnoreWhitespace(in);
            R_ASSERT(in.good());

            c = in.get();
            R_ASSERT(c == '}');

            entities.emplace_back(std::move(entity));
            entity = {};
        }

        if(!entity.empty())
            std::cerr << "Non-empty entity after entity processing" << std::endl;

        return entities;
    }
    void BspEntities::ProcessIntoFastAccess()
    {
        Entities_Type.clear();
        Entities_Name.clear();
        Entities_Model.clear();

        // Process entities into fast-access maps
        for(const Entity& ent : Entities)
        {
            auto classname = ent.find("classname");
            auto name = ent.find("targetname");
            auto model = ent.find("model");

            if(classname != ent.end())
                Entities_Type[classname->second].emplace_back(ent);

            if(name != ent.end())
                Entities_Name[name->second].emplace_back(ent);

            if(model != ent.end())
            {
                if(model->second.size() > 1 && model->second[0] == '*')
                {
                    try
                    {
                        int value = std::stoi(model->second.data() + 1);
                        Entities_Model.emplace(value, ent);
                    }
                    catch(std::invalid_argument& ex)
                    {
                        std::cerr << "Model '" << (model->second.data() + 1) << "' could not be parsed - No Conversion" << std::endl;
                    }
                    catch(std::out_of_range& ex)
                    {
                        std::cerr << "Model '" << (model->second.data() + 1) << "' could not be parsed - Out of Range" << std::endl;
                    }
                }
            }
        }
    }
    std::ostream& operator<<(std::ostream& out, const BspEntities& entities)
    {
        for(int i = 0; i < entities.size(); i++)
        {
            const auto& entity = entities[i];

            out << "{\n";
            for(const auto& kv : entity)
            {
                out << "\"" << kv.first << "\" \"" << kv.second << "\"\n";
            }
            out << "}\n";
        }
        out << '\0';

        return out;
    }
}
