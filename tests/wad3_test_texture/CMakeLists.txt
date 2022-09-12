add_executable(Text_Wad3_Texture main.cpp)

target_link_libraries(Text_Wad3_Texture DecayLib)

add_test(NAME Text_Wad3_Texture COMMAND Text_Wad3_Texture)
set_tests_properties(Text_Wad3_Texture PROPERTIES LABELS "GoldSrc;wad;wad3")
