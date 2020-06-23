#pragma once

static_assert(sizeof(unsigned int) == 4);
static_assert(sizeof(unsigned char) == 1);

#ifdef __cplusplus
extern "C"
{
#endif

    /// Equivalent to glm::u8vec4
    typedef struct {
        unsigned char r, g, b, a;
    } wad_rgba;

    typedef struct {
        /// Name of the texture
        char name[16];
        /// Dimensions
        unsigned int width, height;
        /// This is pointer to raw data
        /// First pixel starts here, more follow
        wad_rgba data;
    } wad_texture;

    wad_texture** wad_load_textures(const char* path, int* length);
    void wad_free_texture(wad_texture* texture);
    void wad_free_textures(int length, wad_texture** textures);

    //TODO Font
    //TODO Image

#ifdef __cplusplus
};
#endif
