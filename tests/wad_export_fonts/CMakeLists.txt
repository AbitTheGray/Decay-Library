add_executable(Test_Wad_ExportFonts main.cpp)

target_link_libraries(Test_Wad_ExportFonts DecayLib)

add_test(NAME Test_Wad_ExportFonts COMMAND Test_Wad_ExportFonts)
