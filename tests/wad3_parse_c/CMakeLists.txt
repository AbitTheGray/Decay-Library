add_executable(Test_Wad3_Parse_C main.cpp)

target_link_libraries(Test_Wad3_Parse_C DecayLib_API)

add_test(NAME Test_Wad3_Parse_C COMMAND Test_Wad3_Parse_C)
set_tests_properties(Test_Wad3_Parse_C PROPERTIES LABELS "GoldSrc;wad;wad3;C")
