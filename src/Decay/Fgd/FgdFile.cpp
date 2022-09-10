#include "FgdFile.hpp"

// Uncomment to use STD implementation of whitespace detection.
//#define FGD_WHITESPACE_STD

namespace Decay::Fgd
{
    /// Convert `std::vector<char>` to `std::string`
    [[nodiscard]] inline static std::string str(const std::vector<char>& in)
    {
        if(in.empty())
            return std::string();
        return std::string(in.data(), in.size());
    }
    /// Represent `std::vector<char>` as `std::string_view`
    [[nodiscard]] inline static std::string_view str_view(const std::vector<char>& in)
    {
        if(in.empty())
            return std::string_view();
        return std::string_view(in.data(), in.size());
    }

    /// Simple logic to decide whenever an ASCII character is a whitespace character.
    [[nodiscard]] inline static constexpr bool IsWhitespace(char c) noexcept
    {
#ifdef FGD_WHITESPACE_STD
        return std::isspace(static_cast<unsigned char>(c));
#else
        switch(c)
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                return true;
            default:
                return false;
        }
#endif
    }
    /// Skips (ignores) all whitespace characters inside provided stream.
    /// Also skips all comments.
    inline static int IgnoreWhitespace(std::istream& in)
    {
        int ignoredChars = 0;

        while(in.good())
        {
            int c = in.peek();
            if(c == EOF) // End of File
                return ignoredChars;
            if(!IsWhitespace(static_cast<char>(c))) // Not whitespace
            {
                if(c == '/') // Potentially start of a comment
                {
                    in.ignore(); // Skip 1st '/'

                    c = in.peek();
                    if(c == '/') // Confirmed, there is a comment
                    {
                        in.ignore(); // Skip 2nd '/'

                        // Skip all characters until newline is reached
                        while(true)
                        {
                            c = in.get();
                            if(c == EOF)
                                break;
                            if(c == '\n' || c == '\r')
                                break;
                        }
                    }
                    else // Not a comment
                    {
                        in.seekg(-1, std::ios_base::cur); // Return 1st '/' into the stream
                        break; // We reached non-whitespace character
                    }
                }
                else
                    break;
            }
            ignoredChars++;
            in.ignore();
        }

        return ignoredChars;
    }

    /// Utility function to open file for read (text).
    [[nodiscard]] inline static std::fstream CheckAndOpenFileForRead(const std::filesystem::path& filename)
    {
        if(!std::filesystem::exists(filename))
            throw std::runtime_error("File not found");
        if(!std::filesystem::is_regular_file(filename))
            throw std::runtime_error("Filename does not point to a found");

        return std::fstream(filename, std::ios_base::in);
    }

    /// Checks whenever provided character is UTF-8 character (works for both first and any following characters).
    [[nodiscard]] inline static bool IsUtf8Char(char c) { return (c & 0xb1000'0000) != 0; }
    /// Get total number of UTF-8 characters (for one codepoint of value) from 1st character.
    /// Returns `0` for invalid input.
    [[nodiscard]] inline static int GetUtf8Chars(char c) noexcept
    {
        if(!IsUtf8Char(c))
            return 1;
        if((c & 0xb1100'0000) == 0xb1000'0000)
            return 0;
        if((c & 0xb1110'0000) == 0xb1100'0000)
            return 2;
        if((c & 0xb1111'0000) == 0xb1110'0000)
            return 3;
        if((c & 0xb1111'1000) == 0xb1111'0000)
            return 4;
        return 0;
    }

    /// Read single UTF-8 char (1 to 4 bytes).
    /// Returns whenever reading succeeded.
    [[nodiscard]] inline static bool ReadChar(std::istream& in, std::vector<char>& str)
    {
        if(!in.good())
            return false;
        int c = in.peek();
        if(c == EOF)
            return false;

        int charCount = GetUtf8Chars(static_cast<char>(c));
        if(charCount == 0)
            return false;

        str.resize(str.size() + charCount);
        in.read(str.data() + (str.size() - charCount), charCount);
        if(in.good())
            return true;
        else
        {
            str.resize(str.size() - charCount);
            return false;
        }
    }
    /// Read from input stream until ASCII character is reached.
    [[nodiscard]] inline static std::vector<char> ReadUntil(std::istream& in, char untilChar, bool skipUntilChar = false)
    {
        std::vector<char> rtn = {};
        while(true)
        {
            int c = in.peek();
            if(c == EOF)
                return rtn;
            if(c == untilChar)
            {
                if(skipUntilChar)
                    in.ignore(); // Skip peeked char
                return rtn;
            }

            if(!ReadChar(in, rtn)) // Read into `rtn`
                throw std::runtime_error("Failed to read a char");
        }
    }
    /// Read from input stream until one of ASCII characters is reached.
    template<std::size_t N>
    [[nodiscard]] inline static std::vector<char> ReadUntilAny(std::istream& in, std::array<char, N> untilChars, bool skipUntilChar = false)
    {
        std::vector<char> rtn = {};
        while(true)
        {
            int c = in.peek();
            if(c == EOF)
                return rtn;

            for(std::size_t i = 0; i < N; i++)
            {
                if(c == untilChars[i])
                {
                    if(skipUntilChar)
                        in.ignore(); // Skip peeked char
                    return rtn;
                }
            }

            if(!ReadChar(in, rtn)) // Read into `rtn`
                throw std::runtime_error("Failed to read a char");
        }
    }
    /// Read from input stream until whitespace character is reached.
    [[nodiscard]] inline static std::vector<char> ReadUntilWhitespace(std::istream& in, bool skipUntilChar = false)
    {
        std::vector<char> rtn = {};
        while(true)
        {
            int c = in.peek();
            if(c == EOF)
                return rtn;
            if(IsWhitespace(c))
            {
                if(skipUntilChar)
                    in.ignore(); // Skip peeked char
                return rtn;
            }

            if(!ReadChar(in, rtn)) // Read into `rtn`
                throw std::runtime_error("Failed to read a char");
        }
    }

    /// Read quoted text.
    /// Sets `failbit` on End-of-File or other than '"' char.
    [[nodiscard]] inline static std::vector<char> ReadQuotedString(std::istream& in)
    {
        IgnoreWhitespace(in);

        int c = in.peek();
        if(c == EOF) // End of file
        {
            in.setstate(std::ios_base::failbit);
            return {};
        }
        if(c != '\"') // Not quoted text, no processing
        {
            in.setstate(std::ios_base::failbit);
            return {};
        }
        in.ignore(); // Skip the `\"` char

        auto name = ReadUntil(in, '\"', true);
        while(!name.empty() && name[name.size() - 1] == '\\') // Last char is `\` which mean there was `\"` found.
        {
            name.emplace_back('\"');

            auto name2 = ReadUntil(in, '\"', true);
            if(name2.empty())
                break;

            { // Insert `name2` into `name`
                name.reserve(name2.size());
                for(int n2i = 0; n2i < name2.size(); n2i++)
                    name.emplace_back(name2[n2i]);
            }
        }

        // Replace "\n" by newline character
        for(int ci = 0; ci < name.size(); ci++)
        {
            if(name[ci] == '\\')
            {
                if(ci + 1 >= name.size())
                    throw std::runtime_error("Text cannot end by `\\` character");
                switch(name[ci + 1])
                {
                    case 'n': // \n
                        name[ci] = '\n';
                        name.erase(name.begin() + (ci + 1)); // Set first char to `\n` and erase the second one
                        break;
                    case '\\': // \\
                            name.erase(name.begin() + (ci + 1)); // Erase the second `\`
                        break;
                    case '\"': // \"
                        name.erase(name.begin() + ci); // Erase the `\` and leave `"` there
                        break;
                    default:
                        throw std::runtime_error("Unsupported escaped character");
                }
            }
        }

        return name;
    }
}

namespace Decay::Fgd
{
    FgdFile::FgdFile(std::istream& in)
    {

    }

    std::istream& operator>>(std::istream& in, FgdFile::PropertyFlag& propertyFlag)
    {
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
            propertyFlag.Default = (str.empty() || str[0] == '0');
        }
        else
            propertyFlag.Default = false;

        return in;
    }
    std::ostream& FgdFile::PropertyFlag::Write(std::ostream& out, bool includeDefault) const
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

    std::istream& operator>>(std::istream& in, FgdFile::OptionParam& optionParam)
    {
        // This script processed only single `param` from the preview below:
        // key()
        // key(param)
        // key(param, param)
        // key(param, param, param)

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
            auto name = ReadUntilAny(
                in,
                std::array<char, 6>{
                    ' ', '\t', // Whitespace
                    '\r', '\n', // New-line
                    ',', // Start of next parameter
                    ')' // End of parameters
                }
            );
            if(name.empty())
            {
                in.setstate(std::ios_base::failbit);
                return in;
            }
            optionParam.Name = str(name);
            return in;
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

    std::istream& operator>>(std::istream& in, FgdFile::Option& option)
    {
        IgnoreWhitespace(in);
        auto name = ReadUntilAny(
            in,
            std::array<char, 6>{
                ' ', '\t', // Whitespace
                '\r', '\n', // New-line
                '(' // Start of parameters
            }
        );
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
        assert(!option.Name.empty());
        assert(option.Name.find(' ') == std::string::npos); //TODO Check for other than alphanumeric characters
        out << option.Name;
        if(!StringCaseInsensitiveEqual(option.Name, "halfgridsnap")) // `halfgridsnap` is exception and does not have `()` after it.
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

    std::istream& Fgd::operator>>(std::istream& in, FgdFile::Property&)
    {

    }
    std::ostream& Fgd::operator<<(std::ostream& out, const FgdFile::Property& property)
    {
        assert(!property.Codename.empty());
        assert(!property.Type.empty());
        out << property.Codename << '(' << property.Type << ')';
        for(const FgdFile::Option& option : property.Options)
            out << ' ' << option;

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

                for(const FgdFile::PropertyFlag& pf : property.FlagsOrChoices)
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

                for(const FgdFile::PropertyFlag& pf : property.FlagsOrChoices)
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

    std::istream& Fgd::operator>>(std::istream& in, FgdFile::InputOutputType& type)
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
    std::ostream& Fgd::operator<<(std::ostream& out, const FgdFile::InputOutputType& type)
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

    std::istream& Fgd::operator>>(std::istream& in, FgdFile::InputOutput& io)
    {
        // input <name>(<param>) : "comment"
        // output <name>(<param>) : "comment"
        FgdFile::InputOutputType type;
        in >> type;
        if(!in.good())
            return in;

        IgnoreWhitespace(in);

        auto name = ReadUntilAny(
            in,
            std::array<char, 5> {
                ' ', '\t', // Whitespace
                '\r', '\n', // New-line
                '(' // Start of parameter
            }
        );
        if(in.peek() != '(') // Ended by whitespace character
            throw std::runtime_error("Input/Output name must be followed by '(' inside which must be a parameter type, for no parameter use \"(void)\"");
        if(name.empty())
            throw std::runtime_error("Input/Output name cannot be empty");
        in.ignore(); // Skip '('
        io.Name = str(name);

        IgnoreWhitespace(in);

        auto param = ReadUntilAny(
            in,
            std::array<char, 6> {
                ' ', '\t', // Whitespace
                '\r', '\n', // New-line
                ',', // Multiple parameters = invalid
                ')' // End of parameter
            }
        );
        if(in.peek() != ')') // Ended by whitespace character
        {
            IgnoreWhitespace(in);
            int c = in.peek();
            if(c != ')') // Whitespace followed by character other than end of the parameter
            {
                if(c == ',')
                    throw std::runtime_error("Input/Output parameter must be followed by ')', there cannot be multiple parameters");
                else
                    throw std::runtime_error("Input/Output parameter must be followed by ')'");
            }
        }
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
            int c = in.peek();
            if(c == ':')
            {
                in.ignore();

                IgnoreWhitespace(in);

                auto description = ReadQuotedString(in);
                io.Description = str(description);
            }
            else
                io.Description = {};
        }

        return in;
    }
    std::ostream& Fgd::operator<<(std::ostream& out, const FgdFile::InputOutput& io)
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

    std::istream& Fgd::operator>>(std::istream& in, FgdFile::Class& clss)
    {

    }
    std::ostream& Fgd::operator<<(std::ostream& out, const FgdFile::Class& clss)
    {
    }

    std::istream& Fgd::operator>>(std::istream& in, FgdFile::AutoVisGroup_Child& avgc)
    {

    }
    std::ostream& Fgd::operator<<(std::ostream& out, const FgdFile::AutoVisGroup_Child& avgc)
    {
        out << "\t\"" << avgc.DisplayName << "\"\n";
        out << "\t[\n";
        for(const std::string& entityName : avgc.Entities)
        {
            if(entityName.empty())
                continue;
            out << "\t\t\"" << entityName << "\"\n";
        }
        out << "\t]";
    }

    std::istream& Fgd::operator>>(std::istream& in, FgdFile::AutoVisGroup& avg)
    {

    }
    std::ostream& Fgd::operator<<(std::ostream& out, const FgdFile::AutoVisGroup& avg)
    {
        out << "@AutoVisGroup = \"" << avg.DisplayName << "\"\n";
        out << "[\n";
        for(const auto& ch : avg.Child)
            out << ch << '\n';
        out << "]\n";
        return out;
    }

    std::istream& Fgd::operator>>(std::istream& in, FgdFile& fgd)
    {

    }
    std::ostream& Fgd::operator<<(std::ostream& out, const FgdFile& fgd)
    {
        {
            out << "// Generated FGD file\n";

            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            out << "// " << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << '\n';
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
            for(const auto& cls : fgd.AutoVisGroups)
                out << cls << '\n';
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
