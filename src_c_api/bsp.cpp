#include "bsp.h"

#include <Decay/Bsp/BspFile.hpp>

using namespace Decay::Bsp;

std::vector<std::shared_ptr<BspFile>> BspFiles = {};

void* bsp_file_load(const char* path)
{
    std::shared_ptr<BspFile>& ptr = BspFiles.emplace_back(std::make_shared<BspFile>(path));
    return ptr.get();
}

void bsp_file_free(void* bsp_ptr)
{
    std::size_t i;
    for(i = 0; i < BspFiles.size(); i++)
    {
        const auto& vptr = BspFiles[i];
        if(vptr.get() == bsp_ptr)
            break;
    }

    if(i == BspFiles.size())
        return;
    BspFiles.erase(BspFiles.begin() + i);
}
