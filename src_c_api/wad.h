#pragma once

#ifdef __cplusplus
    #include <glm/glm.hpp>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // 1 byte per channel, 4 channels (red, green, blue, alpha)
#ifdef __cplusplus
    typedef glm::u8vec4 wad_rgba;
#else
    /// Equivalent to glm::u8vec4
    typedef struct {
        unsigned char r, g, b, a;
    } wad_rgba;
#endif

    // Texture

    typedef struct {
        /// Name of the texture.
        char name[16];
        /// Dimensions.
        unsigned int width, height;
        /// This is pointer to raw data.
        /// First pixel starts here, more follow.
        /// `NULL` if there are no data (only from BSP).
        wad_rgba* data;
    } wad_texture;

    /// Load all textures inside WAD2 / WAD3 file
    /// Returns `nullptr` if there were no textures, file was not found or there was problem with loading.
    wad_texture* wad_load_textures(const char* path, int* length);
    /// Release textures loaded from WAD file.
    void wad_free_textures(wad_texture* textures);


    // Image

    typedef struct {
        /// Name of the image.
        char name[16];
        /// Dimensions.
        unsigned int width, height;
        /// This is pointer to raw data.
        /// First pixel starts here, more follow.
        /// `NULL` if there are no data (only from BSP).
        wad_rgba* data;
    } wad_image;

    /// Load all images inside WAD2 / WAD3 file
    /// Returns `nullptr` if there were no images, file was not found or there was problem with loading.
    wad_image* wad_load_image(const char* path, int* length);
    /// Release images loaded from WAD file.
    void wad_free_image(wad_image* images);


    //TODO Font

#ifdef __cplusplus
};
#endif
