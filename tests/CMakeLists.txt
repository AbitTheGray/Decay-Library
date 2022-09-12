#--------------------------------
# BSP v30 (GoldSrc, Map)
#--------------------------------

add_subdirectory(bsp30_parse)
add_subdirectory(bsp30_parse_c)

add_subdirectory(bsp30_export_obj)

add_subdirectory(bsp30_export_entities)

#--------------------------------
# WAD 3 (GoldSrc, Textures)
#--------------------------------

add_subdirectory(wad3_parse)
add_subdirectory(wad3_parse_all)

add_subdirectory(wad3_parse_c)

add_subdirectory(wad3_test_image)
add_subdirectory(wad3_test_font)
add_subdirectory(wad3_test_texture)

add_subdirectory(wad3_export_images)
add_subdirectory(wad3_export_textures)
add_subdirectory(wad3_export_fonts)

#--------------------------------
# FGD (GoldSrc/Source, Entity Definitions)
#--------------------------------

add_subdirectory(fgd_out_and_in)

add_subdirectory(fgd_testfile)
