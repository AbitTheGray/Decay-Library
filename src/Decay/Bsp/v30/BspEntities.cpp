#include "BspEntities.hpp"

#include "Decay/CommonReadUtils.hpp"

namespace Decay::Bsp::v30
{
    /*
    std::vector<std::map<std::string, std::string>> BspEntities::ParseEntities(const char* raw, std::size_t len)
    {
        std::vector<std::map<std::string, std::string>> entities = {};
        std::map<std::string, std::string> entity = {};

        const char* keyStart = nullptr;
        const char* keyEnd = nullptr;

        const char* valueStart = nullptr;

        for(std::size_t i = 0; i < len; i++)
        {
            char c = raw[i];
            switch(c)
            {
                case '\0':
                    [[unlikely]]
                {
                    break;
                }

                    // Start of entity
                case '{':
                {
                    if(!entity.empty())
                    {
                        if(keyStart == nullptr || (keyEnd != nullptr && valueStart == nullptr))
                            std::cerr << "Non-empty entity at start of new entity" << std::endl;
                    }
                    break;
                }

                    // End of entity
                case '}':
                {
                    if(entity.empty())
                    {
                        std::cerr << "Empty entity after its end" << std::endl;
                        break;
                    }

                    entities.emplace_back(entity);
                    entity.clear();
                    break;
                }

                    // Key/Value encasement characters
                case '"':
                {
                    if(keyStart == nullptr)
                        keyStart = raw + i + 1;
                    else if(keyEnd == nullptr)
                        keyEnd = raw + i;
                    else if(valueStart == nullptr)
                        valueStart = raw + i + 1;
                    else // valueEnd
                    {
                        entity.emplace(
                            std::string(keyStart, keyEnd),
                            std::string(valueStart, raw + i)
                        );

                        keyStart = nullptr;
                        keyEnd = nullptr;
                        valueStart = nullptr;
                    }
                }

                    // White character, valid anywhere
                case ' ':
                    break;

                    // New line
                case '\n':
                {
                    if(keyStart != nullptr)
                    {
                        std::cerr << "Unexpected new-line character" << std::endl;
                        break;
                    }
                    break;
                }

                    // Special characters
                case '\t':
                case '\b':
                    [[unlikely]]
                {
                    std::cerr << "Unsupported " << (int)c << " character" << std::endl;
                    break;
                }

                    // Content of entity
                default:
                    [[likely]]
                {
                    if(keyStart == nullptr)
                        [[unlikely]]
                    {
                        std::cerr << "Unexpected character '" << c << "' (" << (int)c << ") in key" << std::endl;
                        break;
                    }
                    if(keyEnd != nullptr && valueStart == nullptr)
                        [[unlikely]]
                    {
                        std::cerr << "Unexpected character '" << c << "' (" << (int)c << ") in value" << std::endl;
                        break;
                    }
                    break;
                }
            }
        }

        if(keyStart != nullptr)
            std::cerr << "Incomplete key-value pair after entity processing" << std::endl;
        if(!entity.empty())
            std::cerr << "Non-empty entity after entity processing" << std::endl;

        return entities;
    }
    */
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
}
