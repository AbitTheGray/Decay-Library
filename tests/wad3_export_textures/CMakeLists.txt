add_executable(Text_Wad3_ExportTextures main.cpp)

target_link_libraries(Text_Wad3_ExportTextures DecayLib)

add_test(NAME Text_Wad3_ExportTextures COMMAND Text_Wad3_ExportTextures)
set_tests_properties(Text_Wad3_ExportTextures PROPERTIES LABELS "GoldSrc;wad;wad3")
