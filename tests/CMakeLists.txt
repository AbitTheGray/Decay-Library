#--------------------------------
# BSP (Map)
#--------------------------------

add_subdirectory(bsp_parse)

add_subdirectory(bsp_parse_c)

add_subdirectory(bsp_export_obj)

#--------------------------------
# WAD (Textures)
#--------------------------------

add_subdirectory(wad_parse)
add_subdirectory(wad_parse_all)

add_subdirectory(wad_parse_c)

add_subdirectory(wad_test_image)
add_subdirectory(wad_test_font)
add_subdirectory(wad_test_texture)

add_subdirectory(wad_export_textures)
add_subdirectory(wad_export_fonts)
