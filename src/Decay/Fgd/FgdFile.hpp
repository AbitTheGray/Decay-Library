#pragma once

#include "Decay/Common.hpp"

#ifdef DECAY_JSON_LIB
#   include "nlohmann/json.hpp"
#endif

// Use `std::any_of` instead of loop implementation.
//#define FGD_INPUT_OUTPUT_STD_ANY_OF

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
        /// Useful for property type.
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
        struct OptionParam
        {
            std::string Name;
            /// true = `Name` contains value.
            /// false = `Name` contains property which then contains the value.
            bool Quoted;

            [[nodiscard]] inline operator bool() const noexcept { return Name.empty() && !Quoted; }

            /// Can also be used as boolean because 0 means not a vector.
            /// Won't work with floating-point numbers.
            [[nodiscard]] int GetVectorSize() const;

            [[nodiscard]] inline bool operator==(const OptionParam& other) const
            {
                return Name == other.Name && Quoted == other.Quoted;
            }
            [[nodiscard]] inline bool operator!=(const OptionParam& other) const { return !operator==(other); }
        };
        struct Option
        {
            std::string Name;
            std::vector<OptionParam> Params;

            [[nodiscard]] inline bool operator==(const Option& other) const
            {
                if(Name != other.Name)
                    return false;

                if(Params.size() != other.Params.size())
                    return false;
                for(int i = 0; i < Params.size(); i++)
                    if(Params[i] != other.Params[i])
                        return false;
                return true;
            }
            [[nodiscard]] inline bool operator!=(const Option& other) const { return !operator==(other); }
        };
        /// Value for `choices` or `options` property types.
        struct PropertyFlagOrChoice
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

            [[nodiscard]] inline bool operator==(const PropertyFlagOrChoice& other) const
            {
                return Index == other.Index && DisplayName == other.DisplayName && Default == other.Default;
            }
            [[nodiscard]] inline bool operator!=(const PropertyFlagOrChoice& other) const { return !operator==(other); }
        };
        struct Property
        {
            std::string Codename;
            // (
            std::string Type;
            // )
            bool ReadOnly;
            // :
            std::string DisplayName;
            // :
            std::string DefaultValue;
            // :
            std::string Description;
            // =
            /// Values for `choices` or `options` types.
            std::vector<PropertyFlagOrChoice> FlagsOrChoices;

            [[nodiscard]] inline bool operator==(const Property& other) const
            {
                if(Codename != other.Codename)
                    return false;
                if(Type != other.Type)
                    return false;
                if(ReadOnly != other.ReadOnly)
                    return false;
                if(DisplayName != other.DisplayName)
                    return false;
                if(DefaultValue != other.DefaultValue)
                    return false;
                if(Description != other.Description)
                    return false;

                if(FlagsOrChoices.size() != other.FlagsOrChoices.size())
                    return false;
                for(int i = 0; i < FlagsOrChoices.size(); i++)
                    if(FlagsOrChoices[i] != other.FlagsOrChoices[i])
                        return false;
                return true;
            }
            [[nodiscard]] inline bool operator!=(const Property& other) const { return !operator==(other); }
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

            [[nodiscard]] inline bool operator==(const InputOutput& other) const
            {
                return Type == other.Type && Name == other.Name && ParamType == other.ParamType && Description == other.Description;
            }
            [[nodiscard]] inline bool operator!=(const InputOutput& other) const { return !operator==(other); }
        };
    public:
        struct Class
        {
            static std::vector<std::string> ValidTypes;

            // @
            std::string Type; ///< Without '@' at the beginning.
            std::vector<Option> Options;
            // =
            std::string Codename;
            // :
            std::string Description;
            // [
            std::vector<Property> Properties;
            std::vector<InputOutput> IO;
            // ]

            inline static bool HasType(const std::vector<InputOutput>& ios, InputOutputType type) noexcept
            {
#ifdef FGD_INPUT_OUTPUT_STD_ANY_OF
                return std::any_of(ios.begin(), ios.end(), [&type](const InputOutput& io){ return io.Type == type; });
#else
                for(const InputOutput& io : ios) // NOLINT(readability-use-anyofallof)
                    if(io.Type == type)
                        return true;
                return false;
#endif
            }

            [[nodiscard]] inline bool operator==(const Class& other) const
            {
                if(Type != other.Type)
                    return false;

                if(Options.size() != other.Options.size())
                    return false;
                for(int i = 0; i < Options.size(); i++)
                    if(Options[i] != other.Options[i])
                        return false;

                if(Codename != other.Codename)
                    return false;
                if(Description != other.Description)
                    return false;

                if(Properties.size() != other.Properties.size())
                    return false;
                for(int i = 0; i < Properties.size(); i++)
                    if(Properties[i] != other.Properties[i])
                        return false;

                if(IO.size() != other.IO.size())
                    return false;
                for(int i = 0; i < IO.size(); i++)
                    if(IO[i] != other.IO[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Class& other) const { return !operator==(other); }
        };
    public:
        std::vector<Class> Classes = {};

    // Map Size
    public:
        std::optional<glm::i32vec2> MapSize = {};

    // Auto Vis Group
    public:
        struct AutoVisGroup_Child
        {
            std::string DisplayName;
            std::set<std::string> EntityClasses;

            [[nodiscard]] inline bool operator==(const AutoVisGroup_Child& other) const
            {
                if(DisplayName != other.DisplayName)
                    return false;

                if(EntityClasses.size() != other.EntityClasses.size())
                    return false;
                auto it0 = EntityClasses.begin();
                auto it1 = other.EntityClasses.begin();
                for(int i = 0; i < EntityClasses.size(); i++, it0++, it1++)
                    if(*it0 != *it1)
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const AutoVisGroup_Child& other) const { return !operator==(other); }
        };
        struct AutoVisGroup
        {
            std::string DisplayName;
            std::vector<AutoVisGroup_Child> Child;

            [[nodiscard]] inline bool operator==(const AutoVisGroup& other) const
            {
                if(DisplayName != other.DisplayName)
                    return false;

                if(Child.size() != other.Child.size())
                    return false;
                for(int i = 0; i < Child.size(); i++)
                    if(Child[i] != other.Child[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const AutoVisGroup& other) const { return !operator==(other); }
        };
    public:
        std::vector<AutoVisGroup> AutoVisGroups = {};

    // Material exclusion
    public:
        std::set<std::string> MaterialExclusion = {};

    // Include other FGD files (paths)
    public:
        std::vector<std::string> IncludeFiles = {};

    public:
        explicit FgdFile() = default;
        explicit FgdFile(std::istream& in);
        ~FgdFile() = default;

    public:
        /// TL;DR: Add newer file.
        /// Combines current and provided FGD files together.
        /// Bigger map size is used.
        /// Auto Vis Groups are combined (union) without duplicates (case-sensitive).
        /// Includes are combined without (case-insensitive) duplicates (you need to manually remove paths to both files if you don't want them there).
        /// Material Exclusions are combined.
        /// Classes are combined
        /// - Properties of existing ones are extended or overwritten (if type differs, the new one is used).
        /// - If descriptions differ and a new one is present then it will overwrite the old one.
        void Add(const FgdFile& toAdd);
        /// Opposite to `Add(FgdFile)`, can be used to generate the parameter for `Add(FgdFile)`.
        /// Use to minimize current FGD using `Base.fgd`.
        void Subtract(const FgdFile& toSub, bool ignoreDescription, bool ignoreDisplayName);
        /// TL;DR: Add older file.
        /// Include acts similar to `Add` but prioritizes data in `this` FGD and not `toAdd` one.
        void Include(const FgdFile& toAdd);
        /// Call `Include` for all FGD files mentioned in `IncludeFiles`.
        /// This function is also called on included files before `Add` to allow nested includes.
        /// Every file is processed only once thanks to `filesToIgnore` being filled by files as they are being processed - it is recommended to place path to `this` FGD file there at the beginning.
        void ProcessIncludes(const std::filesystem::path& relativeToDirectory, std::vector<std::filesystem::path>& filesToIgnore);
        /// Completely reworks the order of `Classes` based on `base(...)` option to make sure a class in listed AFTER its dependencies (=base classes first).
        /// Throws exception if a recursive dependency is found.
        /// Tries to use Alphabetical sorting when possible.
        void OrderClassesByDependency();

#ifdef DECAY_JSON_LIB
        [[nodiscard]] nlohmann::json ExportAsJson() const;
#endif
    };

    // All >> (std::istream) operators should set `failbit` on fail only if the stream/data can recover from it (and seek back to original position).
    // Exception should be thrown in middle of reading where there is no valid way to recover (was recognized as the type but formatting was wrong).
#pragma region Stream Operators
    std::istream& operator>>(std::istream& in, FgdFile::PropertyFlagOrChoice&);
    [[deprecated("Use `FgdFile::PropertyFlagOrChoice.Write` as there you can specify whenever you want defaults or not (for `flags` or for `choices`)")]]
    inline std::ostream& operator<<(std::ostream& out, const FgdFile::PropertyFlagOrChoice& pf) { return pf.Write(out, true); }

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
#pragma endregion
}
