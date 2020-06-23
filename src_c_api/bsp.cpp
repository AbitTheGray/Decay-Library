#include "bsp.h"

#include <Decay/Bsp/BspFile.hpp>
#include <Decay/Bsp/BspTree.hpp>

using namespace Decay::Bsp;

std::vector<std::shared_ptr<BspFile>> BspFiles = {};

inline static std::shared_ptr<BspFile> GetBspPointer(bsp_file* bspFile)
{
    BspFile* ptr = reinterpret_cast<BspFile*>(bspFile);

    for(auto& bsp : BspFiles)
        if(bsp.get() == ptr)
            return bsp;
    return nullptr;
}

bsp_file* bsp_file_load(const char* path)
{
    if(path == nullptr)
        return nullptr;

    std::filesystem::path filename(path);
    if(!std::filesystem::exists(filename))
        return nullptr; // File not found
    if(!std::filesystem::is_regular_file(filename))
        return nullptr; // Path target is not a file

    try
    {
        return reinterpret_cast<bsp_file*>(BspFiles.emplace_back(std::make_shared<BspFile>(path)).get());
    }
    catch(std::exception& ex)
    {
        std::cerr << "bsp_file_load(\"" << path << "\") resulted in: " << ex.what() << std::endl;
        return nullptr;
    }
}

void bsp_file_free(bsp_file* bspFile)
{
    BspFile* ptr = reinterpret_cast<BspFile*>(bspFile);

    std::size_t i;
    for(i = 0; i < BspFiles.size(); i++)
    {
        const auto& vptr = BspFiles[i];
        if(vptr.get() == ptr)
            break;
    }

    if(i == BspFiles.size())
        return;
    BspFiles.erase(BspFiles.begin() + i);
}

std::vector<std::shared_ptr<BspTree>> BspTrees = {};

bsp_tree* bsp_tree_create(bsp_file* bspFile)
{
    auto bsp = GetBspPointer(bspFile);
    if(bsp == nullptr)
        return nullptr;

    try
    {
        return reinterpret_cast<bsp_tree*>(BspTrees.emplace_back(std::make_shared<BspTree>(bsp)).get());
    }
    catch(std::exception& ex)
    {
        std::cerr << "bsp_tree_create(bspFile) resulted in: " << ex.what() << std::endl;
        return nullptr;
    }
}

void bsp_tree_free(bsp_tree* bspTree)
{
    BspTree* ptr = reinterpret_cast<BspTree*>(bspTree);

    std::size_t i;
    for(i = 0; i < BspTrees.size(); i++)
    {
        const auto& vptr = BspTrees[i];
        if(vptr.get() == ptr)
            break;
    }

    if(i == BspTrees.size())
        return;
    BspTrees.erase(BspTrees.begin() + i);
}
