add_executable(Text_Wad3_Font main.cpp)

target_link_libraries(Text_Wad3_Font DecayLib)

add_test(NAME Text_Wad3_Font COMMAND Text_Wad3_Font)
set_tests_properties(Text_Wad3_Font PROPERTIES LABELS "GoldSrc;wad;wad3")
