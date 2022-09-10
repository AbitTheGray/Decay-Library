#pragma once

#include <filesystem>
#include <vector>
#include <map>
#include <optional>
#include "glm/glm.hpp"

#include "Decay/Common.hpp"

namespace Decay::Fgd
{
    /// Raw FGD file data, only basic validation is done on it.
    class FgdFile
    {
    public:
        enum class ValueType
        {
            /// `String` which contains no characters.
            Empty = 0,
            /// Only 0-9 characters.
            /// Can also be considered `Float` without decimal point.
            Integer = 1,
            /// Only 0-9 characters with one decimal point (without it considered `Integer`).
            Float = 2,
            /// Any value not fitting `Empty`, `Integer` and `Float`.
            String = 3
        };
        /// Tries to guess type from string variable.
        [[nodiscard]] inline static ValueType GuessTypeFromString(const std::string& str) noexcept
        {
            if(str.empty())
                return ValueType::Empty;
            bool hasDot = false;
            for(char c : str)
            {
                if(c == '.')
                {
                    if(hasDot) // Float cannot have two dots / decimal points
                        return ValueType::String;
                    hasDot = true;
                    continue;
                }
                if(c < '0' || c > '9')
                    return ValueType::String;
            }
            return hasDot ? ValueType::Float : ValueType::Integer;
        }

    // Classes
    public:
        /// Value for `choices` or `options` property types.
        struct PropertyFlag
        {
            /// `choices`: New value to set, can be `int`, `float` or `string`
            /// `flags`: Value to add or subtract, can only be `int`, for bit index use `FlagOrder()`
            std::string Index;
            /// Display text
            std::string DisplayName;
            /// Valid only for `flags`
            bool Default = false;

            /// Returns index of the flag for correct ordering.
            /// -1 = Index out of bounds (<= 0)
            /// -2 = Unable to match single flag
            [[nodiscard]] inline static int16_t FlagOrder(int16_t index) noexcept
            {
                if(index <= 0)
                    return -1;
                for(int i = 0; i < sizeof(decltype(index)) * 8; i++)
                {
                    if(index == (1 << i))
                        return i;
                }
                return -2;
            }
            /// Tries to guess type of `Index` variable.
            [[nodiscard]] inline ValueType GuessIndexType() const noexcept { return GuessTypeFromString(Index); }

            /// `includeDefault` should be set for `flags` type but not for `choices`
            std::ostream& Write(std::ostream& out, bool includeDefault) const;
        };
        struct OptionParam
        {
            std::string Name;
            /// true = `Name` contains value.
            /// false = `Name` contains property which then contains the value.
            bool Quoted;

            [[nodiscard]] inline operator bool() const noexcept { return Name.empty() && !Quoted; }
        };
        struct Option
        {
            std::string Name;
            std::vector<OptionParam> Params;
        };
        struct Property
        {
            std::string Codename;
            // (
            std::string Type;
            // )
            std::vector<Option> Options;
            // :
            std::string DisplayName;
            // :
            std::string DefaultValue;
            // :
            std::string Description;
            // =
            /// Values for `choices` or `options` types.
            std::vector<PropertyFlag> FlagsOrChoices;
        };
    public:
        enum class InputOutputType
        {
            Input = 0,
            Output = 1
        };
        struct InputOutput
        {
            InputOutputType Type;
            //
            std::string Name;
            // (
            std::string ParamType;
            // ) :
            std::string Description;
        };
    public:
        struct Class
        {
            // @
            std::string Type;
            //
            std::vector<Option> Options;
            // =
            std::string Codename;
            // :
            std::string DisplayName;
            // [
            std::vector<Property> Items;
            std::vector<InputOutput> IO;
            // ]
        };
    public:
        std::vector<Class> Classes = {};

    // Map Size
    public:
        std::optional<glm::i32vec2> MapSize;

    // Auto Vis Group
    public:
        struct AutoVisGroup_Child
        {
            std::string DisplayName;
            std::vector<std::string> Entities;
        };
        struct AutoVisGroup
        {
            std::string DisplayName;
            std::vector<AutoVisGroup_Child> Child;
        };
    public:
        std::vector<AutoVisGroup> AutoVisGroups;

    // Material exclusion
    public:
        std::vector<std::string> MaterialExclusion;

    // Include other FGD files (paths)
    public:
        std::vector<std::string> IncludeFiles;

    public:
        explicit FgdFile(std::istream& in);
        ~FgdFile() = default;

    public:
        /// Combines current and provided FGD files together.
        /// Bigger map size is used.
        /// Auto Vis Groups are combined (union) without duplicates.
        /// Includes are combined (you need to manually remove paths to both files if you don't want them there).
        /// Material Exclusions are combined.
        /// Classes are combined
        /// - Properties of existing ones are extended or overwritten (if type differs, the new one is used).
        /// - If descriptions differ and a new one is present then it will overwrite the old one.
        void Add(const FgdFile& toAdd);
        /// Opposite to `Add(FgdFile)`, can be used to generate the parameter for `Add(FgdFile)`.
        /// Use to minimize current FGD using `Base.fgd`.
        void Subtract(const FgdFile& toSub);
        /// Include acts similar to `Add` but prioritizes data in `this` FGD and not `toAdd` one.
        void Include(const FgdFile& toAdd);
        /// Call `Include` for all FGD files mentioned in `IncludeFiles`.
        /// This function is also called on included files before `Add` to allow nested includes.
        /// Every file is processed only once thanks to `filesToIgnore` being filled by files as they are being processed - it is recommended to place path to `this` FGD file there at the beginning.
        void ProcessIncludes(const std::filesystem::path& relativeToDirectory, std::vector<std::filesystem::path>& filesToIgnore);
    };

    std::istream& operator>>(std::istream& in, FgdFile::PropertyFlag&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::PropertyFlag& pf) { return pf.Write(out, true); }

    std::istream& operator>>(std::istream& in, FgdFile::OptionParam&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::OptionParam&);

    std::istream& operator>>(std::istream& in, FgdFile::Option&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::Option&);

    std::istream& operator>>(std::istream& in, FgdFile::Property&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::Property&);


    /// Sets `failbit` if value is not `input` nor `output` to allow further processing without an exception.
    std::istream& operator>>(std::istream& in, FgdFile::InputOutputType&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::InputOutputType&);

    /// Sets `failbit` if failed to read io type to allow further processing without an exception.
    std::istream& operator>>(std::istream& in, FgdFile::InputOutput&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::InputOutput&);


    std::istream& operator>>(std::istream& in, FgdFile::Class&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::Class&);


    std::istream& operator>>(std::istream& in, FgdFile::AutoVisGroup_Child&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::AutoVisGroup_Child&);

    std::istream& operator>>(std::istream& in, FgdFile::AutoVisGroup&);
    std::ostream& operator<<(std::ostream& out, const FgdFile::AutoVisGroup&);


    std::istream& operator>>(std::istream& in, FgdFile&);
    std::ostream& operator<<(std::ostream& out, const FgdFile&);
}
