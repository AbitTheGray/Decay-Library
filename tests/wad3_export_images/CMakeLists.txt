add_executable(Text_Wad3_ExportImages main.cpp)

target_link_libraries(Text_Wad3_ExportImages DecayLib)

add_test(NAME Text_Wad3_ExportImages COMMAND Text_Wad3_ExportImages)
set_tests_properties(Text_Wad3_ExportImages PROPERTIES LABELS "GoldSrc;wad;wad3")
