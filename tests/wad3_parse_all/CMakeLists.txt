add_executable(Test_Wad3_Parse_All main.cpp)

target_link_libraries(Test_Wad3_Parse_All DecayLib)

add_test(NAME Test_Wad3_Parse_All COMMAND Test_Wad3_Parse_All)
set_tests_properties(Test_Wad3_Parse_All PROPERTIES LABELS "GoldSrc;wad;wad3")
