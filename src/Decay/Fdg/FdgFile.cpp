#include "FdgFile.hpp"

#include <optional>

namespace Decay::Fdg
{
    FdgFile::FdgFile(const std::filesystem::path& filename)
    {
        if(!std::filesystem::exists(filename))
            throw std::runtime_error("File not found");

        std::ifstream in(filename, std::ios_base::in);

        /*
        std::string line_;
        while(std::getline(in, line_))
        {
            // Empty line
            if(line_.empty())
                continue;

            // Trim spaces (at the begining)
            std::string_view line = line_;
            while(!line.empty() && (line[0] == ' ' || line[0] == '\t'))
            {
                line = std::string_view(line.data() + 1, line.length() - 1);
            }
            if(line.empty())
                continue;

            // Comment line
            if(line.length() >= 2 && line[0] == '/' && line[1] == '/')
                continue;
        }
        */

        Entity entity;
        bool entityHeaderComplete = false;

        typedef std::pair<std::string, std::vector<std::string>> EntityHeaderWord_t;
        while(in.good())
        {
            if(entityHeaderComplete) // Inside entity
            {
                EntityHeaderWord_t v = ReadEntityHeaderWord(in);
                assert(!v.first.empty());
                std::string codename = v.first;
                assert(!v.second.empty());
                assert(v.second.size() == 1);
                std::string type = v.second[0];

                v = ReadEntityHeaderWord(in);
                std::string displayname = {};
                if(v.first == ":") // Has displayname
                {
                    v = ReadEntityHeaderWord(in);

                    assert(!v.first.empty());
                    assert(v.second.empty());
                    displayname = v.first;

                    v = ReadEntityHeaderWord(in);
                }

                std::string defaultValue = {};
                if(v.first == ":") // Default value
                {
                    v = ReadEntityHeaderWord(in);

                    assert(!v.first.empty());
                    assert(v.second.empty()); // light value is same as other vectors - string with numbers separated by spaces
                    defaultValue = v.first;

                    v = ReadEntityHeaderWord(in);
                }

                if(v.first == "=") // Flags or choices value
                {
                    v = ReadEntityHeaderWord(in);
                    assert(v.first == "[");

                    while(true)
                    {
                        v = ReadEntityHeaderWord(in);
                        if(v.first == "]")
                            break;
                        //TODO One of following:
                        // 0: "Some text"
                        // 0: "Some text" : 0
                        // 0 : "Some text"
                        // 0 : "Some text" : 0
                    }

                    v = ReadEntityHeaderWord(in);
                }

                //TODO iterate over "lines"
            }
            else // Outside entity (its header)
            {
                EntityHeaderWord_t v = ReadEntityHeaderWord(in);
                EntryType entityType = to_enum<EntryType>(v.first);
                assert(v.second.empty());

                // size(0 0 0, 0 0 0)
                // color(0, 255, 0)
                // base(Something)
                // decal()
                // sprite()
                std::vector<EntityHeaderWord_t> options = {};
                v = ReadEntityHeaderWord(in);
                while(v.first != "=")
                {
                    options.emplace_back(v);
                    v = ReadEntityHeaderWord(in);
                }
                assert(v.first == "=");
                assert(v.second.empty());

                // Codename
                v = ReadEntityHeaderWord(in);
                std::string codename = v.first;
                assert(!v.first.empty());
                assert(v.first != "=");
                assert(v.first != ":");
                assert(v.first != "[");
                assert(v.second.empty());
                v = ReadEntityHeaderWord(in);
                if(v.first == ":") // Display Name
                {
                    v = ReadEntityHeaderWord(in);

                    std::string displayname = v.first;
                    assert(v.second.empty());
                    v = ReadEntityHeaderWord(in);
                }
                if(v.first == "[") // Start of body
                {
                    assert(v.second.empty());
                    entityHeaderComplete = true;
                    continue;
                }
                if(v.first == "[]") // No body
                {
                    Entities.emplace_back(entity);
                    entity = {};
                    continue;
                }
                throw std::runtime_error("Reached end of header but no square bracket was found");
            }
        }
    }

    std::pair<std::string, std::vector<std::string>> FdgFile::ReadEntityHeaderWord(std::istream& in)
    {
        char c;
LOOPBACK_BEGIN:
        c = in.peek();
        switch(c)
        {
            // Quoted text
            // Cannot contain values
            case '\"':
                in.ignore();
                return { ReadQuotedString(in, '\"'), {}};
            // Whitespace char
            case '\r':
            case '\n':
            case ' ':
            case '\t':
                in.ignore();
                goto LOOPBACK_BEGIN;
            default:
                break;
        }

        // Read key
        std::vector<char> key = {};
LOOPBACK_KEY:
        c = in.peek();
        switch(c)
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n': // No round brackets, ends with just a key
                return { std::string(key.data(), key.size()), {} };
            case '(': // Start of values
                in.ignore();
                break;
            default: // Normal character, part of the key
                key.emplace_back(c);
                in.ignore();
                goto LOOPBACK_KEY;
        }

        // Read value
        assert(c == '(');
        in.ignore();

        std::vector<std::string> values = {};
        std::vector<char> val = {};
LOOPBACK_VALUE:
        c = in.peek();
        switch(c)
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                if(val.empty())
                    goto LOOPBACK_VALUE; // No value yet so ignore the character
            default: // Normal valid character
                key.emplace_back(c);
                in.ignore();
                goto LOOPBACK_KEY;
            case ',': // Value separator
                in.ignore();
                if(!val.empty())
                {
                    values.emplace_back(std::string(val.data(), val.size()));
                    val.clear();
                }
                goto LOOPBACK_KEY;
            case ')': // End of values
                in.ignore();
                if(!val.empty())
                    values.emplace_back(std::string(val.data(), val.size()));
                return { std::string(key.data(), key.size()), values };
        }
    }
}
