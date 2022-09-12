#include "FgdFile.hpp"

// Uncomment to use STD implementation of whitespace detection.
//#define FGD_WHITESPACE_STD

// Utility functions
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
            {
                in.setstate(std::ios_base::eofbit);
                return ignoredChars;
            }
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
    //TODO Comment
    template<std::size_t N>
    [[nodiscard]] inline static std::vector<char> ReadOnlyAny(std::istream& in, std::array<char, N> untilChars)
    {
        std::vector<char> rtn = {};
        while(true)
        {
            int c = in.peek();
            if(c == EOF)
                return rtn;

            bool found = false;
            for(std::size_t i = 0; i < N; i++)
            {
                if(c == untilChars[i])
                {
                    found = true;
                    if(!ReadChar(in, rtn)) // Read into `rtn`
                        throw std::runtime_error("Failed to read a char");
                    break;
                }
            }
            if(!found)
                return rtn;
        }
    }
    //TODO Comment
    [[nodiscard]] inline static std::vector<char> ReadOnlyNumber(std::istream& in, bool allowNegative)
    {
        std::vector<char> rtn = {};
        while(true)
        {
            int c = in.peek();
            if(c == EOF)
                return rtn;

            if(c >= '0' && c <= '9')
            {
                in.ignore();
                rtn.emplace_back(c);
                break;
            }
            if(allowNegative && c == '-')
            {
                in.ignore();
                rtn.emplace_back('-');
                break;
            }
            else
                return rtn;
        }
    }
    //TODO Comment
    [[nodiscard]] inline static std::vector<char> ReadUntilWhitespaceOr(std::istream& in, char untilChar, bool skipUntilChar = false)
    {
        std::vector<char> rtn = {};
        while(true)
        {
            int c = in.peek();
            if(c == EOF)
                return rtn;

            // Whitespace
            if(IsWhitespace(c))
            {
                if(skipUntilChar)
                    in.ignore(); // Skip peeked char
                return rtn;
            }
            // User-provided character
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
    //TODO Comment
    template<std::size_t N>
    [[nodiscard]] inline static std::vector<char> ReadUntilWhitespaceOrAny(std::istream& in, std::array<char, N> untilChars, bool skipUntilChar = false)
    {
        std::vector<char> rtn = {};
        while(true)
        {
            int c = in.peek();
            if(c == EOF)
                return rtn;

            // Whitespace
            if(IsWhitespace(c))
            {
                if(skipUntilChar)
                    in.ignore(); // Skip peeked char
                return rtn;
            }
            // User-provided list
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
                break; //TODO `"" + ""` is one quoted string

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

    /// Reads data from stream and returns whenever they match.
    /// Puts back (seeks back) all read characters if they did not match.
    /// Does NOT skip whitespaces before the text.
    [[nodiscard]] inline static bool TryReadText(std::istream& in, const std::string_view& str)
    {
        if(str.empty()) [[unlikely]]
            return true;

        for(int i = 0; i < str.length(); i++)
        {
            int c = in.get();
            if(c == EOF || c != str[i])
            {
                in.seekg(-(i + 1), std::ios_base::cur);
                return false;
            }
        }
        return true;
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
        "BaseClass",
        "PointClass",
        "SolidClass",
        "NPCClass",
        "KeyFrameClass",
        "MoveClass",
        "FilterClass"
    };
}

// Stream Operators
namespace Decay::Fgd
{
    // Class >> Property >> FlagOrChoice
    std::istream& operator>>(std::istream& in, FgdFile::PropertyFlagOrChoice& propertyFlag)
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

    // Class >> Option >> Param
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
            auto name = ReadUntilWhitespaceOrAny(
                in,
                std::array<char, 2>{
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

    // Class >> Option
    // Class >> Property >> Option
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
                auto type = ReadUntilWhitespaceOr(in, '(' /* Start of parameter */ );
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
                if(c == '=')
                {
                    in.ignore(); // Skip '='

#ifdef FGD_PROPERTY_ITEMS_LIMIT_FLAGS_CHOICES
                    if(!StringCaseInsensitiveEqual(property.Type, "flags") && !StringCaseInsensitiveEqual(property.Type, "choices"))
                        throw std::runtime_error("List of values (for a property) is only available for `flags` and `choices` types");
#endif

                    IgnoreWhitespace(in);

                    if(in.peek() != '[')
                        throw std::runtime_error("List of values (for a property) must be enclosed inside square brackets ('[' and ']')");
                    in.ignore();

                    while(true)
                    {
                        IgnoreWhitespace(in);

                        c = in.peek();
                        if(c == EOF) [[unlikely]]
                            throw std::runtime_error("End-of-File reached inside FGD property items (`flags` or `choices`)");
                        else if(c == ']')
                        {
                            in.ignore(); // Skip ']'
                            break;
                        }
                        else
                        {
                            FgdFile::PropertyFlagOrChoice flagOrChoice;
                            in >> flagOrChoice;
                            if(in.fail())
                                throw std::runtime_error("Failed to read Flag/Choice item");
                            property.FlagsOrChoices.emplace_back(flagOrChoice);
                        }
                    }
                }
            }
        }

        c = in.peek();
        if(c == EOF)
            return in;
        if(c == '=') // Flag or Choices item
        {
            in.ignore(); // Skip '='

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
                    break;
                }

            }
        }
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::Property& property)
    {
        assert(!property.Codename.empty());
        assert(!property.Type.empty());
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
        FgdFile::InputOutputType type;
        in >> type;
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

            std::vector<char> type = ReadUntilWhitespaceOr(in, ':' /* Separator between class/option and name */ );
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

            std::vector<char> name = ReadUntilWhitespaceOr(in, ':');
            if(name.empty())
                throw std::runtime_error("Class name cannot be empty");

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
            while(true)
            {
                IgnoreWhitespace(in);

                c = in.peek();
                if(c == ']')
                {
                    in.ignore(); // Skip ']'
                    break;
                }
                else if(c == EOF) [[unlikely]]
                    throw std::runtime_error("End-of-File inside a class");
                else
                {
                    // Property
                    {
                        FgdFile::Property property;
                        in >> property;
                        if(in.good())
                        {
                            clss.Properties.emplace_back(property);
                            continue;
                        }
                        else
                        {
                            in.clear(in.rdstate() & ~std::istream::failbit);
                        }
                    }

                    // Input / Output
                    {
                        FgdFile::InputOutput io;
                        in >> io;
                        if(in.good())
                        {
                            clss.IO.emplace_back(io);
                            continue;
                        }
                        else
                        {
                            in.clear(in.rdstate() & ~std::istream::failbit);
                        }
                    }

                    throw std::runtime_error("Class can only contain properties and Inputs/Outputs");
                }
            }
        }
    }
    std::ostream& operator<<(std::ostream& out, const FgdFile::Class& clss)
    {
        { // Header
            assert(!clss.Type.empty());
            assert(clss.Type.find(' ') == std::string::npos);
            out << '@' << clss.Type;
            if(!clss.Options.empty())
            {
                for(const FgdFile::Option& option : clss.Options)
                    out << ' ' << option;
            }

            assert(!clss.Codename.empty());
            assert(clss.Codename.find(' ') == std::string::npos);
            out << " = " << clss.Codename;

            if(!clss.Description.empty())
                out << " = \"" << clss.Codename << '\"';

            if(clss.Properties.empty() && clss.IO.empty())
            {
                out << "[]";
                return out;
            }
            out << '\n';
        }

        out << "[\n";

        // Properties
        for(const auto& property : clss.Properties)
        {
            out << '\t' << property << '\n';
        }
        bool hadPrevious = !clss.Properties.empty();

        // Input
        if(FgdFile::Class::HasType(clss.IO, FgdFile::InputOutputType::Input))
        {
            if(hadPrevious) // Just empty line between Inputs and Properties
                out << '\n';

            for(const auto& io: clss.IO)
                if(io.Type == FgdFile::InputOutputType::Input)
                    out << '\t' << io << '\n';
        }

        // Output
        if(FgdFile::Class::HasType(clss.IO, FgdFile::InputOutputType::Output))
        {
            if(hadPrevious) // Just empty line between Outputs and Inputs/Properties
                out << '\n';

            for(const auto& io: clss.IO)
                if(io.Type == FgdFile::InputOutputType::Output)
                    out << '\t' << io << '\n';
        }

        out << "]\n";
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
        out << "]\n";
        return out;
    }

    // FGD File
    std::istream& operator>>(std::istream& in, FgdFile& fgd)
    {
        while(true)
        {
            IgnoreWhitespace(in);

            int c = in.peek();
            if(c == EOF)
                return in;
            if(c != '@')
                throw std::runtime_error("Unexpected character inside FGD file - expected a class (or other object) that starts with '@'");

            std::vector<char> object = ReadUntilWhitespace(in);

            if(StringCaseInsensitiveEqual(str_view(object), "@mapsize")) [[unlikely]]
            {
                if(fgd.MapSize.has_value())
                    throw std::runtime_error("Single FGD file cannot contain more than one `@mapsize`");

                if(in.peek() != '(')
                    throw std::runtime_error("`@mapsize` must be followed by '(', see documentation");
                in.ignore();

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
                in.ignore();

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

                fgd.IncludeFiles.emplace(str(filename));
            }
            else if(StringCaseInsensitiveEqual(str_view(object), "@MaterialExclusion")) [[unlikely]]
            {
                IgnoreWhitespace(in);

                c = in.peek();
                if(c != '[')
                    throw std::runtime_error("Content of `@MaterialExclusion` must be inside square brackets `[` + `]`");
                in.ignore();

                IgnoreWhitespace(in);

                while(true)
                {
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
                            in.ignore();
                            break;
                        case EOF: // End of file (let's be lenient and allow it as the end of list)
                            break;

                        case ',':
                            throw std::runtime_error("Content of `@MaterialExclusion` must end by `]` and contain only names of directories quoted by '\"', entries are not separated by ','");
                        default:
                            throw std::runtime_error("Content of `@MaterialExclusion` must end by `]` and contain only names of directories quoted by '\"'");
                    }
                }
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
                fgd.Classes.emplace_back(clss);
            }
        }
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
