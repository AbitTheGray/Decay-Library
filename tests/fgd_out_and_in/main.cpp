#include "Decay/Fgd/FgdFile.hpp"

using namespace Decay::Fgd;

void Test_OptionParam(const FgdFile::OptionParam& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    FgdFile::OptionParam result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.Name == original.Name);
    R_ASSERT(result.Quoted == original.Quoted);
    R_ASSERT(result == original);
}

void Test_Option(const FgdFile::Option& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    FgdFile::Option result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.Name == original.Name);
    R_ASSERT(result.Params.size() == original.Params.size());
    for(int i = 0; i < result.Params.size(); i++)
    {
        R_ASSERT(result.Params[i] == original.Params[i]);
    }
    R_ASSERT(result == original);
}

void Test_PropertyFlagOrChoice(const FgdFile::PropertyFlagOrChoice& original)
{
    {
        std::stringstream ss;
        original.Write(ss, true);
        R_ASSERT(ss.good());

#ifdef DEBUG
        std::cout << ss.str() << std::endl;
#endif
        ss.seekg(0, std::ios_base::beg);
        ss.seekp(0, std::ios_base::beg);

        FgdFile::PropertyFlagOrChoice result = {};
        ss >> result;
        R_ASSERT(ss.good() || ss.eof());
        R_ASSERT(result.Index == original.Index);
        R_ASSERT(result.DisplayName == original.DisplayName);
        R_ASSERT(result.Default == original.Default);
        R_ASSERT(result == original);
    }
    {
        std::stringstream ss;
        original.Write(ss, false);

#ifdef DEBUG
        std::cout << ss.str() << std::endl;
#endif
        ss.seekg(0, std::ios_base::beg);
        ss.seekp(0, std::ios_base::beg);

        FgdFile::PropertyFlagOrChoice result;
        ss >> result;
        R_ASSERT(ss.good() || ss.eof());
        R_ASSERT(result.Index == original.Index);
        R_ASSERT(result.DisplayName == original.DisplayName);
        R_ASSERT(result.Default == false);
        //R_ASSERT(result == original); // Cannot check as `original` may differ in `Default`
    }
}

void Test_Property(const FgdFile::Property& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    FgdFile::Property result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.Codename == original.Codename);
    R_ASSERT(result.Type == original.Type);
    R_ASSERT(result.ReadOnly == original.ReadOnly);
    R_ASSERT(result.DisplayName == original.DisplayName);
    R_ASSERT(result.DefaultValue == original.DefaultValue);
    R_ASSERT(result.Description == original.Description);
    R_ASSERT(result.FlagsOrChoices.size() == original.FlagsOrChoices.size());
    for(int i = 0; i < result.FlagsOrChoices.size(); i++)
    {
        R_ASSERT(result.FlagsOrChoices[i] == original.FlagsOrChoices[i]);
    }
    R_ASSERT(result == original);
}

void Test_InputOutputType(const FgdFile::InputOutputType& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    FgdFile::InputOutputType result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result == original);
}

void Test_InputOutput(const FgdFile::InputOutput& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    FgdFile::InputOutput result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.Type == original.Type);
    R_ASSERT(result.Name == original.Name);
    R_ASSERT(result.ParamType == original.ParamType);
    R_ASSERT(result.Description == original.Description);
    R_ASSERT(result == original);
}

void Test_Class(const FgdFile::Class& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    FgdFile::Class result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.Type == original.Type);
    R_ASSERT(result.Options.size() == original.Options.size());
    for(int i = 0; i < result.Options.size(); i++)
    {
        R_ASSERT(result.Options[i] == original.Options[i]);
    }
    R_ASSERT(result.Codename == original.Codename);
    R_ASSERT(result.Description == original.Description);
    R_ASSERT(result.Properties.size() == original.Properties.size());
    //TODO order-independent Properties comparison
    R_ASSERT(result.IO.size() == original.IO.size());
    //TODO order-independent IO comparison
    R_ASSERT(result == original);
}

