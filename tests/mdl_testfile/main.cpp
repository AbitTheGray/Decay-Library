#include "Decay/Mdl/MdlFile.hpp"

#include "glm/gtx/string_cast.hpp"

using namespace Decay::Mdl;

int main(int argc, const char* argv[])
{
    if(argc < 2)
        throw std::runtime_error("Please provide path to MDL file");
    std::cout << argv[1] << std::endl;

    std::fstream in = std::fstream(argv[1], std::ios_base::in | std::ios_base::binary);
    MdlFile mdl(in);

#ifdef DEBUG
    std::cout << "Name: " << mdl.Info.Name << std::endl;
    std::cout << "Size: " << mdl.Info.Size << std::endl;
    std::cout << "Eyes: " << glm::to_string(mdl.Info.EyePosition) << std::endl;

    if(!mdl.Bones.empty())
    {
        std::cout << "Bones: " << mdl.Bones.size() << std::endl;
        for(int i = 0; i < mdl.Bones.size(); i++)
        {
            const auto& bone = mdl.Bones[i];
            std::cout << " - [" << i << "] " << bone.Name << " (parent " << bone.Parent << ")" << std::endl;
        }
    }
    else
        std::cout << "No bones" << std::endl;

    if(!mdl.BoneControllers.empty())
    {
        std::cout << "Bone Controllers: " << mdl.BoneControllers.size() << std::endl;
        for(const auto& boneController : mdl.BoneControllers)
            std::cout << " - " << boneController.Start << " to " << boneController.End << std::endl;
    }
    else
        std::cout << "No bone controllers" << std::endl;

    if(!mdl.Hitboxes.empty())
    {
        std::cout << "Hitboxes: " << mdl.Hitboxes.size() << std::endl;
        for(const auto& hitbox : mdl.Hitboxes)
            std::cout << " - bone " << hitbox.Bone << ", group " << hitbox.Group << ", " << glm::to_string(hitbox.BB_Min) << " to " << glm::to_string(hitbox.BB_Max) << std::endl;
    }
    else
        std::cout << "No hitboxes" << std::endl;

    if(!mdl.Sequences.empty())
    {
        std::cout << "Seqs: " << mdl.Sequences.size() << std::endl;
        for(const auto& seq : mdl.Sequences)
            std::cout << " - " << seq.Label << std::endl;
    }
    else
        std::cout << "No sequences" << std::endl;

    if(!mdl.SeqGroups.empty())
    {
        std::cout << "Seq Groups: " << mdl.SeqGroups.size() << std::endl;
        for(const auto& seqGroup : mdl.SeqGroups)
            std::cout << " - " << seqGroup.Label << " (" << seqGroup.Name << ")" << std::endl;
    }
    else
        std::cout << "No sequence group" << std::endl;

    if(!mdl.Textures.empty())
    {
        std::cout << "Textures: " << mdl.Textures.size() << std::endl;
        for(int i = 0; i < mdl.Textures.size(); i++)
        {
            const auto& kv = mdl.Textures[i];
            std::cout << " - [" << i << "] " << kv.first << " (" << kv.second.Width << " x " << kv.second.Height << ") " << std::endl;

            kv.second.WriteRgbPng(kv.first);
        }
    }
    else
        std::cout << "No textures" << std::endl;

    //TODO Skin

    if(!mdl.BodyParts.empty())
    {
        std::cout << "Body parts: " << mdl.BodyParts.size() << std::endl;

        for(const auto& bodypart : mdl.BodyParts)
        {
            std::cout << " - " << bodypart.Name << " (" << bodypart.Models.size() << " models, " << bodypart.Base << ")" << std::endl;

            for(const auto& model : bodypart.Models)
            {
                std::cout << "   - " << model.Name << " (type " << model.Type << ")" << std::endl;
                for(const auto& kv : model.Meshes)
                {
                    R_ASSERT(kv.first < mdl.Textures.size(), "Texture index out of bounds");
                    std::cout << "     - texture #" << kv.first << " (" << mdl.Textures[kv.first].first << "), " << kv.second.size() << " vertices" << std::endl;
                }
            }
        }
    }
    else
        std::cout << "No body parts" << std::endl;

    if(!mdl.Attachments.empty())
    {
        std::cout << "Attachments: " << mdl.Attachments.size() << std::endl;
        for(const auto& attachment : mdl.Attachments)
            std::cout << " - " << attachment.Name << " (type " << attachment.Type << ")" << std::endl;
    }
    else
        std::cout << "No attachments" << std::endl;
#endif

    //TODO
    /*
    {
        std::fstream out = std::fstream(argv[1] + std::string("_out"), std::ios_base::out);
        out << mdl;
    }

    std::fstream in2 = std::fstream(argv[1] + std::string("_out"), std::ios_base::in | std::ios_base::binary);
    MdlFile mdl2(in2);
     */
}
