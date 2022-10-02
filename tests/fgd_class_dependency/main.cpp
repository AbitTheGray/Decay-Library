#include "Decay/Fgd/FgdFile.hpp"

int main()
{
    using namespace Decay::Fgd;

    FgdFile fgd{};
    fgd.Classes.emplace(
        "a1",
        FgdFile::Class{
            "BaseClass",
            {},
            "a1",
            {},
            {
                { "prop_a1", { "prop_a1", "integer" } }
            }
        }
    );
    fgd.Classes.emplace(
        "a2",
        FgdFile::Class{
            "BaseClass",
            {},
            "a2",
            {},
            {
                { "prop_a2", { "prop_a2", "integer" } }
            }
        }
    );
    fgd.Classes.emplace(
        "a3",
        FgdFile::Class{
            "BaseClass",
            {
                {
                    "base",
                    {
                        { "a1", false }
                    }
                }
            },
            "a3",
            {},
            {
                { "prop_a3", { "prop_a3", "integer" } }
            }
        }
    );
    fgd.Classes.emplace(
        "b1",
        FgdFile::Class{
            "PointClass",
            {
                {
                    "base",
                    {
                        { "a1", false }
                    }
                }
            },
            "b1",
            {},
            {
                { "prop_b1", { "prop_b1", "integer" } }
            }
        }
    );
    fgd.Classes.emplace(
        "b2",
        FgdFile::Class{
            "PointClass",
            {
                {
                    "base",
                    {
                        { "a2", false }
                    }
                }
            },
            "b2",
            {},
            {
                { "prop_b2", { "prop_b2", "integer" } }
            }
        }
    );
    fgd.Classes.emplace(
        "b3",
        FgdFile::Class{
            "PointClass",
            {
                {
                    "base",
                    {
                        { "a3", false }
                    }
                }
            },
            "b3",
            {},
            {
                { "prop_b3", { "prop_b3", "integer" } }
            }
        }
    );
    fgd.Classes.emplace(
        "b4",
        FgdFile::Class{
            "PointClass",
            {
                {
                    "base",
                    {
                        { "a1", false },
                        { "a3", false }
                    }
                }
            },
            "b4",
            {},
            {
                { "prop_b4", { "prop_b4", "integer" } }
            }
        }
    );

    auto classDependency = fgd.ProcessClassDependency();
    auto it_a1 = classDependency.find("a1");
    auto it_a2 = classDependency.find("a2");
    auto it_a3 = classDependency.find("a3");
    auto it_b1 = classDependency.find("b1");
    auto it_b2 = classDependency.find("b2");
    auto it_b3 = classDependency.find("b3");
    auto it_b4 = classDependency.find("b4");
    R_ASSERT(it_a1 == classDependency.end(), "`a1` class should not exist");
    R_ASSERT(it_a2 == classDependency.end(), "`a2` class should not exist");
    R_ASSERT(it_a3 == classDependency.end(), "`a3` class should not exist");
    R_ASSERT(it_b1 != classDependency.end(), "`b1` class should exist");
    R_ASSERT(it_b2 != classDependency.end(), "`b2` class should exist");
    R_ASSERT(it_b3 != classDependency.end(), "`b3` class should exist");
    R_ASSERT(it_b4 != classDependency.end(), "`b4` class should exist");
    R_ASSERT(classDependency.size() == 4, "`ProcessClassDependency` returned incorrect number of classes - they should include only `b` classes not `a`"); // only `b*`, all `a*` are base classes

    /*
    R_ASSERT(it_a1->second.Properties.contains("prop_a1"));

    R_ASSERT(it_a2->second.Properties.contains("prop_a2"));

    R_ASSERT(it_a3->second.Properties.contains("prop_a3"));
    R_ASSERT(it_a3->second.Properties.contains("prop_a1"));
    */

    R_ASSERT(it_b1->second.Properties.contains("prop_b1"), "b1 should contain its' property");
    R_ASSERT(it_b1->second.Properties.contains("prop_a1"), "b1 should contain a1's property");

    R_ASSERT(it_b2->second.Properties.contains("prop_b2"), "b2 should contain its' property");
    R_ASSERT(it_b2->second.Properties.contains("prop_a2"), "b2 should contain a2's property");

    R_ASSERT(it_b3->second.Properties.contains("prop_b3"), "b3 should contain its' property");
    R_ASSERT(it_b3->second.Properties.contains("prop_a1"), "b3 should contain a1's property");

    R_ASSERT(it_b4->second.Properties.contains("prop_b4"), "b4 should contain its' property");
    R_ASSERT(it_b4->second.Properties.contains("prop_a1"), "b4 should contain a1's property");
}