void Test_AutoVisGroup_Child(const FgdFile::AutoVisGroup_Child& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    FgdFile::AutoVisGroup_Child result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.DisplayName == original.DisplayName);
    R_ASSERT(result.EntityClasses.size() == original.EntityClasses.size());
    auto it0 = result.EntityClasses.begin();
    auto it1 = original.EntityClasses.begin();
    for(int i = 0; i < result.EntityClasses.size(); i++, it0++, it1++)
    {
        R_ASSERT(*it0 == *it1);
    }
    R_ASSERT(result == original);
}

void Test_AutoVisGroup(const FgdFile::AutoVisGroup& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good());

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    FgdFile::AutoVisGroup result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof());
    R_ASSERT(result.DisplayName == original.DisplayName);
    R_ASSERT(result.Child.size() == original.Child.size());
    for(int i = 0; i < result.Child.size(); i++)
    {
        R_ASSERT(result.Child[i] == original.Child[i]);
    }
    R_ASSERT(result == original);
}

int main()
{
    // Option Param
    {
        std::cout << "Option Param" << std::endl;

        Test_OptionParam({ "abcd", true });
        Test_OptionParam({ "abcd", false });
        Test_OptionParam({{}, true });
        //INVALID (empty): Test_OptionParam({ {}, false });

        std::cout << std::endl;
    }
    // Option
    {
        std::cout << "Option" << std::endl;

        Test_Option({ "abcd", {}});
        Test_Option({ "abcd", {{ "a", true }}});
        Test_Option({ "abcd", {{ "a", false }}});
        Test_Option({ "abcd", {{ "a", true }, { "b", true }}});
        Test_Option({ "abcd", {{ "a", false }, { "b", false }}});
        Test_Option({ "abcd", {{ "a", true }, { "b", false }}});
        Test_Option({ "abcd", {{ "a", false }, { "b", true }}});

        Test_Option({ "halfgridsnap", {}});

        std::cout << std::endl;
    }
    // Property Flag/Choice
    {
        std::cout << "Property Flag/Choice" << std::endl;

        Test_PropertyFlagOrChoice({ "", "abcd", false });
        Test_PropertyFlagOrChoice({ "", "abcd", true });
        Test_PropertyFlagOrChoice({ "a", "abcd", false });
        Test_PropertyFlagOrChoice({ "a", "abcd", true });
        Test_PropertyFlagOrChoice({ "0", "abcd", false });
        Test_PropertyFlagOrChoice({ "0", "abcd", true });
        Test_PropertyFlagOrChoice({ "1.0", "abcd", false });
        Test_PropertyFlagOrChoice({ "1.0", "abcd", true });

        std::cout << std::endl;
    }
    // Property
    {
        std::cout << "Property" << std::endl;

        Test_Property({ "abcd", "string", false });
        Test_Property({ "abcd", "string", true  });
        Test_Property({ "abcd", "string", false, "AbCd" });
        Test_Property({ "abcd", "string", true , "AbCd" });
        Test_Property({ "abcd", "string", false, "AbCd", "aaBBccDD" });
        Test_Property({ "abcd", "string", true , "AbCd", "aaBBccDD" });
        Test_Property({ "abcd", "string", false, "AbCd", "aaBBccDD", "aaaaBBBBcccc" });
        Test_Property({ "abcd", "string", true , "AbCd", "aaBBccDD", "aaaaBBBBcccc" });
        Test_Property({ "abcd", "string", false, "AbCd", {}, "aaaaBBBBcccc" });
        Test_Property({ "abcd", "string", true , "AbCd", {}, "aaaaBBBBcccc" });
        Test_Property({ "abcd", "string", false, {}, "aaBBccDD", "aaaaBBBBcccc" });
        Test_Property({ "abcd", "string", true , {}, "aaBBccDD", "aaaaBBBBcccc" });
        Test_Property({ "abcd", "string", false, {}, {}, "aaaaBBBBcccc" });
        Test_Property({ "abcd", "string", true , {}, {}, "aaaaBBBBcccc" });

        // Flags
        std::vector<FgdFile::PropertyFlagOrChoice> flags = {
            { "", "a", false },
            { "0", "b", true },
            { "2", "c", false },
            { "1.0", "d", false }
        };
        Test_Property({ "abcd", "flags", false, {}    , {}        , {}            , flags });
        Test_Property({ "abcd", "flags", true , {}    , {}        , {}            , flags });
        Test_Property({ "abcd", "flags", false, "AbCd", {}        , {}            , flags });
        Test_Property({ "abcd", "flags", true , "AbCd", {}        , {}            , flags });
        Test_Property({ "abcd", "flags", false, "AbCd", "aaBBccDD", {}            , flags });
        Test_Property({ "abcd", "flags", true , "AbCd", "aaBBccDD", {}            , flags });
        Test_Property({ "abcd", "flags", false, "AbCd", "aaBBccDD", "aaaaBBBBcccc", flags });
        Test_Property({ "abcd", "flags", true , "AbCd", "aaBBccDD", "aaaaBBBBcccc", flags });
        Test_Property({ "abcd", "flags", false, "AbCd", {}        , "aaaaBBBBcccc", flags });
        Test_Property({ "abcd", "flags", true , "AbCd", {}        , "aaaaBBBBcccc", flags });
        Test_Property({ "abcd", "flags", false, {}    , "aaBBccDD", "aaaaBBBBcccc", flags });
        Test_Property({ "abcd", "flags", true , {}    , "aaBBccDD", "aaaaBBBBcccc", flags });
        Test_Property({ "abcd", "flags", false, {}    , {}        , "aaaaBBBBcccc", flags });
        Test_Property({ "abcd", "flags", true , {}    , {}        , "aaaaBBBBcccc", flags });

        std::vector<FgdFile::PropertyFlagOrChoice> choices = {
            { "", "a" },
            { "0", "b" },
            { "2", "c" },
            { "1.0", "d" }
        };
        Test_Property({ "abcd", "choices", false, {}    , {}        , {}            , choices });
        Test_Property({ "abcd", "choices", true , {}    , {}        , {}            , choices });
        Test_Property({ "abcd", "choices", false, "AbCd", {}        , {}            , choices });
        Test_Property({ "abcd", "choices", true , "AbCd", {}        , {}            , choices });
        Test_Property({ "abcd", "choices", false, "AbCd", "aaBBccDD", {}            , choices });
        Test_Property({ "abcd", "choices", true , "AbCd", "aaBBccDD", {}            , choices });
        Test_Property({ "abcd", "choices", false, "AbCd", "aaBBccDD", "aaaaBBBBcccc", choices });
        Test_Property({ "abcd", "choices", true , "AbCd", "aaBBccDD", "aaaaBBBBcccc", choices });
        Test_Property({ "abcd", "choices", false, "AbCd", {}        , "aaaaBBBBcccc", choices });
        Test_Property({ "abcd", "choices", true , "AbCd", {}        , "aaaaBBBBcccc", choices });
        Test_Property({ "abcd", "choices", false, {}    , "aaBBccDD", "aaaaBBBBcccc", choices });
        Test_Property({ "abcd", "choices", true , {}    , "aaBBccDD", "aaaaBBBBcccc", choices });
        Test_Property({ "abcd", "choices", false, {}    , {}        , "aaaaBBBBcccc", choices });
        Test_Property({ "abcd", "choices", true , {}    , {}        , "aaaaBBBBcccc", choices });

        std::cout << std::endl;
    }
    // Input/Output Type
    {
        std::cout << "Input/Output Type" << std::endl;

        Test_InputOutputType(FgdFile::InputOutputType::Input);
        Test_InputOutputType(FgdFile::InputOutputType::Output);

        std::cout << std::endl;
    }
    // Input/Output
    {
        std::cout << "Input/Output" << std::endl;

        Test_InputOutput({ FgdFile::InputOutputType::Input, "abcd", "void" });
        Test_InputOutput({ FgdFile::InputOutputType::Input, "abcd", "integer" });
        Test_InputOutput({ FgdFile::InputOutputType::Input, "abcd", "void", "aaBBccDD" });
        Test_InputOutput({ FgdFile::InputOutputType::Input, "abcd", "integer", "aaBBccDD" });

        Test_InputOutput({ FgdFile::InputOutputType::Output, "abcd", "void" });
        Test_InputOutput({ FgdFile::InputOutputType::Output, "abcd", "integer" });
        Test_InputOutput({ FgdFile::InputOutputType::Output, "abcd", "void", "aaBBccDD" });
        Test_InputOutput({ FgdFile::InputOutputType::Output, "abcd", "integer", "aaBBccDD" });

        std::cout << std::endl;
    }
    // Class
    {
        std::cout << "Class" << std::endl;

        Test_Class({ "BaseClass", {}, "abcd" });
        Test_Class({ "BaseClass", {}, "abcd", "aaBBccDD" });
        Test_Class({ "BaseClass", {}, "abcd", "aaBBccDD",
                     {
                         { "abcd", { "abcd", "string", false } }
                     }
                   });
        Test_Class({ "BaseClass", {}, "abcd", "aaBBccDD",
                     {
                         { "abcd", { "abcd", "string", false } },
                         { "abcd2", { "abcd2", "integer", false } }
                     }
                   });
        Test_Class({ "BaseClass", {}, "abcd", "aaBBccDD",
                     {},
                     {
                         { "abcd", { FgdFile::InputOutputType::Input, "abcd", "void" } },
                         { "abcd2", { FgdFile::InputOutputType::Input, "abcd2", "integer", "aaBBccDD" } }
                     }
                   });
        Test_Class({ "BaseClass", {}, "abcd", "aaBBccDD",
                     {},
                     {
                         { "abcd", { FgdFile::InputOutputType::Input, "abcd", "void" } },
                         { "abcd2", { FgdFile::InputOutputType::Input, "abcd2", "integer", "aaBBccDD" } },
                         { "abcd3", { FgdFile::InputOutputType::Output, "abcd3", "integer", "aaBBccDD" } }
                     }
                   });
        Test_Class({ "BaseClass", {}, "abcd", "aaBBccDD",
                     {
                         { "abcd", { "abcd", "string", false } },
                         { "abcd2", { "abcd2", "integer", false } }
                     },
                     {
                         { "abcd", { FgdFile::InputOutputType::Input, "abcd", "void" } },
                         { "abcd2", { FgdFile::InputOutputType::Input, "abcd2", "integer", "aaBBccDD" } },
                         { "abcd3", { FgdFile::InputOutputType::Output, "abcd3", "integer", "aaBBccDD" } }
                     }
                   });

        std::cout << std::endl;
    }
    // AutoVisGroup Child
    {
        std::cout << "AutoVisGroup Child" << std::endl;

        Test_AutoVisGroup_Child({ "abcd", {} });
        Test_AutoVisGroup_Child({ "abcd", { "a" } });
        Test_AutoVisGroup_Child({ "abcd", { "a", "b" } });
        Test_AutoVisGroup_Child({ "abcd", { "a", "b", "c" } });

        std::cout << std::endl;
    }
    // AutoVisGroup
    {
        std::cout << "AutoVisGroup" << std::endl;

        Test_AutoVisGroup({ "abcd", {}});
        Test_AutoVisGroup({ "abcd",
                            {
                                { "abcd", {} },
                                { "abcd2", {} }
                            }
                          });
        Test_AutoVisGroup({ "abcd",
                            {
                                { "abcd", { "a", "b" } },
                                { "abcd2", { "a", "b", "c" } }
                            }
                          });

        std::cout << std::endl;
    }
}
