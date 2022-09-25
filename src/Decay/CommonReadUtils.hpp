#pragma once
#include "Common.hpp"

// Uncomment to use STD implementation of whitespace detection.
//#define DECAY_WHITESPACE_STD

// Utility functions
namespace Decay
{
    /// Simple logic to decide whenever an ASCII character is a whitespace character.
    [[nodiscard]] inline static constexpr bool IsWhitespace(char c) noexcept
    {
#ifdef DECAY_WHITESPACE_STD
        return std::isspace(static_cast<unsigned char>(c));
#else
        switch(c)
        {
            case '\0':
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
    /// Also skips all comments (from `//` to end of the line).
    inline static int IgnoreWhitespace(std::istream& in)
    {
        int ignoredChars = 0;

        while(in.good())
        {
            int c = in.peek();
            if(c == EOF) // End of File
            {
                in.setstate(std::ios_base::eofbit);
                assert(!in.fail());
                return ignoredChars;
            }
            else if(IsWhitespace(static_cast<char>(c))) // Whitespace
            {
                ignoredChars++;
                in.ignore();
                continue;
            }
            else // Not whitespace
            {
                if(c == '/') // Potentially start of a comment
                {
                    ignoredChars++;
                    in.ignore(); // Skip 1st '/'

                    c = in.peek();
                    if(c == '/') // Confirmed, there is a comment
                    {
                        ignoredChars++;
                        in.ignore(); // Skip 2nd '/'

                        // Skip all characters until newline is reached
                        while(true)
                        {
                            c = in.get();
                            if(c == EOF)
                                break;
                            ignoredChars++;
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
        }

        return ignoredChars;
    }

    /// Checks is provided "string" contains only numbers.
    /// Optional negative numbers ('-' character) and whitespaces.
    [[nodiscard]] inline static bool IsNumber(std::vector<char>& string, bool allowNegative, bool allowWhitespace)
    {
        for(char c : string)
        {
            if(c >= '0' && c <= '9')
                continue;
            if(allowNegative && c == '-')
                continue;
            if(allowWhitespace && IsWhitespace(c))
                continue;
            return false;
        }
        return true;
    }

    /// Convert `std::vector<char>` to `std::string`
    [[nodiscard]] inline static std::string str(const std::vector<char>& in, bool trim = false)
    {
        if(in.empty())
            return {};
        if(trim)
        {
            int startIndex = 0;
            { // Trim Start
                for(; startIndex < in.size(); startIndex++)
                {
                    if(!IsWhitespace(in[startIndex]))
                        break;
                }
                if(startIndex == in.size()) // All whitespace
                    return {};
            }
            int endIndex = 0;
            { // Trim End
                for(; endIndex >= 0; endIndex--)
                {
                    if(!IsWhitespace(in[in.size() - 1 - endIndex]))
                        break;
                }
                if(endIndex == 0) [[unlikely]] // All whitespace
                    return {};
            }
            return std::string(in.data() + startIndex, in.size() - startIndex - endIndex); // Trim
            //return std::string(in.data() + startIndex, in.size() - startIndex); // Trim Start
            //return std::string(in.data(), in.size() - endIndex); // Trim End
        }
        return std::string(in.data(), in.size());
    }
    /// Represent `std::vector<char>` as `std::string_view`
    [[nodiscard]] inline static std::string_view str_view(const std::vector<char>& in)
    {
        if(in.empty())
            return std::string_view();
        return std::string_view(in.data(), in.size());
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
    /// Reads text from `in` stream but only until first character which is not mentioned inside `readChars`.
    /// Works like `ReadUntilAny` but you specify which characters to read and not by which ones to end.
    template<std::size_t N>
    [[nodiscard]] inline static std::vector<char> ReadOnlyAny(std::istream& in, std::array<char, N> readChars)
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
                if(c == readChars[i])
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
    /// Variant of `ReadOnlyAny` but reads only numeric characters and optionally '-' char.
    [[nodiscard]] inline static std::vector<char> ReadOnlyNumber(std::istream& in, bool allowNegative, bool allowDecimal = false)
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
                continue;
            }
            else if(allowNegative && c == '-' && rtn.empty() /* Only first character */)
            {
                in.ignore();
                rtn.emplace_back('-');
                continue;
            }
            else if(allowDecimal && c == '.')
            {
                in.ignore();
                rtn.emplace_back('.');
                continue;
            }
            else
                return rtn;
        }
    }
    /// Extended version of `ReadUntilWhitespace` which allows you to specify 1 additional character.
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
    /// Extended version of `ReadUntilWhitespace` which allows you to specify additional characters.
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

    /// Puts `std::vector<char>` at the end of another `std::vector<char>`.
    inline static void Combine(std::vector<char>& to, const std::vector<char>& from)
    {
        to.reserve(from.size());
        for(int n2i = 0; n2i < from.size(); n2i++)
            to.emplace_back(from[n2i]);
    }

    /// Read quoted text.
    /// Sets `failbit` on End-of-File or other than '"' char.
    [[nodiscard]] inline static std::vector<char> ReadQuotedString(std::istream& in, bool allowEscapeCharacters = false)
    {
        if(!in.good())
            throw std::runtime_error("Stream is not in a good shape");

        std::vector<char> name;
        int c;
        {
            GOTO_QUOTED_STRING_START:
            IgnoreWhitespace(in);

            c = in.peek();
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

            { // Main part, always exists
                auto name2 = ReadUntil(in, '\"', true);
                // Insert `name2` into `name`
                Combine(name, name2);
            }
            while(!name.empty() && allowEscapeCharacters && name[name.size() - 1] == '\\') // Last char is `\` which mean there was `\"` found.
            {
                name.emplace_back('\"');

                auto name2 = ReadUntil(in, '\"', true);

                // Insert `name2` into `name`
                if(!name2.empty())
                    Combine(name, name2);
            }

            IgnoreWhitespace(in);

            c = in.peek();
            if(c == '+')
            {
                in.ignore(); // Skip '+'

                // `goto` to the beginning of parsing.
                // Recursion could be used here (but at different position in code - later in this function).
                goto GOTO_QUOTED_STRING_START;
            }
        }

        // Post-processing
        if(allowEscapeCharacters)
        {
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
                if(i != 0 || c != EOF)
                    in.seekg(-(i + 1), std::ios_base::cur);
                return false;
            }
        }
        return true;
    }
}
