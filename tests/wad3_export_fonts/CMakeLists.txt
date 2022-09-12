add_executable(Text_Wad3_ExportFonts main.cpp)

target_link_libraries(Text_Wad3_ExportFonts DecayLib)

add_test(NAME Text_Wad3_ExportFonts COMMAND Text_Wad3_ExportFonts)
set_tests_properties(Text_Wad3_ExportFonts PROPERTIES LABELS "GoldSrc;wad;wad3")
